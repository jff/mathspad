#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "variable.h"
#include "leaftree.h"
#include "flexarray.h"
#include "unistring.h"
#include "refcounting.h"

typedef struct {
    char *name;     /* UTF */
    Argument value;
} Variable;

static void var_free(void *var)
{
    free(((Variable*)var)->name);
    /* free_argument(((Variable*)var)->value); */
}

static int var_cmp(const void *var1, const void *var2)
{
  return strcmp(((Variable*)var1)->name, ((Variable*)var2)->name);
}

static LeafTree allvars = { 0, var_cmp, var_free, sizeof(Variable) };

static int value_cmp(const void *val1, const void *val2)
{
  return -1;
}

static Value empty_val= {0};

/* global values/variables are never removed
**
** an Argument points in the valuepool. To find the constants
** in the valuepool, a database of Arguments is set on top of the
** value pool. To find a constant C for a type T, set the type of
** argument 0 to T and the value of argument 0 to C and use a
** type-T-compare function to compare two arguments, known to be
** of type T.
*/

static FlexArray valuepool = { 0, 0, 0, sizeof(Value), value_cmp };
#define valuepool_add(V)   fx_add(&valuepool, &(V))
#define valuepool_item(I)  (*((Value*) fx_item(&valuepool,(I))))
#define valuepool_max      fx_max(&valuepool)

Argument define_global_variable(Type type, char *name)
{
    Value v;
    Variable vr;
    Argument r;
    v=empty_val;
    v.type=type;
    ConstructValue(&v);
    r = valuepool_max;
    vr.name=name;
    vr.value=r;
    valuepool_add(v);
    LT_insert(&allvars, &vr);
    return r;
}

Argument define_program_variable(Type type, char *name, void *address)
{
    Value v;
    Variable vr;
    Argument r;
    v=empty_val;
    v.type=ToRefType(type);
    v.val.stval=address;
    r = valuepool_max;
    vr.name=name;
    vr.value=r;
    valuepool_add(v);
    LT_insert(&allvars, &vr);
    return r;
}

static int argint_cmp(const void *arg1, const void *arg2)
{
  int i,j;
  i= valuepool_item(*((int*)arg1)).val.ival;
  j= valuepool_item(*((int*)arg2)).val.ival;
  return (i<j?-1:(i==j?0:1));
}

static LeafTree intconsts = { 0, argint_cmp, 0, sizeof(Argument) };

Argument define_int_constant(int n)
{
    Value v;
    Argument r;
    v=empty_val;
    v.type=ToConstType(IntType);
    v.val.ival=n;
    valuepool_item(0)=v;
    r=0;
    if (!LT_member(&intconsts,&r)) {
	r = valuepool_max;
	valuepool_add(v);
	LT_insert(&intconsts, &r);
    }
    if (LT_found_leaf()) 
        return *((Argument*)LT_found_leaf());
    else
        return 0;
}

static int argstring_cmp(const void *arg1, const void *arg2)
{
  Uchar *s,*t;
  s= valuepool_item(*((int*)arg1)).val.stval;
  t= valuepool_item(*((int*)arg2)).val.stval;
  return Ustrcmp(s,t);
}

static LeafTree stringconsts = { 0, argstring_cmp, 0, sizeof(Argument) };

/* it make a copy of the argument if it needs to be added. Freeing
** the argument is not always possible or wanted.
*/
Argument define_string_constant(Uchar *s)
{
    Value v;
    Argument r;
    v=empty_val;
    v.type=ToConstType(StringType);
    v.val.stval=s;
    valuepool_item(0)=v;
    r=0;
    if (!LT_member(&stringconsts,&r)) {
	int i;
	Uchar *c;
	i=Ustrlen(s)+1;
	c=malloc(sizeof(Uchar)*i);
	if (c) {
	    memcpy(c,s,sizeof(Uchar)*i);
	    v.val.stval=c;
	    increase_refcount(c,free);
	    r = valuepool_max;
	    valuepool_add(v);
	    LT_insert(&stringconsts, &r);
	}
    }
    if (LT_found_leaf()) return *((Argument*)LT_found_leaf());
    else return 0;
}

static int argreal_cmp(const void *arg1, const void *arg2)
{
  double i,j;
  i= valuepool_item(*((int*)arg1)).val.rval;
  j= valuepool_item(*((int*)arg2)).val.rval;
  return (i<j?-1:(i==j?0:1));
}

static LeafTree realconsts = { 0, argreal_cmp, 0, sizeof(Argument) };

Argument define_real_constant(double d)
{
    Value v;
    Argument r;
    v=empty_val;
    v.type=ToConstType(RealType);
    v.val.rval=d;
    valuepool_item(0)=v;
    r=0;
    if (!LT_member(&realconsts,&r)) {
	r = valuepool_max;
	valuepool_add(v);
	LT_insert(&realconsts, &r);
    }
    if (LT_found_leaf()) return *((Argument*)LT_found_leaf());
    else return 0;
}

/* localname, localvar and localscope all contain the same
** number of items. localname contains the names of the variables,
** localvar contains the values and localscope contains the
** depth of the block in which the variable occurs.
*/
typedef int (*CmpFunc)(const void*,const void*);

