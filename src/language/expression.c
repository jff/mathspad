#include <stdlib.h>
#include <stdio.h>

#include "types.h"
#include "variable.h"
#include "userdef.h"
#include "function.h"
#include "expression.h"

#ifndef NULL
#define NULL ((void*)0)
#endif

static char *op_name[] = {
  "?",
  ":=",  "[]", "?", "?", "?", "?", "?", "?", "?", "?",
  "+", "-", "*", "/", "%", "?", "?", "?", "?", "?",
  "&&", "||", "^^", "!", "=", "!=", ">", "<", "<=", ">=",
  "?", "?", "?", "?", "?", "?", "?", "?", "?", "?",
  "&", "|", "^", "~", "<<", ">>", "?", "?", "?", "?" };

/* an operator could be an enumerated type */
#define Operator int
#define OpType 1
#define ArgType 2
#define IntFuncType 3
#define UserFuncType 4
#define LazyType 5

/* An Expression contains an expression in reverse polish notation
** functions are provided to calculate the result of an Expression.
** Assignments and function calls are also handled as expressions.
*/

typedef struct OpStruct OpStruct;

struct OpStruct {
  Operator baseop;   /* +                 + */
  FuncRef ifunc;     /* Concat            Add */
  Type *list;        /* String,String     Int,Int */
  Type restype;      /* String            Int */
  OpStruct *next;
};

/* These operators are used as  "First" + "Second"  or 5 + 6
** The functions are used as  Concat("First","Second") or Add(5,6)
** Internally, it calls StrCat("First","Second",res) or add(5,6,res)
** where res is a temporary variable.
** Each function has a call function, which might translate the above
** into  res=strcat("First","Second")  or res=5+6
*/

static OpStruct *oplist=NULL;

#ifdef DEBUG
static void print_expression(Expression *e)
{
    printf("Expression\n");
    while (e) {
	switch (e->type) {
	case ArgType:
	    printf("  push %i (%i)\n", e->val.arg, e->restype);
	    break;
	case LazyType:
	    printf("  push lazy (%i)\n", e->restype);
	    break;
	case OpType:
	    if (!e->val.op) {
		printf("  Deref previous\n");
		break;
	    }
	case IntFuncType:
	case UserFuncType:
	    printf("  call %s (res:%i, d:%i)\n", e->val.ifunc->name,
		   e->restype, e->delta);
	    break;
	}
	e=e->next;
    }
}
#endif

/* make an expression for only one argument */
Expression *make_expression(Argument arg)
{
  Expression *te;
  te= (Expression*) malloc(sizeof(Expression));
  if (!te) return NULL;
  te->type=ArgType;
  te->delta=0;
  te->restype=get_type(arg);
  te->val.arg=arg;
  te->next=NULL;
  return te;
}

static int typelisteq(Type *l1, Type *l2)
{
    if (!l1 || !l2) return 0;
    while (*l1 && (TypeMatch(*l1,*l2))) {
	l1++;l2++;
    }
    return (!*l1 && !*l2);
}

static OpStruct *get_operator(Type t1, Operator op, Type t2)
{
  OpStruct *ol = oplist;
  Type tlist[3];

  tlist[0]=t1;
  tlist[1]=t2;
  tlist[2]=0;
  while (ol) {
    if ( ol->baseop==op && typelisteq(tlist, ol->list))
      return ol;
    ol=ol->next;
  }
  return 0;
}


