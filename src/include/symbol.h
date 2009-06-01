#ifndef MP_SYMBOL_H
#define MP_SYMBOL_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/* File : symbol.h
 */

extern void symbol_init(void);
extern void symbol_open(void);
extern Char symbol_last(void);
extern void symbol_new_page(Char *str, int columns, int rows);
extern void symbol_range_page(Char *str, int start, int end);
extern void symbol_selected_symbol(Char *str);
extern void symbol_empty_page(Char *str, int length);
extern void symbol_goto_page(Char *str);
extern void symbol_select_up(int num);
extern void symbol_select_down(int num);
extern void symbol_select_left(int num);
extern void symbol_select_right(int num);
#endif
