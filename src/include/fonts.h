#ifndef MP_FONTS_H
#define MP_FONTS_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/

#include "unifont.h"

extern int fontattributes[NR_SIZE];

extern Char *generalinfofile;
extern int      missing_font(FILE *f);
extern Bool     new_fontfile(int fontgroup, Char *newname);

extern void push_attributes(int attributes);
extern void pop_attributes(void);

#define push_fontgroup(A)  push_attributes(A)
#define pop_fontgroup()    pop_attributes()
#define font_ID(A) ((A)->font)

extern short    font_width(void);
extern int      font_ascent(void);
extern int      font_height(void);
extern int      font_descent(void);

extern int char_width(Char c);
extern int char_ascent(Char c);
extern int char_descent(Char c);
extern int char_left(Char c);
extern int char_right(Char c);

extern int      string_width(Char *txt, int len);

#endif
