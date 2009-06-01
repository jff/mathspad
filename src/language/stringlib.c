#include "language.h"
#include "unistring.h"
#include "refcounting.h"
static Uchar *testvar=0;


/* Strings are similar to C strings. The main disadvantage is that
** operations that require the length of the string, such as array
** checking or concatenation, are slower.  Especially bound checking
** for accessing single characters require the length of the string,
** turning an O(1) operation into an O(n) operation.
**
** The best solution would be to extend the string type by adding
** the length of the string to it.  However, this complicates
** accessing program variables which use C-style string.
**
** For the moment, bound checking is disabled in character access.
** So, the bound checking should be done in the interpreted language.
*/

static void assign(Uchar *s, Uchar **t)
{
    int i;
    Uchar *oldt;
    i = Ustrlen(s)+1;
    oldt=*t;
    /* with reference counting, making a copy is not needed. */
    increase_refcount(s,free);
    *t=s;
    decrease_refcount(oldt);
}

static int arrayvalue(Uchar *s, int pos)
{
  if (pos<0 || !s)
    return 0;
  else {
#ifdef BOUND_CHECKING
    int i=Ustrlen(s);
    if (pos>i)
      return 0;
    else
#endif
      return s[pos];
  }
}

static void printstring(Uchar *s)
{
    printf("%s", UstrtoLocale(s));
}

static Uchar *stringadd(Uchar *s1, Uchar *s2)
{
    int j=Ustrlen(s1);
    int i=j+Ustrlen(s2)+1;
    Uchar *t;
    t=malloc(i*sizeof(Uchar));
    Ustrcpy(t,s1);
    Ustrcpy(t+j, s2);
    increase_refcount(t,free);
    return t;
}

static Uchar *stringminus(Uchar *s1, Uchar *s2)
{
    int i=Ustrlen(s1);
    int j=Ustrlen(s2);
    Uchar *t;
    if (i<j) { return s1; }
    if (Ustrcmp(s1+i-j, s2)) { return s1; }
    t=malloc((i-j+1)*sizeof(Uchar));
    Ustrncpy(t, s1, i-j);
    t[i-j]='\0';
    increase_refcount(t,free);
    return t;
}

static int stringequal(Uchar *s1, Uchar *s2)
{
    return !Ustrcmp(s1, s2);
}

static int stringnotequal(Uchar *s1, Uchar *s2)
{
    return Ustrcmp(s1,s2);
}

static int stringgreater(Uchar *s1, Uchar *s2)
{
    return Ustrcmp(s1,s2)>0;
}
static int stringless(Uchar *s1, Uchar *s2)
{
    return Ustrcmp(s1,s2)<0;
}
static int stringgreaterequal(Uchar *s1, Uchar *s2)
{
    return Ustrcmp(s1,s2)>=0;
}
static int  stringlessequal(Uchar *s1, Uchar *s2)
{
    return Ustrcmp(s1,s2)<=0;
}

static int call_strstrstrref(int (*fcal)(), void **args)
{
    return (*fcal)(*((Uchar**)args[0]),*((Uchar**)args[1]),*((Uchar***)args[2]));
}

static int call_strstrresstr(Uchar* (*fcal)(), void **args)
{
    *(*((Uchar***)args[2])) = (*fcal)(*((Uchar**)args[0]),*((Uchar**)args[1]));
}

static int call_strstrref(int (*fcal)(), void **args)
{
    return (*fcal)(*((Uchar**)args[0]),*((Uchar***)args[1]));
}

static int call_str(int (*fcal)(), void **args)
{
    return (*fcal)(*((Uchar**)args[0]));
}

static int call_strstrintref(int (*fcal)(), void **args)
{
    return (*fcal)(*((Uchar**)args[0]),*((Uchar**)args[1]),*((int**)args[2]));
}

static int call_strstrresint(int (*fcal)(), void **args)
{
    *(*((int**)args[2])) = (*fcal)(*((Uchar**)args[0]),*((Uchar**)args[1]));
    return 0;
}

static int call_strintresint(int (*fcal)(), void **args)
{
    *(*((int**)args[2])) = (*fcal)(*((Uchar**)args[0]),*((int*)args[1]));
    return 0;
}

static Type tlist[6][4] = {
   {StringType, StringType, ToRefType(StringType), 0},
   {StringType,	ToRefType(StringType), 0},
   {StringType, StringType, ToRefType(IntType),0 },
   {StringType, StringType, 0},
   {StringType, 0},
   {StringType, IntType,0},
};

static void construct_string(void **object)
{
   *object=0;
}

static void destruct_string(void *object)
{
   decrease_refcount(object);
}

static void copy_string(void **object, void *orig)
{
  increase_refcount(orig,free);
  *object=orig;
}


int init_library(void)
{
    Prototype *pt;
    FuncRef fr;
    Type t;

    t=define_type("String", sizeof(Uchar*), construct_string,
		  destruct_string, copy_string);

    define_program_variable(StringType, "TestVar", &testvar);
    pt=define_prototype(tlist[4], 1, 0, call_str);
    define_function("PrintString", "", pt, (Function) printstring);
    pt=define_prototype(tlist[1], 2, 0, call_strstrref);
    fr = define_function("AssignString", "", pt, (Function) assign);
    define_operator(OPASSIGN, fr);
    pt=define_prototype(tlist[5], 2, IntType, call_strintresint);
    fr = define_function("StringElem", "", pt, (Function) arrayvalue);
    define_operator(OPARRAY, fr);
    pt=define_prototype(tlist[0], 2, StringType, call_strstrresstr);
    fr = define_function("StringAdd", "", pt, (Function) stringadd);
    define_operator(OPADD, fr);
    fr = define_function("StringMin", "", pt, (Function) stringminus);
    define_operator(OPSUB, fr);
    pt=define_prototype(tlist[2], 2, IntType, call_strstrresint);
    fr = define_function("StringEqual", "", pt, (Function) stringequal);
    define_operator(OPEQUAL, fr);
    fr = define_function("StringNotEqual", "", pt, (Function) stringnotequal);
    define_operator(OPNOTEQUAL, fr);
    fr = define_function("StringGreater", "", pt, (Function) stringgreater);
    define_operator(OPGREATER, fr);
    fr = define_function("StringLess", "", pt, (Function) stringless);
    define_operator(OPLESS, fr);
    fr = define_function("StringLessEqual", "", pt, (Function) stringlessequal);
    define_operator(OPLESSEQUAL, fr);
    fr = define_function("StringGreaterEqual", "", pt, (Function) stringgreaterequal);
    define_operator(OPGREATEREQUAL, fr);
    return 1;
}
