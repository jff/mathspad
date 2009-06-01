#ifndef MP_LATEXOUT_H
#define MP_LATEXOUT_H

#include <stdlib.h>
#include <stdio.h>

#include "mathpad.h"
#include "system.h"

typedef enum { ExprOpen, ExprClose, SOpOpen, SOpClose, LOpOpen, LOpClose,
               SIdOpen, SIdClose, LIdOpen, LIdClose, VarOpen, VarClose,
               TextOpen, TextClose, DispOpen, DispClose
	   } TexCode;


extern void push_math_pref(Bool premath);
extern void pop_math_pref(void);
extern void tex_set_file(FILE *f);
extern void tex_open_proof(void);
extern void tex_close_proof(void);
extern void tex_unset(void);
extern int  tex_current_pos(void);
extern void tex_placeholders(Bool texthem);
extern void out_latex_char(Char c);
extern void set_display_delta(int d);
extern void tex_code(TexCode c);
extern void tex_to_mode(int tmode);
extern void tex_set_string(char **string);
extern void tex_mode(int mode);

extern char*    font_opentexttex(int attribute);
extern char*    font_openmathtex(int attribute);
extern char*    font_closetex(int attribute);

extern char*    char_latex(Char combicode, Bool math);
extern char*    char_latex_next(Char *data, int *math);
#endif
