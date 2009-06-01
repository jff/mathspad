#ifndef MP_NOTATION_H
#define MP_NOTATION_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**   File : notation.h
**   Datum: 14-5-92
*/

extern void  notation_init(void);
extern void  notation_open(void);
extern void  notation_move_begin(void);
extern void  notation_move_end(void);
extern void  notation_move_left(void);
extern void  notation_move_right(void);
extern int   notation_last(void);
extern int   notation_version(void);
extern void  notation_confirm_backup(int i);
extern Bool  make_notation_popup(int nnr, int vnr, void (*func)(void*,int),
				 Char *title, Bool motion);
#endif
