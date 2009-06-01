#ifndef MP_SYSTEM_H
#define MP_SYSTEM_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
** file  system.h
** doel  aantal constanten en variabelen definieren zodat het programma
**       voldoende informatie heeft en defaults (latex, directories)
*/

#define MAXWINDOWS      200
#define MAXFONTS         16
#define TEXTFONT          0
#define BUTTONFONT      254
#define BORDERWIDTH       0
#define INTERSPACE        2
#define SCROLLBARSIZE    12
#define MAX_PRECEDENCE   21
#define MAX_GETSTRING    31
#define CURSORWIDTH       6
#define CURSORHEIGHT      6

#define EDITFONT 0
#define NOTATIONFONT 1
#define SYMBOLFONT 2
#define POPUPFONT 3
#define NR_SIZE 4

/*
** mogelijk ook define's voor constanten die layout van buttons en
** scrollbars bepalen
*/

extern char **arguments;
extern int  number_of_arguments;
extern Char *progname,
            *homedir,
            *userdir,
            *fontdir,
            *latexdir,
            *notationdir,
            *fontfile[NR_SIZE],
	    *keypath,
            *program_dir,
            *program_latexdir,
            *program_notationdir,
            *program_fontfilename,
	    *program_keypath;
extern Char **help_dirs;
extern int nr_help_dirs;

extern Char *latex_line_unit,
            *latex_space_unit,
            *latex_tab_unit;

extern int latex_side,
           line_space,
           screen_tab,
           screen_space,
           notation_precedence,
	   textdots,
	   autowordwrap,
	   singlemode,
	   autodouble,
	   saveonexit,
           wait_time;

extern int symbol_is_open,
           notation_is_open,
           edit_is_open,
           buffer_is_open,
           notadef_is_open,
           default_is_open,
           remark_is_open,
	   find_is_open,
           menu_is_open,
           symbol_iconized,
           notation_iconized,
           edit_iconized,
           buffer_iconized,
           notadef_iconized,
           default_iconized,
           remark_iconized,
	   find_iconized,
           menu_iconized;

#define  nrio   !remark_is_open

#define  can_open_symbol   (nrio)
#define  can_open_notation (nrio)
#define  can_open_buffer   (!buffer_is_open && nrio)
#define  can_open_edit     (nrio)
#define  can_open_notadef  (notation_is_open && !notadef_is_open && nrio)
#define  can_open_default  (!default_is_open && nrio)
#define  can_open_remark   (nrio)
#define  can_open_find     (!find_is_open && nrio)

#define  can_close_symbol    (symbol_is_open && nrio)
#define  can_close_notation  (notation_is_open && nrio)
#define  can_close_buffer    (buffer_is_open && nrio)
#define  can_close_edit      (nrio)
#define  can_close_notadef   (notadef_is_open && nrio)
#define  can_close_default   (default_is_open && nrio)
#define  can_close_remark    (remark_is_open)
#define  can_close_find      (find_is_open && nrio)


extern void system_init(int argc, char *argv[]);
#endif
