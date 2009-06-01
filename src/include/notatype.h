#ifndef MP_NOTATYPE_H
#define MP_NOTATYPE_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/*
**  File: notatype.h
*/

#if !defined int_contains
#include "flexarray.h"
#endif

#define EDIT_SIZE  1000
#define SCREENFORMAT 0
#define LATEXFORMAT 1
#define NAMEFORMAT 2
#define MAXFORMAT  3

extern Char *kind_description(int kind);

#define MAX_KIND   6

typedef
struct {  unsigned long vnr;
	  int ivnr;
	  int lines;
	  int latexmode;
	  int nr_plhd;
	  int *phindex;
	  Char *format[MAXFORMAT];
	  int max[MAXFORMAT];
	  /* more formats are possible */
       } VERSION;

typedef
struct  { Opkind kind;
	  int space;
	  int prec, versions;
	  int locks,fillocks;
	  unsigned long nnr;
	  int innr;
	  Char *name;
	  VERSION *vers;
	  Char *helpfilename;
        } NOTATION;

typedef NOTATION *NOTATIONLIST;

typedef
struct  { Char *name;
	  Char *dirname;
	  int nr_windows;
	  int samename;
	  int nrnt;
          FlexArray nkind[MAX_KIND];
	  int saved, autosaved;
        } NOTATIONFILE;

extern int edit_fnr;
extern int use_file_nr;

extern void set_change_function( void (*cfunc)(void));
extern void changed_notation(void);
extern void notatype_init(void);
extern unsigned long new_number(void);

extern void     add_version(VERSION **list, int place, int *max);
extern void     remove_version(VERSION **list, int place, int *max);
extern VERSION *maximize_version(VERSION *list, int max);
extern void     destroy_version(VERSION *list, int max);
extern VERSION *minimize_version(VERSION *list, int max);
extern Bool     make_size_version(VERSION *vers, int fmnr, int newmax);

extern int       save_stencil(Index innr);
extern int       load_stencil(int nnr);
extern void      cleanup_stencilstack(void);
extern void      remove_double_file(int sfnr);
extern void      remove_multiple_files(int sfnr);
extern Bool      remove_double_template(int innr);
extern void      remove_double(void);
extern int       match_format_or_make(Char *str, int len, int kind,
				      int prec, int spac);
extern NOTATION *which_notation(int innr);
extern VERSION  *which_version(int innr);
extern Index     which_version_nr(int innr);
extern int       position(VERSION *vers, Char placeh);
extern int       nnr_vnr2innr(int nnr, int vnr);
extern void      lock_stencil(Index innr);
extern void      unlock_stencil(Index innr);

extern NOTATION *get_notation_kind(int fnr, int kind, int nr);
extern int       get_notation_nr(int fnr, int kind, int nr);
extern int       add_notation(int oldnr, NOTATION *list);
extern void      remove_notation(int fnr, int notanr);
extern int       notation_with_name(Char *name);
extern int       notation_with_number(unsigned long number);
extern Bool      move_nota_left(int fnr, int kind, int anr);
extern Bool      move_nota_right(int fnr, int kind, int anr);
extern int       nr_visible(int fnr);

extern int  new_notation_window(void);
extern int  clear_notation_window(int fnr);
extern int  load_notation_window(int fnr, Char *filename);
extern void save_notation_window(int fnr, Char *filename);
extern void free_notation_window(int fnr);
extern void auto_save_window(int fnr, int dump);
extern int  notation_not_saved(int fnr);
extern void saved_notation_file(int fnr);
extern Bool last_window(int fnr);
extern void rename_notation_window(int fnr, Char *name);

extern int  get_notation_number(int fnr);
extern int  get_next_filename(int fnr, Char **c, Bool *opened);
extern Char *get_notation_filename(int fnr);
extern Char *get_notation_dirname(int fnr);
extern Bool load_notation_filenames(FILE *f);
extern void view_notation_filenames(FILE *f);
extern Char *make_info(int nnr);
extern void add_file_ref(int fnr);
extern void clear_file_ref(void);

#define stencil_prec(A)           (which_notation(A)->prec)
#define stencil_kind(A)           (which_notation(A)->kind)
#define stencil_space(A)          (which_notation(A)->space)
#define stencil_lines(A)          (which_version(A)->lines)
#define stencil_latexmode(A)          (which_version(A)->latexmode)
#define stencil_screen(A)         (which_version(A)->format[SCREENFORMAT])
#define stencil_size(A)           (which_version(A)->max[SCREENFORMAT])
#define stencil_latex(A)          (which_version(A)->format[LATEXFORMAT])
#define stencil_latex_size(A)     (which_version(A)->max[LATEXFORMAT])
#define stencil_position(A,B)     (position(which_version(A), B))
#define stencil_position_first(A) (which_version(A)->nr_plhd?\
				   which_version(A)->phindex[0]>>16:0)
#define stencil_position_last(A)  (which_version(A)->nr_plhd ? \
				   which_version(A)->phindex[\
					(which_version(A)->nr_plhd)-1]>>16 : 0)

extern int stencil_position_right(Index innr, Char phnr);
extern int stencil_position_left(Index innr, Char phnr);
#endif
