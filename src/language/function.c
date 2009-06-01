#include <stdlib.h>
#include <stdio.h>
#include "function.h"
#include "variable.h"
#include "leaftree.h"


/* For make internal function available to the language
** Prototypes are used to check if variables are passed correctly.
*/

typedef struct {
    char *name;        /* UTF8 (easier to write in source files) */
    char *description; /* UTF8 (easier to write in source files) */
    char nr_args;
    char functype;
    Type restype;         /* type of the result, passed as an argument */ 
    Prototype *prototype; /* contains a function to call funcptr */
    void (*funcptr)();    /* function that performs the action */
} FunctionDef;

/* to report errors */
int error_status=0;

static void func_free(void *var __attribute__((unused)))
{
    /* name and description are pointers to build-in strings
    ** prototype is shared between functions.
    */
}

static int func_cmp(const void *func1, const void *func2)
{
    return strcmp(((FunctionDef*)func1)->name, ((FunctionDef*)func2)->name);
}

static LeafTree funcpool = { 0, func_cmp, func_free, sizeof(FunctionDef) };
#define funcpool_add(fdef) LT_insert(&funcpool, &fdef)
#define funcpool_member(fdef) LT_member(&funcpool,&fdef)
#define funcpool_found        LT_found_leaf()

/* Define an internal function. */
FuncRef define_function(char *name, char *description,
			Prototype *prototype, void (*funcptr)())
{
    FunctionDef fdef;
    fdef.name=name;
    fdef.description=description;
    fdef.nr_args=ProtoArgs(prototype);
    fdef.restype=ProtoResult(prototype);
    fdef.prototype=prototype;
    fdef.funcptr=funcptr;
    funcpool_add(fdef);
    return (FuncRef) funcpool_found;
}
    
/* Lookup an internal function  */
FuncRef lookup_function(char *name)
{
    FunctionDef fdef;
    fdef.name=name;
    if (funcpool_member(fdef)) {
	return funcpool_found;
    } else {
	return 0;
    }
}

Prototype *proto_function(FuncRef function)
{
    return (Prototype*) (function?function->prototype:0);
}

Type restype_function(FuncRef function)
{
    return ((FunctionDef*)function)->restype;
}

#define BadTypeError 1
#define NoProtoCaller 2
/* call internal function */
int call_function(FuncRef function, Value **list)
{
    Type t;
    FunctionDef *fdef = (FunctionDef*) function;
    void *refval[50];
    void *argval[50];
    Value *v;
    int i,n;
    /* call no functions until the error_status is cleared */
    if (error_status) return error_status;
#ifdef DEBUG
    fprintf(stderr, "%s (%s) %i %i\n",
	    fdef->name, fdef->description, fdef->nr_args, fdef->functype);
#endif
    if (!ProtoCaller(fdef->prototype)) {
	error_status = NoProtoCaller;
	return NoProtoCaller;
    }
    n=ProtoArgs(fdef->prototype);
    for (i=0; i<n; i++) {
	v=list[i];
	t = TypeNeeded(fdef->prototype,i);
        if (!TypeMatch(t, v->type)) {
	    /* types should always match, since the prototypes are checked
	    ** at load/parse/compile time.
	    */
	    error_status = BadTypeError;
	    return BadTypeError;
	}
	/* convert to correct type */
	if (IsRef(t)) {
	    /* convert to reference type */
	    if (IsRef(v->type)) {
		argval[i]=&v->val;
	    } else {
		refval[i]=&v->val;
		argval[i]=&refval[i];
	    }
	} else {
	    if (IsRef(v->type)) {
		argval[i]= *((void**)(&v->val));
	    } else {
		argval[i]=&v->val;
	    }
	}
    }
    t=fdef->restype;
    if (t) {
      v=list[i];
      t=ToRefType(t);
      if (IsRef(v->type)) {
	argval[i]=&v->val;
      } else {
	refval[i]=&v->val;
	argval[i]=&refval[i];
      }
    }	
    (ProtoCaller(fdef->prototype))(fdef->funcptr, &argval);
    return error_status;
}

