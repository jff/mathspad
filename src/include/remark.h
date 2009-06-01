#ifndef MP_REMARK_H
#define MP_REMARK_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
 /*
 **  File : remark.h
 **  Datum: 21-6-92
 **  Doel : Zorg dat de opmerking op het scherm verschijnt.
 */

#define LONG_REMARK  -1
#define SHORT_REMARK -2
#define NO_REMARK    -3
#define REMARK_CENTRE  1
#define REMARK_POINTER 2
#define REMARK_LASTPOS 4
#define REMARK_VCENTRE 8
#define REMARK_BUTTON  16
#define REMARK_POSITION 31
#define REMARK_MOTION  32

extern int kind_of_remark;
extern Bool select_line;
extern int selected_line;
extern Bool remark_no_button;

extern void  remark_init(void);
extern Bool  remark_make(Window mwin, void *info, void (*func)(void*,int),
			 int where, Char *remarktext, Char **buttontext,
			 Char **stringtext, int maxlength,
			 Char *helpfile);
extern Bool  remark_removable(void);
extern void  remark_raise(void);
extern void  remark_unmap(void);

#endif
