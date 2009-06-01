
#include "language.h"
#include "unistring.h"
#include "refcounting.h"

typedef struct {
   int t;
   Uchar *value;
} Rubbish;

typedef Rubbish *RubRef;

static void construct_rubbish(void **rub)
{
  Rubbish *a;
  a = malloc(sizeof(Rubbish));
  a->t=0;
  a->value=0;
  *rub=a;
}

static void copy_rubbish(void **rubd, void *rubs)
{
  Rubbish *a;
  Rubbish *b = (Rubbish*) rubs;
  a = malloc(sizeof(Rubbish));
  a->t=b->t;
  a->value=b->value;
  increase_refcount(a->value,free);
  *rubd=a;
}

static void destroy_rubbish(void *rub)
{
  Rubbish *b = (Rubbish*) rub;
  decrease_refcount(b->value);
  free(b);
}

static void assignint(int y, RubRef *x)
{
    (*x)->t=y;
}

static void assignstr(Uchar *y, RubRef *x)
{
    Uchar *oldval;
    oldval=(*x)->value;
    (*x)->value=y;
    increase_refcount(y,free);
    decrease_refcount(oldval);
}

static void assignrub(RubRef y, int *x)
{
    *x = y->t;
}

static void assignrubrub(RubRef y, RubRef *x)
{
    (*x)->t = y->t;
}

static void printrubref(RubRef x)
{
    printf("%s: %i\n", (x->value?UstrtoLocale(x->value):"???"), x->t);
}

static void rubadd(RubRef i, RubRef j, RubRef *res)
{
  (*res)->t=i->t+j->t;
}

static int call_rubref(int (*fcal)(), void **args)
{
   return (*fcal)(*((RubRef*)args[0]));
}
static int call_rubrefrubreff(int (*fcal)(), void **args)
{
   return (*fcal)(*((RubRef*)args[0]), *((RubRef**)args[1]));
}

static int call_intrubreff(int (*fcal)(), void **args)
{
   return (*fcal)(*((int*)args[0]), *((RubRef**)args[1]));
}
static int call_rubrefintref(int (*fcal)(), void **args)
{
   return (*fcal)(*((RubRef*)args[0]), *((int**)args[1]));
}
static int call_rubrefrubrefrubreff(int (*fcal)(), void **args)
{
   return (*fcal)(*((RubRef*)args[0]), *((RubRef*)args[1]),
		  *((RubRef**)args[2]));
}
static int call_strrubreff(int (*fcal)(), void **args)
{
   return (*fcal)(*((Uchar**)args[0]), *((RubRef**)args[1]));
}

static char *tname[7][4] = {
   {"Rubbish", 0},
   {"Rubbish", "RubbishRef", 0},
   {"Int", "RubbishRef", 0},
   {"Rubbish", "IntRef", 0},
   {"String", "RubbishRef", 0},
   {"Rubbish", "Rubbish", "RubbishRef", 0},
   {"Rubbish", "Rubbish", 0}
};
static Type tlist[7][4];
static Type rubtype;

int init_library(void)
{
    Prototype *pt;
    FuncRef fr;
    int i,j;


    rubtype = define_type("Rubbish", sizeof(void*), construct_rubbish,
			  destroy_rubbish, copy_rubbish);
    for (i=0; i<7; i++)
      for (j=0; tname[i][j]; j++) {
	tlist[i][j] = lookup_type(tname[i][j]);
      }
    pt=define_prototype(tlist[0], 1, 0, call_rubref);
    define_function("PrintRubbish", "", pt, printrubref);
    pt=define_prototype(tlist[1], 2, 0, call_rubrefrubreff);
    fr = define_function("AssignRR", "", pt, assignrubrub);
    define_operator(OPASSIGN, fr);

    pt=define_prototype(tlist[2], 2, 0, call_intrubreff);
    fr = define_function("AssignIR", "", pt, assignint);
    define_operator(OPASSIGN, fr);

    pt=define_prototype(tlist[3], 2, 0, call_rubrefintref);
    fr = define_function("AssignRI", "",pt, assignrub);
    define_operator(OPASSIGN, fr);

    pt=define_prototype(tlist[4], 2, 0, call_strrubreff);
    fr = define_function("AssignSR", "", pt, assignstr);
    define_operator(OPASSIGN, fr);

    pt=define_prototype(tlist[5], 2, rubtype, call_rubrefrubrefrubreff);
    fr = define_function("RubAdd", "", pt, rubadd);
    define_operator(OPADD, fr);
    return 1;
}
