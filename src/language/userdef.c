#include <stdlib.h>
#include <stdio.h>



#include "userdef.h"
#include "sequence.h"
#include "leaftree.h"
#include "variable.h"
#include "expression.h"

/* userdefined functions */
typedef struct {
    char *name;          /* UTF string */
    char *description;   /* UTF string */
    char nr_args;        /* number of arguments (<16) */
    char functype;
    Type rettype;
    Prototype *prototype; /* prototype for this function */
    char nr_local;       /* number of local variables + arguments */
                         /* list of arguments and */
    Value *localvar;     /* local variables  (static) */
    char **localname;    /* list of local names */
    Sequence *sequence;  /* sequence of functions to call */
} UserDef;

static void userdef_free(void *udef)
{
    UserDef *usd=(UserDef*)udef;
    free(usd->name);
    free(usd->description);
    free(usd->localvar);
    destroy_local_names(usd->localname);
    free_sequence(usd->sequence);
}

static int userdef_cmp(const void *func1, const void *func2)
{
    return strcmp(((UserDef*)func1)->name, ((UserDef*)func2)->name);
}

static LeafTree userfuncpool = {0,userdef_cmp, userdef_free,
				sizeof(UserDef) };
#define userpool_add(D)  LT_insert(&userfuncpool, &(D))
#define userpool_member(D) LT_member(&userfuncpool, &(D))
#define userpool_found     LT_found_leaf()

FuncRef lookup_user_function(char *name)
{
    UserDef udef;
    udef.name=name;
    if (userpool_member(udef)) {
	return userpool_found;
    } else {
	return 0;
    }
}

static Value dummyval;

#define TypeSize(T)    (sizeof(TVal))

int call_user_function(FuncRef userdef, Value **alist)
{
    UserDef *udef = (UserDef *) userdef;
    int i;
    Value *lv;
    
    /* make a copy of the local variables */
    lv = (Value*) malloc(sizeof(Value)*(udef->nr_local));
    memcpy(lv, udef->localvar, sizeof(Value)*udef->nr_local);
    for (i=0; i<udef->nr_args; i++) {
        CopyValue(&lv[i], alist[i]);
	/*
	if (IsRef(lv[i].type)) {
	    if (IsRef(alist[i]->type)) {
		lv[i].val.stref=alist[i]->val.stref;
	    } else {
		lv[i].val.stref=(void*) (&(alist[i]->val));
	    }
	} else {
	    if (IsRef(alist[i]->type)) {
		memcpy(&lv[i].val, alist[i]->val.stref, TypeSize(lv[i].type));
	    } else {
		lv[i]=*alist[i];
		lv[i].type=udef->localvar[i].type;
	    }
	}
	*/
    }
    for (;i<udef->nr_local;i++)
      ConstructValue(&lv[i]);
    /* push the local variables to the stack */
    push_local_variables(lv, udef->nr_local);
    /* execute the sequence */
    push_eval_stack();
    run_sequence(udef->sequence);
    /* pop local variable */
    pop_eval_stack();
    pop_local_variables();
    for (i=0; i<udef->nr_local; i++)
      DestructValue(&lv[i]);
    free(lv);
    return 0;
}

int user_define(char *name, char *description, Prototype *prototype,
		int nr_local,
		Value *localvar, char **localname, Sequence *sequence)
{
    UserDef udef;
    udef.name=name;
    udef.description=description;
    udef.nr_args=ProtoArgs(prototype);
    udef.rettype=ProtoResult(prototype);
    udef.prototype=prototype;
    udef.localvar=localvar;
    udef.localname=localname;
    udef.sequence=sequence;
    udef.nr_local=nr_local;
    userpool_add(udef);
    return 1;
}

void user_destroy(FuncRef udef)
{
    userdef_free(udef);
}

Prototype *proto_user(FuncRef function)
{
    return ((UserDef*) function)->prototype;
}

Type restype_user_function(FuncRef function)
{
    return ((UserDef*)function)->rettype;
}
