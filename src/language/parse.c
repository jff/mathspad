
/*  A Bison parser, made from parse.y
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	LOGICOR	257
#define	LOGICXOR	258
#define	LOGICAND	259
#define	BITOR	260
#define	BITXOR	261
#define	BITAND	262
#define	EQUAL	263
#define	LESS	264
#define	GREATER	265
#define	LESSEQUAL	266
#define	GREATEREQUAL	267
#define	NOTEQUAL	268
#define	SHIFTLEFT	269
#define	SHIFTRIGHT	270
#define	ADD	271
#define	MINUS	272
#define	MULTIPLY	273
#define	DIVIDE	274
#define	REMAINDER	275
#define	LOGICNOT	276
#define	BITNOT	277
#define	LAZYREF	278
#define	STRING	279
#define	IDENTIFIER	280
#define	INTEGER	281
#define	TYPEVAL	282
#define	REAL	283
#define	KEY	284
#define	INPUT	285
#define	LAYOUT	286
#define	MENU	287
#define	KEYBOARD	288
#define	FUNCTION	289
#define	VARIABLE	290
#define	IF	291
#define	ELSEIF	292
#define	ELSE	293
#define	FI	294
#define	DO	295
#define	ELSEDO	296
#define	OD	297
#define	OPTIONS	298
#define	PIN	299
#define	LEFTRIGHT	300
#define	RIGHTLEFT	301
#define	SEPARATOR	302
#define	TITLE	303
#define	DEFAULT	304
#define	BUTTON	305
#define	IMAGE	306
#define	SCROLLBAR	307
#define	LEFT	308
#define	RIGHT	309
#define	BOTTOM	310
#define	TOP	311
#define	GEOMETRY	312
#define	TYPE	313
#define	EDIT	314
#define	COMMENT	315
#define	PROGRAM	316
#define	CONSOLE	317
#define	BUFFER	318
#define	SYMBOL	319
#define	STENCIL	320
#define	DEFINE	321
#define	FINDREPLACE	322
#define	ALL	323
#define	SHELL	324
#define	FILESELECT	325
#define	REMARK	326
#define	CLEAR	327
#define	INCLUDE	328
#define	ASSIGN	329
#define	RANGE	330
#define	TRANSLATION	331

#line 1 "parse.y"

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "language.h"
#include "loadlib.h"
#include "parse.h"
#include "unicode.h"
#include "unistring.h"
#include "unitype.h"
#include "translate.h"

typedef struct {
    Sequence *first;
    Sequence *last;
} SeqPair;

typedef struct {
    int nr;
    Argument *list;
} ArgList;

typedef struct {
    Expression *first;
    Expression *last;
} ExprPair;

typedef struct {
    Expression *first;
    Expression *last;
    int nr;
} ExprList;

typedef struct {
    FuncRef func;
    int is_userfunc;
} FuncDesc;

typedef struct {
  KeyNum key;
  KeyMode mode;
} KeyDesc;

typedef struct {
  KeyNum *key;
  KeyMode *mode;
  short len;
} KeyList;

/* the parser only uses one KeyList at a time. So, static arrays
** are used to store the keys and modes
*/

static KeyNum keys[500];
static KeyMode mode[500];
static KeyMap *current_keymap=0;

static int current_type=0;

static PopupMenu *current_menu =0;

int yyerror(char *message);
int yylex(void);


#line 68 "parse.y"
typedef union {
	int ival;
	double rval;
        Uchar *utval;
	char *tval;
	FuncRef *uval;
	Expression *eval;
	ExprPair epval;
	ExprList elval;
	Argument aval;
	ArgList alval;
	SeqPair spval;
	FuncDesc fref;
	Sequence *seqval;
        Prototype *protval;
        KeyDesc keyval;
        KeyList klistval;
	} YYSTYPE;
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		273
#define	YYFLAG		-32768
#define	YYNTBASE	87

#define YYTRANSLATE(x) ((unsigned)(x) <= 331 ? yytranslate[x] : 136)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,    79,
    80,     2,     2,    83,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    86,    78,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
    84,     2,    85,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    81,     2,    82,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
    37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
    47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
    57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
    67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
    77
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     4,     7,    10,    13,    16,    19,    22,    25,
    28,    32,    33,    44,    45,    47,    51,    54,    55,    58,
    60,    64,    65,    70,    72,    76,    78,    80,    84,    85,
    91,    93,    95,    96,    98,   106,   116,   124,   125,   132,
   133,   140,   144,   146,   150,   152,   156,   157,   159,   163,
   164,   170,   172,   174,   176,   178,   180,   181,   187,   190,
   194,   198,   202,   206,   210,   214,   218,   222,   226,   229,
   233,   237,   241,   244,   248,   252,   256,   260,   264,   268,
   272,   276,   277,   284,   286,   290,   291,   293,   297,   303,
   305,   308,   313,   314,   317,   320,   321,   329,   330,   334,
   336,   339,   341,   343,   345,   347,   351,   352,   354,   357,
   361,   365,   368,   374,   376,   380,   381,   385,   391,   393,
   397,   400,   403,   407,   413,   416,   421,   424,   427,   429,
   433,   439,   442,   445,   447,   449,   451,   453,   455,   457,
   459,   461,   463,   465,   467,   469,   471,   473,   475,   477,
   479,   483,   485
};

