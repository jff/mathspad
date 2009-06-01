#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>

#ifdef HAVE_POW_IN_LIBMP

struct mint {
  int len;
  short *val;
};
typedef struct mint MINT;

extern void pow(MINT*, MINT*,MINT*,MINT*);
extern double floor(double);

/* return address of correct power function */

void *getpowfunc(void)
{
  void *libhandle;
  void *powf=0;
  MINT *a, *b, *c, *d;

  /* Call pow(a,b,c,d) with correct values of a, b, c and d.
  ** If d does not change, the correct version of pow is called.
  ** If d does change, use dlopen to open libm.so, use dlsym to
  ** get the correct pow() function.
  */
  a = malloc(sizeof(MINT));
  b = malloc(sizeof(MINT));
  c = malloc(sizeof(MINT));
  d = malloc(sizeof(MINT));
  a->val=malloc(sizeof(short));
  b->val=malloc(sizeof(short));
  c->val=malloc(sizeof(short));
  d->val=0;
  a->len=b->len=c->len=1;
  d->len=0;
  a->val[0]=2;
  b->val[0]=8;
  c->val[0]=17;
  (void) floor(1.5);
  pow(a,b,c,d);
  if (d->len || d->val) {
    /* wrong version of pow */
    fprintf(stderr, "Fixing pow() function\n");
    libhandle = dlopen("libm.so", RTLD_LAZY);
    if (libhandle) {
      powf = dlsym(libhandle, "pow");
      dlclose(libhandle);
    }
  } else {
    powf = pow;
  }
  free(a->val); free(b->val); free(c->val); if (d->val) free(d->val);
  free(a); free(b); free(c); free(d);
  return powf;
}

#endif
