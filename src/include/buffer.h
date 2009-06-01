#ifndef MP_BUFFER_H
#define MP_BUFFER_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  File : buffer.h
**  Datum: 11-4-92
*/

extern void buffer_init(void);
extern void buffer_open(void);
extern void buffer_close(void);
extern void buffer_set_number_of_lines(void *data, int numlin);
#endif