static const short yyrhs[] = {    -1,
    87,    88,     0,    87,   119,     0,    87,   112,     0,    87,
   129,     0,    87,   126,     0,    87,    78,     0,    87,   100,
     0,    87,   117,     0,    87,    94,     0,    87,    74,    25,
     0,     0,    35,    26,    79,    89,    90,    80,    81,    92,
    97,    82,     0,     0,    91,     0,    90,    83,    91,     0,
    28,    26,     0,     0,    93,    78,     0,    94,     0,    93,
    78,    94,     0,     0,    36,    28,    95,    96,     0,    26,
     0,    96,    83,    26,     0,   101,     0,    98,     0,    97,
    78,   101,     0,     0,    81,    99,    92,    97,    82,     0,
   104,     0,   108,     0,     0,   100,     0,    37,    79,   110,
    80,    97,   102,    40,     0,    37,    79,   110,    80,    97,
   102,    39,    97,    40,     0,    41,    79,   110,    80,    97,
   103,    43,     0,     0,   102,    38,    79,   110,    80,    97,
     0,     0,   103,    42,    79,   110,    80,    97,     0,   105,
    75,   106,     0,    26,     0,   105,    83,    26,     0,   110,
     0,   106,    83,   110,     0,     0,   110,     0,   107,    83,
   110,     0,     0,    26,    79,   109,   107,    80,     0,    27,
     0,    25,     0,    29,     0,   108,     0,    26,     0,     0,
    26,    84,   111,   110,    85,     0,    24,   110,     0,    79,
   110,    80,     0,   110,    17,   110,     0,   110,    18,   110,
     0,   110,    19,   110,     0,   110,    20,   110,     0,   110,
    21,   110,     0,   110,     5,   110,     0,   110,     3,   110,
     0,   110,     4,   110,     0,    22,   110,     0,   110,     8,
   110,     0,   110,     6,   110,     0,   110,     7,   110,     0,
    23,   110,     0,   110,     9,   110,     0,   110,    14,   110,
     0,   110,    11,   110,     0,   110,    10,   110,     0,   110,
    12,   110,     0,   110,    13,   110,     0,   110,    15,   110,
     0,   110,    16,   110,     0,     0,    34,    26,    81,   113,
   114,    82,     0,   115,     0,   114,    78,   115,     0,     0,
    73,     0,   116,    86,   100,     0,    30,    76,    30,    86,
   100,     0,    30,     0,   116,    30,     0,    31,    81,   118,
    82,     0,     0,   118,    25,     0,   118,    30,     0,     0,
    33,    26,    81,   120,   121,   124,    82,     0,     0,    44,
   122,    78,     0,   123,     0,   122,   123,     0,    45,     0,
    46,     0,    47,     0,   125,     0,   124,    78,   125,     0,
     0,    48,     0,    49,    25,     0,    25,    86,   100,     0,
    25,    86,    26,     0,    50,   125,     0,    77,    26,    81,
   127,    82,     0,   128,     0,   127,    78,   128,     0,     0,
    25,    86,    25,     0,    32,    26,    81,   130,    82,     0,
   131,     0,   130,    78,   131,     0,    49,    25,     0,    49,
    26,     0,    51,    25,   100,     0,    51,    25,    52,    25,
   100,     0,    53,   132,     0,    58,    27,    27,    27,     0,
    59,   133,     0,    34,   134,     0,    48,     0,    60,    25,
    26,     0,    60,    25,    26,    50,   135,     0,    61,    25,
     0,    62,    25,     0,    54,     0,    55,     0,    56,     0,
    57,     0,    63,     0,    60,     0,    64,     0,    65,     0,
    66,     0,    67,     0,    68,     0,    69,     0,    70,     0,
    50,     0,    71,     0,    72,     0,    26,     0,   134,    83,
    26,     0,    25,     0,   135,    83,    25,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
   120,   121,   122,   123,   124,   125,   126,   127,   131,   132,
   133,   148,   153,   163,   165,   169,   173,   178,   179,   181,
   182,   184,   187,   188,   190,   193,   195,   197,   207,   209,
   214,   216,   219,   221,   223,   227,   234,   239,   241,   246,
   248,   253,   264,   269,   276,   282,   289,   291,   293,   300,
   311,   317,   319,   321,   323,   325,   334,   342,   347,   349,
   351,   353,   355,   357,   359,   361,   363,   365,   367,   369,
   371,   373,   375,   377,   379,   381,   383,   385,   387,   389,
   391,   394,   396,   399,   400,   402,   403,   405,   409,   414,
   418,   425,   427,   428,   430,   436,   439,   442,   443,   445,
   446,   448,   450,   452,   455,   456,   458,   459,   461,   463,
   465,   467,   470,   472,   473,   475,   476,   479,   481,   482,
   484,   485,   486,   487,   488,   489,   490,   491,   492,   493,
   494,   495,   496,   498,   499,   500,   501,   503,   504,   505,
   506,   507,   508,   509,   510,   511,   512,   513,   514,   516,
   518,   520,   522
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","LOGICOR",
"LOGICXOR","LOGICAND","BITOR","BITXOR","BITAND","EQUAL","LESS","GREATER","LESSEQUAL",
"GREATEREQUAL","NOTEQUAL","SHIFTLEFT","SHIFTRIGHT","ADD","MINUS","MULTIPLY",
"DIVIDE","REMAINDER","LOGICNOT","BITNOT","LAZYREF","STRING","IDENTIFIER","INTEGER",
"TYPEVAL","REAL","KEY","INPUT","LAYOUT","MENU","KEYBOARD","FUNCTION","VARIABLE",
"IF","ELSEIF","ELSE","FI","DO","ELSEDO","OD","OPTIONS","PIN","LEFTRIGHT","RIGHTLEFT",
"SEPARATOR","TITLE","DEFAULT","BUTTON","IMAGE","SCROLLBAR","LEFT","RIGHT","BOTTOM",
"TOP","GEOMETRY","TYPE","EDIT","COMMENT","PROGRAM","CONSOLE","BUFFER","SYMBOL",
"STENCIL","DEFINE","FINDREPLACE","ALL","SHELL","FILESELECT","REMARK","CLEAR",
"INCLUDE","ASSIGN","RANGE","TRANSLATION","';'","'('","')'","'{'","'}'","','",
"'['","']'","':'","file","functiondef","@1","prototype","proto","vardefs","vardeflist",
"variabledef","@2","defidents","sequence","block","@3","funcseq","statement",
"elseiflist","elsedolist","assignment","identlist","assexprlist","exprlist",
"functioncall","@4","expression","@5","keyboarddef","@6","keydefs","keydef",
"keylist","inputdef","inputitems","menudef","@7","options","opitems","opitem",
"menulines","menuline","translation","translines","transline","layoutdef","layoutlines",
"layoutline","location","windowtype","keyidents","stringlist", NULL
};
#endif

