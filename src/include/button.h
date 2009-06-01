#ifndef MP_BUTTON_H
#define MP_BUTTON_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  File : button.h
**  Datum: 29-3-92
**  Doel : Zorg dat de buttons goed op het scherm verschijnen.
*/

#define BINTERSPACE  2*INTERSPACE
extern unsigned int button_height;

typedef void (*BTFUNC)(void*,int);

extern void         button_init(void);
extern unsigned int button_width(Char *txt);
extern void         button_move(void *data, int x, int y);
extern void         button_stick(int gravity);
extern void        *button_make(int bnr, Window parent, Char *txt,
				int *x, int y, int border, void *data,
				char *helpfile,
				BTFUNC func1p, BTFUNC func2p, BTFUNC func3p,
				BTFUNC func1r, BTFUNC func2r, BTFUNC func3r);
#endif
