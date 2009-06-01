
#include "language.h"
#include "unistring.h"

static void assign(int y, int *x)
{
    *x=y;
}

static void updatevar(int *x)
{
    printf("Updated variable %p\n", x);
}

static void updateedit(int *x)
{
    printf("Updated editfield for %p\n", x);
}

static void printint(Uchar *str, int x)
{
    printf("%s: %i\n", UstrtoLocale(str), x);
}

static int intadd(int i, int j)
{
  return i+j;
}
static int intminus(int i, int j)
{
  return i-j;
}
static int intmult(int i, int j)
{
  return i*j;
}
static int intdivide(int i, int j)
{
  return i/j;
}
static int intremain(int i, int j)
{
  return i%j;
}
static int intand(int i, int j)
{
  return i&j;
}
static int intor(int i, int j)
{
  return i|j;
}
static int intxor(int i, int j)
{
  return i^j;
}
static int intnot(int i)
{
    return ~i;
}
static int intland(int i, int j)
{
  return i&&j;
}
static int intlor(int i, int j)
{
  return i||j;
}
static int intlxor(int i, int j)
{
  return (i&&!j)||(!i&&j);
}
static int intlnot(int i)
{
  return !i;
}
static int intequal(int i, int j)
{
  return i==j;
}
static int intnotequal(int i, int j)
{
  return i!=j;
}
static int intgreater(int i, int j)
{
  return i>j;
}
static int intless(int i, int j)
{
  return i<j;
}
static int intlessequal(int i, int j)
{
  return i<=j;
}
static int intgreaterequal(int i, int j)
{
  return i>=j;
}
static int intshiftleft(int i, int j)
{
  return i<<j;
}
static int intshiftright(int i, int j)
{
  return i>>j;
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

static int call_intresint(int (*fcal)(), void **args)
{
  *(*((int**)args[1])) = (*fcal)(*((int*)args[0]));
  return 0;
}

static int call_strint(int (*fcal)(), void **args)
{
   return (*fcal)(*((Uchar**)args[0]), *((int*)args[1]));
}

static Type tlist[5][4] = {
   {IntType, ToRefType(IntType), 0},
   {StringType,	IntType, 0},
   {IntType, IntType, ToRefType(IntType),0 },
   {IntType, IntType, 0},
   {IntType, 0}
};

int init_library(void)
{
    Prototype *pt;
    FuncRef fr;

    pt=define_prototype(tlist[0], 1, 0, call_intref);
    define_function("UpdateVar", "", pt,(Function)  updatevar);
    define_function("UpdateEdit", "", pt, (Function) updateedit);
    pt=define_prototype(tlist[1], 2, 0, call_strint);
    define_function("PrintInt", "", pt, (Function) printint);
    pt=define_prototype(tlist[0], 2, 0, call_intintref);
    fr = define_function("Assign", "", pt, (Function) assign);
    define_operator(OPASSIGN, fr);
    pt=define_prototype(tlist[0], 1, IntType, call_intresint);
    fr = define_function("IntNot", "", pt, (Function) intnot);
    define_operator(OPNOT, fr);
    fr = define_function("IntLogicNot", "", pt, (Function) intlnot);
    define_operator(OPLOGICNOT, fr);
    pt=define_prototype(tlist[2], 2, IntType, call_intintresint);
    fr = define_function("IntAdd", "", pt, (Function) intadd);
    define_operator(OPADD, fr);
    fr = define_function("IntMin", "", pt, (Function) intminus);
    define_operator(OPSUB, fr);
    fr = define_function("IntMult", "", pt, (Function) intmult);
    define_operator(OPMULT, fr);
    fr = define_function("IntDivide", "", pt, (Function) intdivide);
    define_operator(OPDIV, fr);
    fr = define_function("IntRemain", "", pt, (Function) intremain);
    define_operator(OPREMAIN, fr);
    fr = define_function("IntAnd", "", pt, (Function) intand);
    define_operator(OPAND, fr);
    fr = define_function("IntOr", "", pt, (Function) intor);
    define_operator(OPOR, fr);
    fr = define_function("IntXor", "", pt, (Function) intxor);
    define_operator(OPXOR, fr);
    fr = define_function("IntLogicAnd", "", pt, (Function) intland);
    define_operator(OPLOGICAND, fr);
    fr = define_function("IntLogicOr", "", pt, (Function) intlor);
    define_operator(OPLOGICOR, fr);
    fr = define_function("IntLogicXor", "", pt, (Function) intlxor);
    define_operator(OPLOGICXOR, fr);
    fr = define_function("IntEqual", "", pt, (Function) intequal);
    define_operator(OPEQUAL, fr);
    fr = define_function("IntNotEqual", "", pt, (Function) intnotequal);
    define_operator(OPNOTEQUAL, fr);
    fr = define_function("IntGreater", "", pt, (Function) intgreater);
    define_operator(OPGREATER, fr);
    fr = define_function("IntLess", "", pt, (Function) intless);
    define_operator(OPLESS, fr);
    fr = define_function("IntLessEqual", "", pt, (Function) intlessequal);
    define_operator(OPLESSEQUAL, fr);
    fr = define_function("IntGreaterEqual", "", pt, (Function) intgreaterequal);
    define_operator(OPGREATEREQUAL, fr);
    fr = define_function("IntShiftLeft", "", pt, (Function) intshiftleft);
    define_operator(OPSHIFTLEFT, fr);
    fr = define_function("IntShiftRight", "", pt, (Function) intshiftright);
    define_operator(OPSHIFTRIGHT, fr);
    return 1;
}