static const short yyr1[] = {     0,
    87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
    87,    89,    88,    90,    90,    90,    91,    92,    92,    93,
    93,    95,    94,    96,    96,    97,    97,    97,    99,    98,
   100,   100,   101,   101,   101,   101,   101,   102,   102,   103,
   103,   104,   105,   105,   106,   106,   107,   107,   107,   109,
   108,   110,   110,   110,   110,   110,   111,   110,   110,   110,
   110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
   110,   110,   110,   110,   110,   110,   110,   110,   110,   110,
   110,   113,   112,   114,   114,   115,   115,   115,   115,   116,
   116,   117,   118,   118,   118,   120,   119,   121,   121,   122,
   122,   123,   123,   123,   124,   124,   125,   125,   125,   125,
   125,   125,   126,   127,   127,   128,   128,   129,   130,   130,
   131,   131,   131,   131,   131,   131,   131,   131,   131,   131,
   131,   131,   131,   132,   132,   132,   132,   133,   133,   133,
   133,   133,   133,   133,   133,   133,   133,   133,   133,   134,
   134,   135,   135
};

static const short yyr2[] = {     0,
     0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     3,     0,    10,     0,     1,     3,     2,     0,     2,     1,
     3,     0,     4,     1,     3,     1,     1,     3,     0,     5,
     1,     1,     0,     1,     7,     9,     7,     0,     6,     0,
     6,     3,     1,     3,     1,     3,     0,     1,     3,     0,
     5,     1,     1,     1,     1,     1,     0,     5,     2,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     2,     3,
     3,     3,     2,     3,     3,     3,     3,     3,     3,     3,
     3,     0,     6,     1,     3,     0,     1,     3,     5,     1,
     2,     4,     0,     2,     2,     0,     7,     0,     3,     1,
     2,     1,     1,     1,     1,     3,     0,     1,     2,     3,
     3,     2,     5,     1,     3,     0,     3,     5,     1,     3,
     2,     2,     3,     5,     2,     4,     2,     2,     1,     3,
     5,     2,     2,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     3,     1,     3
};

static const short yydefact[] = {     1,
     0,    43,     0,     0,     0,     0,     0,     0,     0,     0,
     7,     2,    10,     8,    31,     0,    32,     4,     9,     3,
     6,     5,    50,    93,     0,     0,     0,     0,    22,    11,
     0,     0,     0,    47,     0,     0,    96,    82,    12,     0,
   116,     0,     0,     0,    53,    56,    52,    54,     0,    42,
    55,    45,    44,     0,    48,    94,    95,    92,     0,   129,
     0,     0,     0,     0,     0,     0,     0,     0,     0,   119,
    98,    86,    14,    24,    23,     0,     0,   114,    69,    73,
    59,    57,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,    51,     0,   150,   128,   121,   122,     0,
   134,   135,   136,   137,   125,     0,   147,   139,   138,   140,
   141,   142,   143,   144,   145,   146,   148,   149,   127,     0,
   132,   133,     0,   118,     0,   107,    90,    87,     0,    84,
     0,     0,     0,    15,     0,     0,   116,   113,     0,    60,
    46,    67,    68,    66,    71,    72,    70,    74,    77,    76,
    78,    79,    75,    80,    81,    61,    62,    63,    64,    65,
    49,     0,     0,   123,     0,   130,   120,   102,   103,   104,
     0,   100,     0,   108,     0,   107,     0,   105,     0,    86,
    83,    91,     0,    17,     0,     0,    25,   117,   115,     0,
   151,     0,   126,     0,    99,   101,     0,   109,   112,   107,
    97,     0,    85,    88,    18,    16,    58,   124,   152,   131,
    43,   110,   106,     0,    33,     0,    20,     0,    89,     0,
     0,    29,     0,    27,    34,    26,    19,   153,     0,     0,
    18,    33,    13,    21,     0,     0,    33,    28,    33,    33,
     0,    38,    40,    30,     0,     0,     0,    33,    35,     0,
    37,     0,     0,     0,     0,    36,     0,    33,    33,    39,
    41,     0,     0
};

static const short yydefgoto[] = {     1,
    12,    73,   143,   144,   225,   226,   227,    40,    75,   233,
   234,   241,   235,   236,   255,   256,    15,    16,    50,    54,
    51,    34,    52,   149,    18,    72,   139,   140,   141,    19,
    35,    20,    71,   136,   181,   182,   187,   188,    21,    77,
    78,    22,    69,    70,   115,   129,   107,   220
};

static const short yypact[] = {-32768,
    13,   -73,   -66,   -18,    25,    53,    70,    80,    88,    72,
-32768,-32768,-32768,-32768,-32768,     5,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,    35,    37,    96,   105,-32768,-32768,
   113,   156,   160,   156,    12,    44,-32768,-32768,-32768,   171,
   175,   156,   156,   156,-32768,   -75,-32768,-32768,   156,   122,
-32768,   363,-32768,   -42,   363,-32768,-32768,-32768,   190,-32768,
   283,   202,   188,   206,   257,   204,   211,   215,    33,-32768,
   203,   -23,   284,-32768,   230,   228,   120,-32768,-32768,-32768,
-32768,-32768,   116,   156,   156,   156,   156,   156,   156,   156,
   156,   156,   156,   156,   156,   156,   156,   156,   156,   156,
   156,   156,   156,-32768,   156,-32768,   232,-32768,-32768,   -12,
-32768,-32768,-32768,-32768,-32768,   289,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   292,
-32768,-32768,    44,-32768,   213,   163,   243,-32768,   121,-32768,
   -14,   418,     6,-32768,   419,   421,   175,-32768,   156,-32768,
   363,   343,   380,   324,   395,   409,   422,   235,   235,   235,
   235,   235,   235,    64,    64,   242,   242,-32768,-32768,-32768,
   363,   423,   425,-32768,   420,   398,-32768,-32768,-32768,-32768,
    29,-32768,   365,-32768,   427,   163,   132,-32768,   424,   -23,
-32768,-32768,   429,-32768,   372,   284,-32768,-32768,-32768,    15,
-32768,   429,-32768,   431,-32768,-32768,   432,-32768,-32768,   163,
-32768,   371,-32768,-32768,   426,-32768,-32768,-32768,-32768,   376,
   111,-32768,-32768,   429,   150,   382,-32768,   436,-32768,   384,
   385,-32768,   140,-32768,-32768,-32768,   426,-32768,   156,   156,
   426,    73,-32768,-32768,   135,   154,   150,-32768,   150,   150,
   143,   387,   387,-32768,   226,   268,   388,   150,-32768,   389,
-32768,   156,   -35,   156,   266,-32768,   285,   150,   150,   387,
   387,   466,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,   273,   229,-32768,     0,-32768,-32768,   -30,
-32768,-32768,     2,   231,-32768,-32768,-32768,-32768,-32768,-32768,
    -1,-32768,   -32,-32768,-32768,-32768,-32768,   281,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,   291,-32768,  -109,-32768,-32768,
   327,-32768,-32768,   342,-32768,-32768,-32768,-32768
};


