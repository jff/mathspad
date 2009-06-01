#ifndef MP_FUNCS_H
#define MP_FUNCS_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "memman.h"

#ifndef NULL
#define NULL 0
#endif

extern int user_id, group_id;
extern Bool failure;

extern int mystrtoi(char *s, char **endp, int base);
extern char *begins_with(char *search, char *string);
extern Char *strip_name(Char *name);
extern void concat_in(Char *dest, Char *s1, Char *s2);
extern Char *concat(Char *s1, Char *s2);
extern FILE *open_dirfile(Char *dir, Char *filename,char *mode);
extern Bool is_directory(Char *dir);
extern int  read_dir_contents(Char *dir, Bool only_files,
                              Bool (*mask_check)(Char*,Char*),
			      Char* mask,
                              Char ***dirs, int *nrdirs,
                              Char ***files, int *nrfiles);
extern Char *standard_dir(Char *dirname);
extern Char *search_through_dirs(Char **dirs, int nr,
				 Char *filename);
extern void remove_file(Char *name);
extern void get_currentwd(void);

extern int skip_fontpart(FILE *f);
#endif
