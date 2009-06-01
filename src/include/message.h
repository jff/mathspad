#ifndef MP_MESSAGE_H
#define MP_MESSAGE_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**   File: message.h
*/

#define MP_MESSAGE     0
#define MP_ERROR       1
#define MP_CLICKREMARK 2
#define MP_KEYREMARK   3
#define MP_MESSAGECURS 4
#define MP_EXIT        1000

extern void message(int lvl, Char *text);
extern void message2(int lvl, Char *text1, Char *text2);
#endif