/* combine two expression with an operator. Unary operators
** only use the first argument.
** The function automatically chooses that operator that correctly
** converts the sub expressions to the correct types.
** If no operator is given, the two expressions are combined into
** a list. If only the first expression is given and no operator,
** then an evaluate operator is added if necessary, which ensures
** that the value is placed in a stack variable (which needed for
** concurrent assignments).
*/
Expression *combine_expression(Expression *e1, Expression *e2,
			       Operator op)
{
  Expression *te, *tf;
  OpStruct *os;
  int i;
  Type *restype;
  Type t[3];
  if (!op) {
      i=1;
      te=e1;
      while (1) {
	  if (te->type==ArgType) i=1;
	  else i=0;
	  if (!te->next) break;
	  te=te->next;
      }
      if (!e2 && i) {
	  te->next=malloc(sizeof(Expression));
	  te->next->restype=NoRefType(te->restype);
	  te=te->next;
	  te->type=OpType;
	  te->delta=-1;
	  te->val.op=0;
	  te->next=NULL;
      } else {
	  te->next=e2;
      }
      return e1;
  }
  restype = type_of_expression(e1);
  /* e1 could result in a list of expressions.
  ** a combination would use the last element from that list
  */
  for (i=0; restype[i]; i++);
  if (i) i--; /* else restype[0]==0 */
  t[0] = restype[i];
  if (e2) {
      restype = type_of_expression(e2);
      t[1]=restype[0];
      t[2]=0;
  } else t[1]=0;
  os=get_operator(t[0], op, t[1]);
  if (!os) {
      /* no operator for that type combination */
      fprintf(stderr, "Operator not defined for combination (%s %s %s)\n",
	      lookup_typename(t[0]), op_name[op], lookup_typename(t[1]));
      return NULL;
  }
  /* automatic type conversion should be added to both expressions */
  /* or operator functions have to be defined for all possible combinations */
  te = (Expression*) malloc(sizeof(Expression));
  te->type=OpType;
  te->delta=(t[1]>0)?-2:-1;
  te->val.ifunc=os->ifunc;
  te->restype=os->restype;
  tf=e1;
  while (tf->next) tf=tf->next;
  tf->next=e2;
  while (tf->next) tf=tf->next;
  tf->next=te;
  te->next=NULL;
  return e1;
}

/* concurrent assignment   (a,b,c:=c-b,a-c,b-a)
** NR: number of variables   3
** ALIST: list of variables  a,b,c
** ELIST: expression leaving values on the stack c-b,a-c,b-a
*/
Expression *assign_expression(int nr, Argument *alist, Expression *elist)
{
    /* assignment is a special operator */
    Type *tlist;
    Type ct;
    int i;
    Expression *e1;
    if ((i=left_on_stack(elist)) != nr) return 0;
    tlist = type_of_expression(elist);
    for (i=0; i<nr; i++) {
	ct = get_type(alist[i]);
	if (IsConst(ct)) {
	    fprintf(stderr, "Unable to make assignments to constants.\n");
	    return 0;
	}
	if (!get_operator(tlist[i], OPASSIGN, NoRefType(ct))) {
	    fprintf(stderr, "Assignment not defined for %s:=%s.\n",
		    lookup_typename(ct), lookup_typename(tlist[i]));
	    return 0;
	}
    }
    i=nr;
    while (i) {
	i--;
	e1=make_expression(alist[i]);
	e1=combine_expression(elist, e1, OPASSIGN);
	if (!e1) return 0;
	elist=e1;
    }
    return elist;
}

Expression *func_expression(Expression *argexp,
			    int num __attribute__((unused)),
			    FuncRef func, int user_func)
{
    Expression *te;
    int i;
    Type *restype;
    Type fres;
    Prototype *p;
    restype = type_of_expression(argexp);
    i=0;
    while (restype[i]) i++;
    if (user_func) {
	p=proto_user(func);
	fres=restype_user_function(func);
    } else {
	p=proto_function(func);
	fres=restype_function(func);
    }
    /* Pass if:
    **    * the number of arguments equals the number that prototype requires
    **    * the number of arguments is 1 less the number that the prototype
    **      requires, but the extra argument that is needed is equal to a
    **      reference to the resulttype of the function.
    */
    if (i!=ProtoArgs(p) &&
	(i+1!=ProtoArgs(p) || TypeNeeded(p,i)!=ToRefType(fres))) {
	return NULL;
    }
    if (matches_prototype(restype,p,i)) {
      if (argexp) {
	te=argexp;
	while (te->next) te=te->next;
	te->next=malloc(sizeof(Expression));
	te=te->next;
      } else {
	te = argexp = malloc(sizeof(Expression));
      }
      te->type=(user_func?UserFuncType:IntFuncType);
      te->restype=(i!=ProtoArgs(p)?0:fres);
      te->delta=-i;
      te->next=NULL;
      if (user_func)
	te->val.ufunc=func;
      else te->val.ifunc=func;
      return argexp;
    } else return NULL;
}

