#ifndef MP_DEFAULT_H
#define MP_DEFAULT_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**   File : default.h
**   Datum: 11-4-92
**   Doel : Instellingen van het programma veranderen.
**          Vastleggen van de layout.
**          Later mogelijk het verwerken van command-line.
*/

extern void default_init(void);
extern void default_open(void);
extern void default_close(void);
extern void default_update(void);
extern void load_keypath(void);
#endif