#define	YYLAST		475


static const short yytable[] = {    17,
    13,    55,    14,    23,   266,    23,   137,    25,    82,    79,
    80,    81,   272,     2,    24,   192,    83,    85,    86,    87,
    88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
    98,    99,   100,   101,   102,   103,    56,   104,     2,   173,
   105,    57,   242,     3,     4,     5,     6,     7,     8,   138,
    26,   151,   152,   153,   154,   155,   156,   157,   158,   159,
   160,   161,   162,   163,   164,   165,   166,   167,   168,   169,
   170,   193,   171,   178,   179,   180,   209,    59,    27,    32,
    99,   100,   101,   102,   103,   195,     9,    33,   196,    10,
    11,    60,    61,    58,    62,    28,    63,    31,     2,   217,
   223,    64,    65,    66,    67,    68,   205,    29,    17,   230,
   133,   174,    30,   231,   134,    36,   200,    37,    85,    86,
    87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
    97,    98,    99,   100,   101,   102,   103,    85,    86,    87,
    88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
    98,    99,   100,   101,   102,   103,    85,    86,    87,    88,
    89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
    99,   100,   101,   102,   103,     2,    38,    42,    43,    44,
    45,    46,    47,    39,    48,    53,   230,   183,  -111,    23,
   231,    17,  -111,    41,   214,   150,    74,   147,   190,    76,
    17,   148,   191,   218,    84,    17,   245,   246,   222,   210,
   184,   185,   186,   211,   249,   106,   251,   242,   252,   253,
   242,   243,    17,    17,   254,   229,   110,   263,   130,   265,
   232,   267,   116,   250,    49,   131,   244,   270,   271,   132,
    17,   111,   112,   113,   114,    17,   135,    17,    17,    97,
    98,    99,   100,   101,   102,   103,    17,   178,   179,   180,
   101,   102,   103,   257,   258,   259,    17,    17,    85,    86,
    87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
    97,    98,    99,   100,   101,   102,   103,    85,    86,    87,
    88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
    98,    99,   100,   101,   102,   103,   117,   108,   109,   260,
   261,   142,   145,   146,   172,   175,   118,   176,   189,   119,
   120,   121,   122,   123,   124,   125,   126,   127,   128,    88,
    89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
    99,   100,   101,   102,   103,   268,    86,    87,    88,    89,
    90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
   100,   101,   102,   103,   269,    85,    86,    87,    88,    89,
    90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
   100,   101,   102,   103,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,    89,    90,    91,    92,    93,    94,    95,    96,    97,
    98,    99,   100,   101,   102,   103,    90,    91,    92,    93,
    94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
    91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
   101,   102,   103,   194,   197,   198,   203,   204,   201,   202,
   207,   208,   215,   212,     2,   219,   224,   221,   228,   237,
   238,     8,   239,   240,   242,   273,   262,   264,   216,   247,
   213,   206,   248,   199,   177
};

