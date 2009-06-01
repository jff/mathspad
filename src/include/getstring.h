#ifndef MP_GETSTRING_H
#define MP_GETSTRING_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  File  : getstring.h
**  Datum : 26-4-92
**  Doel  : De invoer van strings mogelijk maken
*/

extern unsigned int string_height(void);
extern unsigned int string_window_width(unsigned int nchars);
extern void  string_init(void);
extern void *string_make(Window parent, Char *txt, unsigned int maxlen,
                         unsigned int width, char *helpfile,
			 int x_offset, int y_offset, Bool is_integer);
extern void  string_destroy(void *data);
extern void  string_relation(void *data, void *prev, void *next);
extern void  string_refresh(void *data, Char *txt);
extern Char *string_text(void *data);
extern void  string_keyboard(void);
extern void  string_resize(void *data, unsigned int new_width);
extern void  string_move(void *data, int newx, int newy);
extern void  string_get_input(void *data);
extern void  string_unmap(void *data);
extern void  string_map(void *data);
extern void  string_change_return(int funcnr);
extern void  string_reset_return(void);
#endif
