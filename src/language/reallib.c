
#include <stdlib.h>
#include <stdio.h>
#include "language.h"
#include "unistring.h"
#include <math.h>

#include "powfix.h"

static void assigndouble(double y, double *x)
{
    *x=y;
}

static void updatevardouble(double *x)
{
    printf("Updated variable %p\n", x);
}

static void updateeditdouble(double *x)
{
    printf("Updated editfield for %p\n", x);
}

static void printdouble(Uchar *str, double x)
{
    printf("%s: %lg\n", UstrtoLocale(str), x);
}

static double doubleadd(double i, double j)
{
  return i+j;
}
static double doubleminus(double i, double j)
{
  return i-j;
}
static double doublemult(double i, double j)
{
  return i*j;
}
static double doubledivide(double i, double j)
{
  return i/j;
}
static double doubleremain(double i, double j)
{
  return fmod(i,j);
}
/*
static double doubleand(double i, double j)
{
  return i&j;
}
static double doubleor(double i, double j)
{
  return i|j;
}
*/
static double doublexor(double i, double j)
{
  /* pow is part of libm.so
  **
  ** To make pow available, the option -lm has to be added to the program
  ** using dlopen, to make sure that the libm.so is already opened.
  ** The difference between RTLD_LAZY, RTLD_NOW and RTLD_GLOBAL is
  ** not clear to me.
  ** If I understand the dlopen documentation correctly, libm.so would
  ** automatically be loaded if this library is loaded and linked with
  ** -lm.  Somehow that does not work.
  */
  return pow(i,j);
}
static double doublenot(double i)
{
    return  -i;
}
static double doubleland(double i, double j)
{
  return i && j;
}
static double doublelor(double i, double j)
{
  return i || j;
}
static double doublelxor(double i, double j)
{
  return (i&&!j)||(!i&&j);
}
static double doublelnot(double i)
{
  return !i;
}
static double doubleequal(double i, double j)
{
  return i==j;
}
static double doublenotequal(double i, double j)
{
  return  i != j;
}
static double doublegreater(double i, double j)
{
  return  i>j;
}
static double doubleless(double i, double j)
{
  return  i<j;
}
static double doublelessequal(double i, double j)
{
  return  i<=j;
}
static double doublegreaterequal(double i, double j)
{
  return  i>=j;
}
static double doubleshiftleft(double i, double j)
{
  /* pow is part of libm.so, see earlier comment */
  return  i*pow(2.0,j);
}
static double doubleshiftright(double i, double j)
{
  /* pow is part of libm.so, see earlier comment */
  return  i*pow(0.5,j);
}

static int call_doubleref(int (*fcal)(), void **args)
{
   return (*fcal)(*((double**)args[0]));
}
static int call_doubledoubleref(int (*fcal)(), void **args)
{
   return (*fcal)(*((double*)args[0]), *((double**)args[1]));
}
static int call_doubledoubledoubleref(int (*fcal)(), void **args)
{
   return (*fcal)(*((double*)args[0]), *((double*)args[1]), *((double**)args[2]));
}
static int call_doubleresdouble(double (*fcal)(), void **args)
{
   *(*((double**)args[1]))= (*fcal)(*((double*)args[0]));
   return 0;
}
static int call_doubledoubleresdouble(double (*fcal)(), void **args)
{
   *(*((double**)args[2]))= (*fcal)(*((double*)args[0]), *((double*)args[1]));
}
static int call_strdouble(int (*fcal)(), void **args)
{
   return (*fcal)(*((Uchar**)args[0]), *((double*)args[1]));
}

static Type tlist[5][4] = {
   {RealType, ToRefType(RealType), 0},
   {StringType,	RealType, 0},
   {RealType, RealType, ToRefType(RealType),0 },
   {RealType, RealType, 0},
   {RealType, 0}
};

int init_library(void)
{
    Prototype *pt;
    FuncRef fr;

    pt=define_prototype(tlist[0], 1, 0, call_doubleref);
    define_function("UpdateVarReal", "", pt, (Function) updatevardouble);
    define_function("UpdateEditReal", "", pt, (Function) updateeditdouble);
    pt=define_prototype(tlist[1], 2, 0, call_strdouble);
    define_function("PrintReal", "", pt, printdouble);
    pt=define_prototype(tlist[0], 2, 0, call_doubledoubleref);
    fr = define_function("AssignReal", "", pt, (Function) assigndouble);
    define_operator(OPASSIGN, fr);
    pt=define_prototype(tlist[4], 1, RealType, call_doubleresdouble);
    fr = define_function("RealNot", "", pt, (Function) doublenot);
    define_operator(OPNOT, fr);
    fr = define_function("RealLogicNot", "", pt, (Function) doublelnot);
    define_operator(OPLOGICNOT, fr);
    pt=define_prototype(tlist[3], 2, RealType, call_doubledoubleresdouble);
    fr = define_function("RealAdd", "", pt, (Function) doubleadd);
    define_operator(OPADD, fr);
    fr = define_function("RealMin", "", pt, (Function) doubleminus);
    define_operator(OPSUB, fr);
    fr = define_function("RealMult", "", pt, (Function) doublemult);
    define_operator(OPMULT, fr);
    fr = define_function("RealDivide", "", pt, (Function) doubledivide);
    define_operator(OPDIV, fr);
    fr = define_function("RealRemain", "", pt, (Function) doubleremain);
    define_operator(OPREMAIN, fr);
    /*
    fr = define_function("RealAnd", "", pt, (Function) doubleand);
    define_operator(OPAND, fr);
    fr = define_function("RealOr", "", pt, (Function) doubleor);
    define_operator(OPOR, fr);
    */
    fr = define_function("RealXor", "", pt, (Function) doublexor);
    define_operator(OPXOR, fr);
    fr = define_function("RealLogicAnd", "", pt, (Function) doubleland);
    define_operator(OPLOGICAND, fr);
    fr = define_function("RealLogicOr", "", pt, (Function) doublelor);
    define_operator(OPLOGICOR, fr);
    fr = define_function("RealLogicXor", "", pt, (Function) doublelxor);
    define_operator(OPLOGICXOR, fr);
    fr = define_function("RealEqual", "", pt, (Function) doubleequal);
    define_operator(OPEQUAL, fr);
    fr = define_function("RealNotEqual", "", pt, (Function) doublenotequal);
    define_operator(OPNOTEQUAL, fr);
    fr = define_function("RealGreater", "", pt, (Function) doublegreater);
    define_operator(OPGREATER, fr);
    fr = define_function("RealLess", "", pt, (Function) doubleless);
    define_operator(OPLESS, fr);
    fr = define_function("RealLessEqual", "", pt, (Function) doublelessequal);
    define_operator(OPLESSEQUAL, fr);
    fr = define_function("RealGreaterEqual", "", pt, (Function) doublegreaterequal);
    define_operator(OPGREATEREQUAL, fr);
    fr = define_function("RealShiftLeft", "", pt, (Function) doubleshiftleft);
    define_operator(OPSHIFTLEFT, fr);
    fr = define_function("RealShiftRight", "", pt, (Function) doubleshiftright);
    define_operator(OPSHIFTRIGHT, fr);
    return 1;
}