static const short yycheck[] = {     1,
     1,    34,     1,    79,    40,    79,    30,    26,    84,    42,
    43,    44,     0,    26,    81,    30,    49,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    25,    80,    26,    52,
    83,    30,    78,    31,    32,    33,    34,    35,    36,    73,
    26,    84,    85,    86,    87,    88,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,   101,   102,
   103,    86,   105,    45,    46,    47,   186,    34,    26,    75,
    17,    18,    19,    20,    21,    80,    74,    83,    83,    77,
    78,    48,    49,    82,    51,    26,    53,    26,    26,    85,
   210,    58,    59,    60,    61,    62,    78,    28,   110,    37,
    78,   110,    25,    41,    82,    81,   149,    81,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    26,    81,    22,    23,    24,
    25,    26,    27,    79,    29,    26,    37,    25,    78,    79,
    41,   193,    82,    81,   193,    80,    26,    78,    78,    25,
   202,    82,    82,   202,    83,   207,   239,   240,   207,    78,
    48,    49,    50,    82,    80,    26,   247,    78,   249,   250,
    78,    82,   224,   225,    82,   224,    25,   258,    25,   262,
    81,   264,    27,    80,    79,    25,   237,   268,   269,    25,
   242,    54,    55,    56,    57,   247,    44,   249,   250,    15,
    16,    17,    18,    19,    20,    21,   258,    45,    46,    47,
    19,    20,    21,    38,    39,    40,   268,   269,     3,     4,
     5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
    15,    16,    17,    18,    19,    20,    21,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,    50,    25,    26,    42,
    43,    28,    83,    86,    83,    27,    60,    26,    76,    63,
    64,    65,    66,    67,    68,    69,    70,    71,    72,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    80,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,    80,     3,     4,     5,     6,     7,
     8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
    18,    19,    20,    21,     5,     6,     7,     8,     9,    10,
    11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
    21,     7,     8,     9,    10,    11,    12,    13,    14,    15,
    16,    17,    18,    19,    20,    21,     8,     9,    10,    11,
    12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
     9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
    19,    20,    21,    26,    26,    25,    27,    50,    26,    25,
    86,    25,    81,    30,    26,    25,    86,    26,    83,    78,
    25,    36,    79,    79,    78,     0,    79,    79,   196,   241,
   190,   181,   242,   147,   133
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 8:
#line 128 "parse.y"
{ eval_sequence(yyvsp[0].seqval);
			  free_sequence(yyvsp[0].seqval);
			;
    break;}
case 11:
#line 134 "parse.y"
{ char *conv;
			  conv = UstrtoFilename(yyvsp[0].utval);
			  if (strstr(conv, ".so")) {
			      if (!load_library(conv)) {
				if (!lex_open_file(conv)) {
				  yyerror("Include file or library not found.");
				}
			      }
			  } else if (!lex_open_file(conv)) {
			      yyerror("Include file not found");
			  }
			  free(yyvsp[0].utval);
			;
    break;}
case 12:
#line 149 "parse.y"
{ new_local_variables();
			  yyval.tval=malloc(strlen(yyvsp[-1].tval)+1);
			  strcpy(yyval.tval, yyvsp[-1].tval);
			;
    break;}
case 13:
#line 154 "parse.y"
{ Value *lvlist;
			  char **lnames;
			  int nrlvar;
			  lnames=get_local_names();
			  lvlist=get_local_variables(&nrlvar);
			  user_define(yyvsp[-6].tval, NULL, yyvsp[-5].protval,
				      nrlvar, lvlist, lnames, yyvsp[-1].spval.first);
			;
    break;}
case 14:
#line 164 "parse.y"
{ yyval.protval=ProtoEmpty; ;
    break;}
case 15:
#line 166 "parse.y"
{ yyval.protval=create_prototype(NULL);
			  add_type(yyval.protval, yyvsp[0].ival);
			;
    break;}
case 16:
#line 170 "parse.y"
{ add_type(yyvsp[-2].protval, yyvsp[0].ival); yyval.protval=yyvsp[-2].protval;
			;
    break;}
case 17:
#line 174 "parse.y"
{ define_local_variable(yyvsp[-1].ival, yyvsp[0].tval);
			  yyval.ival=yyvsp[-1].ival;
			;
    break;}
case 22:
#line 185 "parse.y"
{ current_type=yyvsp[0].ival; ;
    break;}
case 24:
#line 189 "parse.y"
{ define_local_variable(current_type, yyvsp[0].tval); ;
    break;}
case 25:
#line 191 "parse.y"
{ define_local_variable(current_type, yyvsp[0].tval); ;
    break;}
case 26:
#line 194 "parse.y"
{ yyval.spval = yyvsp[0].spval; ;
    break;}
case 27:
#line 196 "parse.y"
{ yyval.spval = yyvsp[0].spval; ;
    break;}
case 28:
#line 198 "parse.y"
{ if (!(yyvsp[-2].spval.first)) yyval.spval=yyvsp[0].spval;
			  else if (!(yyvsp[0].spval.first)) yyval.spval=yyvsp[-2].spval;
			  else {
			      yyval.spval.first=yyvsp[-2].spval.first;
			      yyvsp[-2].spval.last->next=yyvsp[0].spval.first;
			      yyval.spval.last=yyvsp[0].spval.last;
			  }
			;
    break;}
case 29:
#line 208 "parse.y"
{ new_local_variables(); /* open variable block */ ;
    break;}
case 30:
#line 210 "parse.y"
{ get_local_variables(NULL); /* close variable block */
			  yyval.spval=yyvsp[-1].spval;
			;
    break;}
case 31:
#line 215 "parse.y"
{ yyval.seqval=yyvsp[0].seqval; ;
    break;}
case 32:
#line 217 "parse.y"
{ yyval.seqval=seq_expression(yyvsp[0].eval); ;
    break;}
case 33:
#line 220 "parse.y"
{ yyval.spval.first=yyval.spval.last=0; ;
    break;}
case 34:
#line 222 "parse.y"
{ yyval.spval.first=yyval.spval.last=yyvsp[0].seqval; ;
    break;}
case 35:
#line 224 "parse.y"
{ yyval.spval.first = seq_if_statement(yyvsp[-4].eval, yyvsp[-2].spval.first, yyvsp[-2].spval.last,
						yyvsp[-1].spval.first, yyvsp[-1].spval.last, &(yyval.spval.last));
			;
    break;}
case 36:
#line 228 "parse.y"
{ yyval.spval.first = seq_elseif(yyvsp[-3].spval.first, yyvsp[-3].spval.last, NULL,
						yyvsp[-1].spval.first, yyvsp[-1].spval.last, &(yyval.spval.last));
			  yyvsp[-3].spval=yyval.spval;
			  yyval.spval.first = seq_if_statement(yyvsp[-6].eval, yyvsp[-4].spval.first, yyvsp[-4].spval.last,
						yyvsp[-3].spval.first, yyvsp[-3].spval.last, &(yyval.spval.last));
			;
    break;}
case 37:
#line 235 "parse.y"
{ yyval.spval.first = seq_do_statement(yyvsp[-4].eval, yyvsp[-2].spval.first, yyvsp[-2].spval.last,
						yyvsp[-1].spval.first, yyvsp[-1].spval.last, &(yyval.spval.last));
			;
    break;}
case 38:
#line 240 "parse.y"
{ yyval.spval.first=yyval.spval.last=0; ;
    break;}
case 39:
#line 242 "parse.y"
{ yyval.spval.first = seq_elseif(yyvsp[-5].spval.first, yyvsp[-5].spval.last, yyvsp[-2].eval,
						yyvsp[0].spval.first, yyvsp[0].spval.last, &(yyval.spval.last));
			;
    break;}
case 40:
#line 247 "parse.y"
{ yyval.spval.first=yyval.spval.last=0; ;
    break;}
case 41:
#line 249 "parse.y"
{ yyval.spval.first=seq_elsedo(yyvsp[-5].spval.first, yyvsp[-5].spval.last, yyvsp[-2].eval,
					     yyvsp[0].spval.first, yyvsp[0].spval.last, &(yyval.spval.last));
			;
    break;}
case 42:
#line 254 "parse.y"
{ if (yyvsp[-2].alval.nr < yyvsp[0].elval.nr)
				yyerror("Too many expressions in "
					"concurrent assignment");
			  else if (yyvsp[-2].alval.nr > yyvsp[0].elval.nr)
				yyerror("Too many variables in "
					"concurrent assignment");
			  else
				yyval.seqval=seq_assign(yyvsp[-2].alval.nr, yyvsp[-2].alval.list, yyvsp[0].elval.first);
			;
    break;}
case 43:
#line 265 "parse.y"
{ yyval.alval.nr=1;
			  yyval.alval.list= (Argument*) malloc(sizeof(Argument));
			  yyval.alval.list[0]=lookup_variable(yyvsp[0].tval);
			;
    break;}
case 44:
#line 270 "parse.y"
{ yyval.alval.nr=yyvsp[-2].alval.nr+1;
			  yyval.alval.list = (Argument*)
				realloc(yyvsp[-2].alval.list, sizeof(Argument)*yyval.alval.nr);
			  yyval.alval.list[yyvsp[-2].alval.nr]=lookup_variable(yyvsp[0].tval);
			;
    break;}
case 45:
#line 277 "parse.y"
{ yyval.elval.first =
			    yyval.elval.last =
			    combine_expression(yyvsp[0].eval,0,0);
			  yyval.elval.nr=1;
			;
    break;}
case 46:
#line 283 "parse.y"
{ yyval.elval.first = yyvsp[-2].elval.first;
			  yyval.elval.last = combine_expression(yyvsp[0].eval,0,0);
	                  combine_expression(yyvsp[-2].elval.last, yyval.elval.last, 0);
			  yyval.elval.nr=yyvsp[-2].elval.nr+1;
			;
    break;}
case 47:
#line 290 "parse.y"
{ yyval.elval.first = yyval.elval.last = 0; yyval.elval.nr=0; ;
    break;}
case 48:
#line 292 "parse.y"
{ yyval.elval.first = yyval.elval.last = yyvsp[0].eval; yyval.elval.nr=1; ;
    break;}
case 49:
#line 294 "parse.y"
{ yyval.elval.first = yyvsp[-2].elval.first;
			  yyval.elval.last = yyvsp[0].eval;
	                  combine_expression(yyvsp[-2].elval.last, yyvsp[0].eval, 0);
			  yyval.elval.nr=yyvsp[-2].elval.nr+1;
			;
    break;}
case 50:
#line 301 "parse.y"
{ yyval.fref.func = lookup_user_function(yyvsp[-1].tval);
			  if (!(yyval.fref.func)) {
				yyval.fref.func = lookup_function(yyvsp[-1].tval);
				yyval.fref.is_userfunc=0;
			  } else { yyval.fref.is_userfunc=1; }
			  if (!(yyval.fref.func)) {
				yyerror("Undefined function.");
				yyerror(yyvsp[-1].tval);
			  }
			;
    break;}
case 51:
#line 312 "parse.y"
{ yyval.eval=func_expression(yyvsp[-1].elval.first, yyvsp[-1].elval.nr, yyvsp[-2].fref.func,
					yyvsp[-2].fref.is_userfunc);
			  if (!yyval.eval) { yyerror("Incorrect functioncall."); }
			;
    break;}
case 52:
#line 318 "parse.y"
{ yyval.eval=make_expression(lookup_int_constant(yyvsp[0].ival)); ;
    break;}
case 53:
#line 320 "parse.y"
{ yyval.eval=make_expression(lookup_string_constant(yyvsp[0].utval)); ;
    break;}
case 54:
#line 322 "parse.y"
{ yyval.eval=make_expression(lookup_real_constant(yyvsp[0].rval)); ;
    break;}
case 55:
#line 324 "parse.y"
{ yyval.eval=yyvsp[0].eval; ;
    break;}
case 56:
#line 326 "parse.y"
{ Argument a;
			  a=lookup_variable(yyvsp[0].tval);
			  if (!a) {
			       yyerror("Undefined identifier");
			       yyerror(yyvsp[0].tval);
			  }
			  yyval.eval=make_expression(a);
			;
    break;}
case 57:
#line 335 "parse.y"
{ yyval.aval=lookup_variable(yyvsp[-1].tval);
			  if (!(yyval.aval)) {
				yyerror("Undefined identifier");
				yyerror(yyvsp[-1].tval);
				/* conerror=1; */
			  }
			;
    break;}
case 58:
#line 343 "parse.y"
{
			  yyval.eval=combine_expression(make_expression(yyvsp[-2].aval),
					      yyvsp[-1].eval, OPARRAY);
			;
    break;}
case 59:
#line 348 "parse.y"
{ yyval.eval=make_lazy_expression(yyvsp[0].eval); ;
    break;}
case 60:
#line 350 "parse.y"
{ yyval.eval=yyvsp[-1].eval; ;
    break;}
case 61:
#line 352 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPADD); ;
    break;}
