#ifndef MP_CHECKBOX_H
#define MP_CHECKBOX_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/


#define CHECKBOXSIZE 9


extern void  checkbox_init(void);
extern void  checkbox_set(void *data, Bool on);
extern Bool  checkbox_value(void *data);
extern void  checkbox_connect(void *box, void *other_box);
extern void *checkbox_make(Window parent, int xpos, int ypos, int val);

#endif
