#ifndef MP_EDIT_H
#define MP_EDIT_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  File : edit.h
**  Datum: 11-4-92
**  Doel : Het open, sluiten en werken met een editwindow mogelijk maken.
*/

extern void edit_init(void);
extern void edit_open(void);
extern void edit_close(void *data);
extern Bool edit_saved(void *data);
extern void edit_bad_end(void *data);
extern void open_program(Char *prog, Char *title, int (*func)(unsigned char*,unsigned int*));
extern void open_helpfile(void *data, int nr);
extern void open_temporary_file(char *buffername, char *filename, int disp, int linenum);
extern void edit_set_number_of_lines(void *data, int numlin);
extern void edit_string_to_proces(Char *format, Char *shellname);
extern void edit_signal_to_proces(int signal, Char *shellname);

extern void open_message_window(Char *messagetitle);
extern void string_to_window(Char *messagegroup, Char *mess);
extern void structure_to_window(Char *messagegroup);
#endif