/* create a lazy expression from EXPR, such that EXPR is not evaluate
** now, but evaluated later, but in the current context (local variables)
*/
/* How is this achieved?
** Argument 0 has a special meaning and refers to the local variable
** list (located on the stack).  The resulting expression will consist
** of a reference to the lazy expression and argument 0.  The evaluator
** will combine the lazy expression with the local variables into one
** argument, which can be evaluated later.
*/
Expression *make_lazy_expression(Expression *expr)
{
    Expression *e1;
    e1=malloc(sizeof(Expression));
    e1->type=LazyType;
    e1->delta=0;
    e1->restype=LazyEvalType;
    e1->val.le=expr;
    e1->next=NULL;
    return e1;
}
/* The expression evaluation stack is a list of pointers to values
** and a list of temporary values for intermediate results.
** Since expressions can contain calls to user-defined functions, which
** themself use expressions, a stack of expression evaluation stacks
** is used.
*/

#define MAX_EESTACK 50
/* 50 positions is just a guess, but should suffice for most purposes.
** If temporary value n is used, list item n will point to it.
*/
typedef struct ExpEvaStack ExpEvaStack;
struct ExpEvaStack {
    Value tv[MAX_EESTACK];  /* temporary values */
    Value *vl[MAX_EESTACK]; /* value list */
    int tos;                /* top of stack */
    ExpEvaStack *prev;
};

static ExpEvaStack *ces=0; /* current expression/evaluation stack */
static ExpEvaStack *used_eestack=0; /* free list */

static void expr_stack(void)
{
#ifdef DEBUG
    int i;char *c;
    printf("Stack: \n");
    for (i=0; i<ces->tos; i++) {
	if (IsRef(ces->vl[i]->type)) c="\tRef"; else
        if (IsConst(ces->vl[i]->type)) c="\tCon"; else c="\t   ";
	printf("%s %x ", c, ces->vl[i]->type&0x1fff);
	switch (ces->vl[i]->type) {
	case (ToRefType(IntType)): printf("(%i)\n", *ces->vl[i]->val.iref); break;
	case (ToConstType(IntType)):
	case (IntType): printf("(%i)\n", ces->vl[i]->val.ival); break;
	default: printf("\n"); break;
	}
    }
#endif
}



static Value dummyval;

#define TypeSize(T)    (sizeof(Value)-((char*)&dummyval.val-(char*)&dummyval.type))

void calculate_expression(Expression *e)
{
    int d,i;
    ces->tos=0;
    expr_stack();
    while (e) {
	d=e->delta;
	switch (e->type) {
	case ArgType:
	    ces->vl[ces->tos]=get_value(e->val.arg);
	    if (e->restype) ces->tos++;
	    ces->tos+=d;
	    break;
	case LazyType:
	    {
		LazyExpression *le;
		ces->vl[ces->tos]=ces->tv+ces->tos;
		/* when will this be freed? */
		le=ces->tv[ces->tos].val.stval=malloc(sizeof(LazyExpression));
		le->expr=e->val.le;
		le->vlist=get_local_values(&le->len);
		ces->tv[ces->tos].type=e->restype;
		if (e->restype) ces->tos++;
	    }
	    break;
	case OpType:
	    if (e->delta==-1 && !e->val.op && e->restype) {
		/* dereference last element on stack */
		if (IsRef(ces->vl[ces->tos-1]->type)) {
		    ces->tv[ces->tos-1].type=e->restype;
		} else {
		    ces->tv[ces->tos-1].type=ces->vl[ces->tos-1]->type;
		}
		/* copy value */
		CopyValue(&(ces->tv[ces->tos-1]),ces->vl[ces->tos-1]);
		break;
	    }
	case IntFuncType:
	    if (e->restype) {
		ces->vl[ces->tos]=ces->tv+ces->tos;
		ces->tv[ces->tos].type=e->restype;
		ConstructValue(&(ces->tv[ces->tos]));
	    }
#ifdef DEBUG
	    printf("Calling function %p\n", e->val.ifunc);
#endif
	    call_function(e->val.ifunc, ces->vl+(ces->tos+e->delta));
	    /* clean up temporary values */
	    for (i=-1; i>=e->delta; i--) {
	      if (ces->vl[ces->tos+i]== &(ces->tv[ces->tos+i])) {
		DestructValue(&(ces->tv[ces->tos+i]));
	      }
	    }
	    /* move result */
	    if (e->restype) {
		ces->vl[ces->tos+d]=ces->tv+(ces->tos+d);
		ces->tv[ces->tos+d]=ces->tv[ces->tos];
		ces->tos++;
	    }
	    ces->tos+=d;
	    break;
	case UserFuncType:
	    if (e->restype) {
		ces->vl[ces->tos]=ces->tv+ces->tos;
		ces->tv[ces->tos].type=e->restype;
		ConstructValue(&(ces->tv[ces->tos]));
	    }
#ifdef DEBUG
	    printf("Calling user function %p\n", e->val.ufunc);
#endif
	    call_user_function(e->val.ufunc, ces->vl+(ces->tos+d));
	    /* clean up temporary values */
	    for (i=-1; i>=e->delta; i--) {
	      if (ces->vl[ces->tos+i]== &(ces->tv[ces->tos+i])) {
		DestructValue(&(ces->tv[ces->tos+i]));
	      }
	    }
	    /* move result */
	    if (e->restype) {
		ces->vl[ces->tos+d]=ces->tv+(ces->tos+d);
		ces->tv[ces->tos+d]=ces->tv[ces->tos];
		ces->tos++;
	    }
	    ces->tos+=d;
	    break;
	default:
	    break;
	}
	expr_stack();
	e=e->next;
    }
}


