#ifndef MP_FILESELC_H
#define MP_FILESELC_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
extern void fileselc_init(void);
extern void fileselc_open(void (*func)(void*,Char*), void *arg,
			  Char *descript,
			  Char *dir, Char *mask,
			  Char *deffile, Window checkw);
extern void dirselc_open(void (*func)(void*,Char*), void *arg,
			  Char *descript,
			  Char *dir, Char *mask,
			  Char *deffile, Window checkw);
#endif