case 62:
#line 354 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPSUB); ;
    break;}
case 63:
#line 356 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPMULT); ;
    break;}
case 64:
#line 358 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPDIV); ;
    break;}
case 65:
#line 360 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPREMAIN); ;
    break;}
case 66:
#line 362 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPLOGICAND); ;
    break;}
case 67:
#line 364 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPLOGICOR); ;
    break;}
case 68:
#line 366 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPLOGICXOR); ;
    break;}
case 69:
#line 368 "parse.y"
{ yyval.eval=combine_expression(yyvsp[0].eval,0,OPLOGICNOT); ;
    break;}
case 70:
#line 370 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPAND); ;
    break;}
case 71:
#line 372 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPOR); ;
    break;}
case 72:
#line 374 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPXOR); ;
    break;}
case 73:
#line 376 "parse.y"
{ yyval.eval=combine_expression(yyvsp[0].eval,0,OPNOT); ;
    break;}
case 74:
#line 378 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPEQUAL); ;
    break;}
case 75:
#line 380 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPNOTEQUAL); ;
    break;}
case 76:
#line 382 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPGREATER); ;
    break;}
case 77:
#line 384 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPLESS); ;
    break;}
case 78:
#line 386 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPLESSEQUAL); ;
    break;}
case 79:
#line 388 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPGREATEREQUAL); ;
    break;}