void calculate_lazy_expression(void *lazyexpr)
{
  LazyExpression *le = (LazyExpression*)lazyexpr;
  push_local_variables(le->vlist, le->len);
  push_eval_stack();
  calculate_expression(le->expr);
  clear_expr_stack();
  pop_eval_stack();
  pop_local_variables();
}

/* Clear the stack used to calculate an expression */
void clear_expr_stack(void)
{
    while (ces->tos) {
	ces->tos--;
	/* free value if needed */
	ces->vl[ces->tos]=NULL;
    }
}

/* return if last value on expression stack is non-zero. */
int non_zero_on_stack(void)
{
    if (ces->tos && ces->vl[ces->tos-1]->val.ival) {
	return 1;
    } else return 0;
}

void push_eval_stack(void)
{
    ExpEvaStack *h;
    if (used_eestack) {
	h=used_eestack;
	used_eestack=h->prev;
    } else {
	h=malloc(sizeof(ExpEvaStack));
    }
    h->tos=0;
    h->prev=ces;
    ces=h;
}

void pop_eval_stack(void)
{
    ExpEvaStack *h;
    clear_expr_stack();
    h=ces->prev;
    ces->prev=used_eestack;
    used_eestack=ces;
    ces=h;
}

void clear_eval_stack(void)
{
    while (ces) {
	pop_eval_stack();
    }
}

static Type tlist[1000];
/* returns a list of result types of an expression
** It returns a reference to a static variable, so it is not
** possible to call it two times and compare the two list
*/
Type *type_of_expression(Expression *e)
{
    int i=0;
    while (e) {
	if (e->delta>=0 && e->restype) {
	    tlist[i]=e->restype;
	    i=i+1+e->delta;
	} else if (e->delta<0) {
	    if (i<e->delta) {
                /* stack underflow, should not happen */
		return NULL;
	    }
	    i=i+e->delta;
	    if (e->restype) {
		tlist[i++]=e->restype;
	    }
	}
	e=e->next;
    }
    tlist[i]=0;
    return tlist;
}

/* return number of values on stack after expression is evaluated */
int left_on_stack(Expression *e)
{
    int res=0;
    while (e && res>=0) {
	res=res+e->delta;
	if (res<0) return -1;
	if (e->restype) res++;
	e=e->next;
    }
    return res;
}


/* free an expression */
void free_expression(Expression *e)
{
    Expression *h;
    if (!e) return;
    while (e) {
	h=e;
	e=e->next;
	free(h);
    }
}


/* define an operator */
void define_operator(Operator op, FuncRef function)
{
    OpStruct *no;
    Prototype *pr;
    no=malloc(sizeof(OpStruct));
    no->next=oplist;
    no->ifunc=function;
    pr=proto_function(function);
    no->list=pr->type;  /* should be hidden */
    no->restype=ProtoResult(pr);
    no->baseop=op;
    oplist=no;
}
