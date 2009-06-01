#ifndef MP_SCROLLBAR_H
#define MP_SCROLLBAR_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
 *  File  : scrollbar.h
 *  Datum : 29-3-92
 *  Doel  : Het maken, tekenen en gebruiken van scrollbars
 */

#define HORIZONTAL 0
#define VERTICAL   1
#define HORIZONTAL_SHORT 2
#define VERTICAL_SHORT 3


extern void  scrollbar_init(void);
extern void *scrollbar_make(int kind, Window parent, int x_offset,
                            int y_offset, unsigned int size, int linesize,
			    void (*func)(void *,int), void *data);
extern void  scrollbar_move(void *data, int xpos, int ypos);
extern void  scrollbar_resize(void *data, unsigned int newsize);
extern void  scrollbar_set(void *data, int f_line, int total_lines);
extern int   scrollbar_line(void *data, int delta);
extern void  scrollbar_linesize(void *data, int newsize);

#endif
