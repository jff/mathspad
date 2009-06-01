#ifndef MP_FSTATE_H
#define MP_FSTATE_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
#include "unicode.h"

typedef Uchar FSTATE;

extern FSTATE *make_fstate(Uchar *mask, int minimize);
extern void free_fstate(FSTATE *fs);
extern int fstate_check(FSTATE *fs, Uchar *s);
#endif
