#ifndef MP_FILEREAD_H
#define MP_FILEREAD_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
/* fileread.h */

#define FAILURE      -1
#define SUCCESS       0
#define INSERT_LENGTH 1
#define INSERT_BUFFER 2
#define FREE_BUFFER   4

#define DETECTFILE    1
#define BINARYFILE    2
#define DOCUMENTFILE  3
#define STENCILFILE   4
#define KEYBOARDFILE  5
#define MACROFILE     6
#define SYMBOLFILE    7
#define FONTFILE      8
#define PROJECTFILE   9
#define FINDREPFILE  10
#define STATEFILE    11
#define MAXFILE      12
/* more to come. */

#define CHAR2TYPE        'C'
#define DEFAULTSTYPE     'D'
#define FONTSTYPE        'F'
#define GENERALFILETYPE  'G'
#define KEYBOARDTYPE     'K'
#define DETECTTYPE       'M'
#define NODETYPE         'N'
#define STENCILTYPE      'S'
#define STENCILFILETYPE  'T'
#define VERSIONTYPE      'V'
#define CHARTYPE         'c'
#define FINDREPTYPE      'f'
#define MACROTYPE        'm'
#define SYMBOLTYPE       's'
/* more to come. */

typedef int (*GETFUNC)(Char*,int*,int);


extern int get_stencil(Char*,int*,int),
           get_node(Char*,int*,int),
	   get_ascii_node(Char*,int*,int),
	   get_version(Char*,int*,int),
	   get_stencilfile(Char*,int*,int),
	   get_document(Char*,int*,int),
	   get_defaults(Char*,int*,int),
	   get_fonts(Char*,int*,int),
	   get_keyboard(Char*,int*,int),
	   get_findrep(Char*,int*,int),
	   get_smacro(Char*,int*,int),
	   get_symbol(Char*,int*,int);

/* more to come. */

extern int read_file(FILE *f, int type);
extern void cleanup_filestack(void);
extern void set_file(FILE *f);
extern void unset_file(void);
extern void put_char(Char c);
extern void put_Char(Char c);
extern void put_integer(unsigned long l);
extern void put_struct(char type, int size);
extern void put_end_struct(void);
extern void push_hidden(void);
extern void pop_hidden(void);
extern void put_filecode(int type);
     /* <27>[1mM<27>^<16><type><27>\athpad <desc(type)><13><27>[0m  */

extern void put_String(Char *s, int len);
extern void put_string(char *s, int len);
extern int  get_String(Char **s, int *len, int *max);
extern int  get_string(Char **s, int *len, int *max);

extern Char oldtonew(Char c);

#endif