case 80:
#line 390 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPSHIFTLEFT); ;
    break;}
case 81:
#line 392 "parse.y"
{ yyval.eval=combine_expression(yyvsp[-2].eval,yyvsp[0].eval,OPSHIFTRIGHT); ;
    break;}
case 82:
#line 395 "parse.y"
{ current_keymap=get_map(LocaletoUstr(yyvsp[-1].tval)); ;
    break;}
case 83:
#line 397 "parse.y"
{ current_keymap=0; ;
    break;}
case 87:
#line 404 "parse.y"
{ clear_keymap(current_keymap); ;
    break;}
case 88:
#line 406 "parse.y"
{ define_keysequence(current_keymap, yyvsp[-2].klistval.len, yyvsp[-2].klistval.key,
					     yyvsp[-2].klistval.mode, yyvsp[0].seqval);
			;
    break;}
case 89:
#line 410 "parse.y"
{ define_keyrange(current_keymap, yyvsp[-4].keyval.key, yyvsp[-2].keyval.key,
					  yyvsp[-4].keyval.mode, yyvsp[0].seqval);
			;
    break;}
case 90:
#line 415 "parse.y"
{ yyval.klistval.key=keys; yyval.klistval.mode=mode; yyval.klistval.len=1;
			  yyval.klistval.key[0]=yyvsp[0].keyval.key; yyval.klistval.mode[0]=yyvsp[0].keyval.mode;
			;
    break;}
case 91:
#line 419 "parse.y"
{ yyval.klistval=yyvsp[-1].klistval;
			  yyval.klistval.key[yyval.klistval.len]=yyvsp[0].keyval.key;
			  yyval.klistval.mode[yyval.klistval.len]=yyvsp[0].keyval.mode;
			  yyval.klistval.len++;
			;
    break;}
case 94:
#line 429 "parse.y"
{ KeyMap *km = get_map(yyvsp[0].utval); free(yyvsp[0].utval); push_keymap(km);;
    break;}
case 95:
#line 431 "parse.y"
{ yyvsp[0].keyval.mode=(yyvsp[0].keyval.mode)>>16;
			  yyvsp[0].keyval.mode=((yyvsp[0].keyval.mode)<<16)|(yyvsp[0].keyval.mode);
			  handle_key(yyvsp[0].keyval.key, yyvsp[0].keyval.mode);
			;
    break;}
case 96:
#line 437 "parse.y"
{ current_menu = popup_define(LocaletoUstr(yyvsp[-1].tval));
			;
    break;}
case 97:
#line 440 "parse.y"
{ popup_store(current_menu); ;
    break;}
case 102:
#line 449 "parse.y"
{ popup_pinable(current_menu); ;
    break;}
case 103:
#line 451 "parse.y"
{ popup_direction(current_menu, 0); ;
    break;}
case 104:
#line 453 "parse.y"
{ popup_direction(current_menu, 1); ;
    break;}
case 108:
#line 460 "parse.y"
{ popup_add_separator(current_menu); ;
    break;}
case 109:
#line 462 "parse.y"
{ popup_set_title(current_menu, yyvsp[0].utval); ;
    break;}
case 110:
#line 464 "parse.y"
{ popup_add_line(current_menu, yyvsp[-2].utval, yyvsp[0].seqval); ;
    break;}
case 111:
#line 466 "parse.y"
{ popup_add_submenu(current_menu, yyvsp[-2].utval, LocaletoUstr(yyvsp[0].tval)); ;
    break;}
case 112:
#line 468 "parse.y"
{ popup_make_default(current_menu); ;
    break;}
case 117:
#line 477 "parse.y"
{ set_translation(yyvsp[-2].utval, yyvsp[0].utval); ;
    break;}
case 150:
#line 517 "parse.y"
{ ; ;
    break;}
case 151:
#line 519 "parse.y"
{ ; ;
    break;}
case 152:
#line 521 "parse.y"
{ ; ;
    break;}
