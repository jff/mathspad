#include "language.h"


void assign(int y, int *x)
{
	*x=y;
}

void updatevar(int *x)
{
	*x=0;
}

void updateedit(int *x __attribute__((unused)))
{
}

void printint(char *str, int x)
{
	printf("%s: %i\n", str, x);
}

static int intadd(int i, int j)
{
  return i+j;
}
static int intminus(int i, int j)
{
  return i-j;
}
static int intgreater(int i, int j)
{
  return i>j;
}
static int intless(int i, int j)
{
  return i<j;
}

static void assign_lazy(void *s, void **t)
{
  *t = s;
}

static int call_lazy(int (*fcal)(), void **args)
{
  return (*fcal)(*((void**)args[0]));
}

static int call_lazylazyref(int (*fcal)(), void **args)
{
  return (*fcal)(*((void**)args[0]), *((void***)args[1]));
}


static int call_intref(int (*fcal)(), void **args)
{
   return (*fcal)(*((int**)args[0]));
}
static int call_intintref(int (*fcal)(), void **args)
{
   return (*fcal)(*((int*)args[0]), *((int**)args[1]));
}
static int call_intintintref(int (*fcal)(), void **args)
{
   return (*fcal)(*((int*)args[0]), *((int*)args[1]), *((int**)args[2]));
}
static int call_intintresint(int (*fcal)(), void **args)
{
    *(*((int**)args[2])) = (*fcal)(*((int*)args[0]), *((int*)args[1]));
    return 0;
}
static int call_strint(int (*fcal)(), void **args)
{
   return (*fcal)(*((char**)args[0]), *((int*)args[1]));
}

static Type tlist[6][4] = {
   {IntType, ToRefType(IntType), 0},
   {StringType,	IntType, 0},
   {IntType, IntType, ToRefType(IntType),0 },
   {IntType, IntType, 0},
   {LazyEvalType, 0},
   {LazyEvalType, ToRefType(LazyEvalType), 0 },
};

int main(int argc __attribute__((unused)), char *argv[])
{
    char *b;
    Prototype *pt;
    FuncRef fr;

    unitype_init();
    
    UConvLoadDatabase("UniConvert");
    
    variable_init();
    pt=define_prototype(tlist[0], 1, 0, call_intref);
    define_function("UpdateVar", "", pt, (Function) updatevar);
    define_function("UpdateEdit", "", pt, (Function) updateedit);
    pt=define_prototype(tlist[1], 2, 0, call_strint);
    define_function("PrintInt", "", pt, (Function) printint);
    pt=define_prototype(tlist[0], 2, 0, call_intintref);
    fr = define_function("Assign", "", pt, (Function) assign);
    define_operator(OPASSIGN, fr);
    pt=define_prototype(tlist[2], 2, IntType, call_intintresint);
    fr = define_function("IntAdd", "", pt, (Function) intadd);
    define_operator(OPADD, fr);
    fr = define_function("IntMin", "", pt, (Function) intminus);
    define_operator(OPSUB, fr);
    fr = define_function("IntGreater", "", pt, (Function) intgreater);
    define_operator(OPGREATER, fr);
    fr = define_function("IntLess", "", pt, (Function) intless);
    define_operator(OPLESS, fr);
    pt=define_prototype(tlist[4], 1, 0, call_lazy);
    fr = define_function("LazyEval", "", pt, (Function) calculate_lazy_expression);
    pt=define_prototype(tlist[5], 2, 0, call_lazylazyref);
    fr = define_function("AssignLazyEval", "", pt, (Function) assign_lazy);
    define_operator(OPASSIGN, fr);
    b=malloc(sizeof(char)*(strlen(argv[1])+1));
    strcpy(b,argv[1]);
    lex_open_file(b);
    parse_input();
    return 0;
}
