#ifndef MP_MENU_H
#define MP_MENU_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
 *  File  : menu.h
 *  Datum : 9-4-92
 *  Doel  : plaatsen van group-window en buttons maken voor de verschillende
 *          functie en windows
 */

extern int quit_sequence;
extern void menu_init(void);
extern void menu_open(int x, int y, int w, int h, int icon, int s, Char *str);
extern void menu_close(void);
extern void menu_bad_end(void *data);
extern void menu_set_command(void);
extern void menu_keyboard(void);
#endif
