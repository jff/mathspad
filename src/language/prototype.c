#include <stdlib.h>
#include <stdio.h>

#include "prototype.h"
#include "leaftree.h"

/* a collection of predefined prototypes. */
static Type ptempty[] = {0};
static Type ptint[] = {IntType};
static Type ptstr[] = {StringType};

#define NoCaller 0

static int empcall(int (*fcal)(), void **args __attribute__((unused)))
{
    return (*fcal)();
}

static int refcall(int (*fcal)(), void **args)
{
    return (*fcal)(*((void**)args[0]));
}

static int intcall(int(*fcal)(), void **args)
{
    return (*fcal)(*((int*)args[0]));
}

Prototype protopredef[] =
{ {ptempty, 0,0,1,empcall},
  {ptint,   1,0,1,intcall},
  {ptstr,   1,0,1,refcall},
  {0,       0,0,0,NoCaller}
};

/* compare two Prototypes */
static int protocmp(const void *pr1, const void *pr2)
{
    int i=0,l;
    Prototype *p1= (Prototype*)pr1;
    Prototype *p2= (Prototype*)pr2;
    if (p1->n < p2->n) l=p1->n; else l=p2->n;
    while (i<l && p1->type[i]==p2->type[i]) i++;
    if (i==l) {
      if (p1->n==p2->n) {
	if (p1->restype==p2->restype)
	  return 0;
	else
	  return (p1->restype < p2->restype ? -1 : 1);
      } else {
	return p1->n - p2->n;
      }
    } else
      return (p1->type[i] < p2->type[i] ? -1:1);
}

/* prototypes are never released */
static LeafTree prototree = { 0,
			      protocmp,
			      0,
			      sizeof(Prototype)
                            };

Prototype *define_prototype(const Type *typelist, const int n,
			    const Type restype, int (*caller)())
{
    Prototype p;
    p.type = (Type*) typelist;
    p.n=n;
    p.restype=restype;
    p.max=n;
    p.caller=caller;
    if (!LT_member(&prototree, &p)) {
	p.type = malloc(sizeof(Type)*(n+1));
	memcpy(p.type, typelist, sizeof(Type)*n);
	p.type[n]=0;
	LT_insert(&prototree, &p);
    }
    return LT_found_leaf();
}

Prototype *make_value_prototype(const Value *valuelist, const int n,
				Type result, int (*caller)())
{
    Type typelist[MAX_PROTOTYPE];
    int i;
    for (i=0; i<n; i++) typelist[i]=valuelist[i].type;
    return define_prototype(typelist, n,result, caller);
}

Prototype *proto_add_to_database(Prototype *proto)
{
    Prototype *retval=0;
    if (proto) {
	if (!((retval = LT_member(&prototree, proto)))) {
	    Prototype p;
	    p.n=p.max=proto->n;
	    p.restype=proto->restype;
	    p.caller=proto->caller;
	    p.type=malloc(sizeof(Type)*p.n);
	    memcpy(p.type, proto->type, sizeof(Type)*p.n);
	    LT_insert(&prototree, &p);
	    retval=LT_found_leaf();
	}
    }
    return retval;
}

Prototype *create_prototype(int (*caller)())
{
    Prototype *p;
    p = malloc(sizeof(Prototype));
    if (p) {
	p->type=0;
	p->n=0;
	p->restype=0;
	p->max=0;
	p->caller=caller;
    }
    return p;
}

int add_type(Prototype *p, const Type type)
{
    if (p->n <= p->max) {
	int ns=p->max+4;
	Type *nt;
	if (p->type) {
	    nt=realloc(p->type, sizeof(Type)*ns);
	} else {
	    nt=malloc(sizeof(Type)*ns);
	}
	if (!nt) return 1;
	p->type=nt;
	p->max=ns;
    }
    p->type[p->n++]=type;
    return 0;
}

int add_result_type(Prototype *prototype, const Type type)
{
  if (prototype) {
    prototype->restype=type;
  }
}

void destroy_prototype(Prototype *p)
{
    if (p) {
	if (p->type) free(p->type);
	free(p);
    }
}

int matches_prototype(Type *typelist, Prototype *p, int nr)
{
    int i;
    for (i=0; i<nr; i++) {
	if (!(TypeMatch(typelist[i],p->type[i]))) return 0;
    }
    return 1;
}

void protoinit(void)
{
    int i;
    i=0;
    while (protopredef[i].max) {
	LT_insert(&prototree, &protopredef[i]);
	i++;
    }
}
