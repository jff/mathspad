#ifndef MP_POWFIX_H
#define MP_POWFIX_H
#include <config.h>
#ifdef HAVE_POW_IN_LIBMP

extern void *getpowfunc(void);
static double (*powfunc)(double,double)=0;

#define pow(X,Y) (!powfunc?((powfunc=getpowfunc())? (*powfunc)(X,Y): 1.0):(*powfunc)(X,Y))

#endif

#endif