case 153:
#line 523 "parse.y"
{ ; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 525 "parse.y"

/*
**  lex related functions
*/

typedef struct FILESTACK FILESTACK;

struct FILESTACK {
    FILE *finput;  /* the file */
    char *memory;  /* the internal string */
    char *name;    /* the name of the file (or a pointer in memory) */
    int linecount; /* number of lines processed so far */
    char *buffer;  /* unput buffer (shared) */
    int bufpos;    /* number of unput characters */
    FILESTACK *next; /* next file */
};

static FILESTACK *inputstack=0;
static char lex_unput_buffer[1024];
#define MAXIDENTIFIER 10
#define IDENTLENGTH 256
static char ident_buffer[MAXIDENTIFIER][IDENTLENGTH];
static int current_id=0;

static int current_line_number(void)
{
    return (!inputstack? 0: inputstack->linecount);
}

static char *current_file(void)
{
    return (!inputstack? "stdin" : inputstack->finput ? inputstack->name : "string");
}

static int layout_input(void)
{
    int c;
    if (!inputstack) return 0;
    if (inputstack->bufpos) c= (unsigned)(inputstack->buffer[--inputstack->bufpos]);
    else if (inputstack->finput) c=getc(inputstack->finput);
    else c=(unsigned)(*inputstack->name++);
    if (c=='\n') inputstack->linecount++;
    if (c==EOF) return 0;
    else return c;
}

static void layout_unput(char c)
{
    if (c=='\n') inputstack->linecount--;
    inputstack->buffer[inputstack->bufpos++]=c;
}

static void layout_output(char c)
{
    putc(c,stdout);
}

#include "filefind.h"
static PathInfo mpkpath = 0;

int lex_open_file(char *name)
{
    FILESTACK *fstack;
    FILE *f;
    int i;
    if (!mpkpath) {
      mpkpath = make_pathinfo("MPKPATH", DEFAULTMPKPATH, ".mpk");
    }
    f = open_file(mpkpath,name, "rb");
    if (!f) return 0;
    fstack = (FILESTACK*) malloc(sizeof(FILESTACK));
    fstack->finput=f;
    fstack->memory=0;
    fstack->name=malloc(strlen(name)+1);
    strcpy(fstack->name, name);
    fstack->linecount=1;
    fstack->next = inputstack;
    /* lex uses look-ahead and remembers which are unput */
    if (inputstack) {
        fstack->buffer = inputstack->buffer+inputstack->bufpos;
        for (i=0; i<inputstack->bufpos; i++) {
            fstack->buffer[i]=inputstack->buffer[i];
            if (fstack->buffer[i]=='\n') fstack->linecount--;
            else if (!isspace(fstack->buffer[i]))
                printf("Unput character %c.\n", fstack->buffer[i]);
        }
        fstack->bufpos=inputstack->bufpos;
    } else {
        fstack->buffer = lex_unput_buffer;
        fstack->bufpos=0;
    }
    inputstack=fstack;
    return 1;
}

void lex_open_string(char *s)
{
    FILESTACK *fstack;
    int i;
    fstack = (FILESTACK*) malloc(sizeof(FILESTACK));
    fstack->finput=0;
    fstack->memory=s;
    fstack->name=s;
    fstack->linecount=1;
    fstack->next = inputstack;
    /* lex uses look-ahead and remembers which are unput */
    if (inputstack) {
        fstack->buffer = inputstack->buffer+inputstack->bufpos;
        for (i=0; i<inputstack->bufpos; i++) {
            fstack->buffer[i]=inputstack->buffer[i];
            if (fstack->buffer[i]=='\n') fstack->linecount--;
            else if (!isspace(fstack->buffer[i]))
                printf("Unput character %c.\n", fstack->buffer[i]);
        }
        fstack->bufpos=inputstack->bufpos;
    } else {
        fstack->buffer = lex_unput_buffer;
        fstack->bufpos=0;
    }
    inputstack=fstack;
}

static int layout_wrapup(void)
{
    FILESTACK *fstack;
    if (inputstack) {
        if (inputstack->finput) {
            close_file(inputstack->finput);
            free(inputstack->name);
        }
        fstack=inputstack;
        inputstack=inputstack->next;
        free(fstack);
    }
    return (inputstack? 0 : 1);
}

static void convertslash(Uchar *res, Uchar *s)
{
    int i=0,j=0,k;
    while (s[i]) {
        if (s[i]!='\\') {
            res[j++]=s[i++];
        } else {
            i++;
            if (Uisdigit(s[i]) && Utovalue(s[i])<8) {
                res[j]=0;
                k=0;
                do {
                    k++;
                    res[j]=res[j]*8+Utovalue(s[i]);
		    i++;
                } while (k<3 && Uisdigit(s[i]) && Utovalue(s[i])<8);
                j++;
            } else if (s[i]=='x' && Uisxdigit(s[i+1])) {
                res[j]=0;
                k=0;
                i++;
                do {
                    k++;
                    res[j]=res[j]*16+Utoxvalue(s[i]);
                    i++;
                } while (k<2 && Uisxdigit(s[i]));
                j++;
            } else if (s[i]=='u' && Uisxdigit(s[i+1])) {
 	        res[j]=0;
	        k=0;
	        i++;
	        do {
		    k++;
                    res[j]=res[j]*16+Utoxvalue(s[i]);
                    i++;
                } while (k<4 && Uisxdigit(s[i]));
                j++;
	    } else {
                switch (s[i]) {
                case 'a': res[j++]='\a'; break;
                case 'b': res[j++]='\b'; break;
                case 'f': res[j++]='\f'; break;
                case 'n': res[j++]='\n'; break;
                case 'r': res[j++]='\r'; break;
                case 't': res[j++]='\t'; break;
                case 'v': res[j++]='\v'; break;
                default: 
		  if (s[i]) res[j++]=s[i];
		  break;
                }
                i++;
            }
        }
    }
    res[j]='\0';
}

static void convertslashascii(char *res, char *s, int limit)
{
    int i=0,j=0,k;
    while (i<limit) {
        if (s[i]!='\\') {
            res[j++]=s[i++];
        } else {
            i++;
            if (isdigit(s[i]) && s[i]<'8') {
                res[j]=0;
                k=0;
                do {
                    k++;
                    res[j]=res[j]*8+s[i]-'0';
		    i++;
                } while (k<3 && isdigit(s[i]) && s[i]<'8');
                j++;
            } else if (s[i]=='x' && isxdigit(s[i+1])) {
                res[j]=0;
                k=0;
                i++;
                do {
                    k++;
                    if (isdigit(s[i])) res[j]=res[j]*16+s[i]-'0';
                    else res[j]=res[j]*16+(toupper(s[i])+10-'A');
                    i++;
                } while (k<2 && isxdigit(s[i]));
                j++;
	    } else {
                switch (s[i]) {
                case 'a': res[j++]='\a'; break;
                case 'b': res[j++]='\b'; break;
                case 'f': res[j++]='\f'; break;
                case 'n': res[j++]='\n'; break;
                case 'r': res[j++]='\r'; break;
                case 't': res[j++]='\t'; break;
                case 'v': res[j++]='\v'; break;
                default: 
		  if (s[i]) res[j++]=s[i];
		  break;
                }
                i++;
            }
        }
    }
    res[j]='\0';
}

void parse_input(void)
{
	yyparse();
}

#include "parselex.c"

int yyerror(char *s)
{
	fprintf( stderr, "%s:%i: %s\n", current_file(),
		 current_line_number(),s);
	return (0);
}