static FlexArray localname = { 0, 0, 0, sizeof(char*), (CmpFunc) strcmp };
#define localname_add(N)  fx_add(&localname, &(N))
#define localname_item(I) (*((char**) fx_item(&localname, (I))))
#define localname_max     fx_max(&localname)
#define localname_clear() fx_clear(&localname)
#define localname_copy()  (char** )fx_copy(&localname)

static FlexArray localvar = { 0, 0, 0, sizeof(Value), value_cmp };
#define localvar_add(V)   fx_add(&localvar, &(V))
#define localvar_item(I)  (*((Value*) fx_item(&localvar,(I))))
#define localvar_max      fx_max(&localvar)
#define localvar_clear()  fx_clear(&localvar)
#define localvar_copy()   (Value*) fx_copy(&localvar)

static FlexArray localscope = { 0,0,0, sizeof(int), int_cmp };
#define localscope_add(S)  fx_add(&localscope, &(S))
#define localscope_item(I) (*((int*) fx_item(&localscope,(I))))
#define localscope_max     fx_max(&localscope)
#define localscope_clear() fx_clear(&localscope)

static int scope_count=0;

/* For parsing: open a block to define a new scope for variables */
void new_local_variables(void)
{
    scope_count++;
}

/* For parsing: add a new local variable to the list */
Argument define_local_variable(Type type, char *name)
{
    Value v;
    char *mname;
    mname=malloc(strlen(name)+1);
    strcpy(mname, name);
    name=mname;
    if (!scope_count) {
        /* global variable */
	return define_global_variable(type, name);
    } else {
	localname_add(name);
	v.type=type;
	localvar_add(v);
	localscope_add(scope_count);
	return -localvar_max;
    }
}

/* For parsing: close the last block of local variables.
** If that block is the main block, the function return a list of
** values for local variables and arguments
*/
Value *get_local_variables(int *nrargs)
{
    Value *retval=0;
    if (scope_count>0) {
	scope_count--;
	if (!scope_count) {
	    int i;
	    retval = localvar_copy();
	    if (nrargs) *nrargs=localvar_max;
	    localvar_clear();
	    localscope_clear();
	    /* free local variable names */
	    for (i=0; i<localname_max; i++) {
		if (localname_item(i)) free(localname_item(i));
	    }
	    localname_clear();
	}
    }
    return retval;
}

/* a stack of local variables (lists of values) */
typedef struct LVStack LVStack;
struct LVStack {
    Value *lvlist;
    int length;
    LVStack *prev;
};
static LVStack *lvstack=0;
   
void push_local_variables(Value *list, int nr)
{
    LVStack *lv;
    lv = malloc(sizeof(LVStack));
    lv->prev=lvstack;
    lv->lvlist=list;
    lv->length=nr;
    lvstack=lv;
}

void pop_local_variables(void)
{
    LVStack *lv;
    if (lvstack) {
	lv=lvstack;
	lvstack=lv->prev;
	free(lv);
    }
}

Value *get_local_values(int *len)
{
  if (lvstack) {
    *len=lvstack->length;
    return lvstack->lvlist;
  } else {
    *len=0;
    return 0;
  }
}

Value *get_value(Argument arg)
{
    if (arg<0) {
	if (lvstack && (-arg-1)<lvstack->length) {
	    return &(lvstack->lvlist[-arg-1]);
	} else return 0;
    } else {
	return &(valuepool_item(arg));
    }
}

Type get_type(Argument arg)
{
    if (arg<0) {
	if (scope_count) {
	    /* parsing a function */
	    return localvar_item(-arg-1).type;
	} else if (lvstack && (-arg-1)<lvstack->length) {
	    return lvstack->lvlist[-arg-1].type;
	} else return 0;
    } else {
	return valuepool_item(arg).type;
    }
}


/* For debugging purposes: get the list of local variables */
char **get_local_names(void)
{
    char **retval;
    int i;
    retval = localname_copy();
    for (i=0; i<localname_max; i++) {
	localname_item(i)=0;
    }
    localname_clear();
    return retval;
}

void destroy_local_names(char **local_names)
{
    int i;
    for (i=0;local_names[i]; i++) free(local_names[i]);
    free(local_names);
}

/*
**  Arguments to functions and local variables:  negative values
**           (arguments and local variables are stored in a stack)
**  Global variables: positive values
*/
Argument lookup_variable(char *name)
{
    int in_scope = scope_count;
    int i,s;
    Variable *v;
    Variable tv;
    /* lookup local variable */
    i=localscope_max;
    while (i) {
	i--;
	s=localscope_item(i);
	if (s<in_scope) in_scope=s;
	if (s==in_scope) {
	    /* variable is in scope. Compare names */
	    if (!strcmp(name, localname_item(i))) {
		/* names equal. Found the local variable or argument */
		return (-i-1);
	    }
	}
    }
    /* lookup global */
    tv.name=name;
    v = LT_member(&allvars, &tv);
    if (!v) return 0;
    else return (v->value);
}

Type lookup_arg_type(Argument arg)
{
    if (arg<0) {
        return localvar_item(-arg-1).type;
    } else {
	return valuepool_item(arg).type;
    }
}

void variable_init(void)
{
    Value v;
    valuepool_add(v);
}
