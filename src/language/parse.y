%{
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

%}
%union {
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
	}
%left LOGICOR
%left LOGICXOR
%left LOGICAND
%left BITOR
%left BITXOR
%left BITAND
%left EQUAL LESS GREATER LESSEQUAL GREATEREQUAL NOTEQUAL
%left SHIFTLEFT SHIFTRIGHT
%left ADD MINUS
%left MULTIPLY DIVIDE REMAINDER
%left LOGICNOT BITNOT LAZYREF
%token <utval> STRING
%token <tval> IDENTIFIER
%token <ival> INTEGER TYPEVAL
%token <rval> REAL
%token <keyval> KEY
%type <ival> proto
%type <protval> prototype
%type <eval> expression
%type <elval> exprlist assexprlist
%type <spval> sequence block elseiflist elsedolist statement
%type <seqval> funcseq assignment
%type <eval> functioncall
%type <alval> identlist
%type <klistval> keylist
%token INPUT
%token LAYOUT MENU KEYBOARD FUNCTION VARIABLE IF ELSEIF ELSE FI DO
%token ELSEDO OD OPTIONS PIN LEFTRIGHT RIGHTLEFT SEPARATOR TITLE
%token DEFAULT BUTTON IMAGE SCROLLBAR LEFT RIGHT BOTTOM TOP GEOMETRY
%token TYPE EDIT COMMENT PROGRAM CONSOLE BUFFER SYMBOL STENCIL
%token DEFINE FINDREPLACE ALL SHELL FILESELECT REMARK CLEAR INCLUDE
%token ASSIGN RANGE
%token TRANSLATION
%%
file	    :	/* empty */
	    |	file functiondef
	    |	file menudef
	    |	file keyboarddef
	    |	file layoutdef
	    |	file translation
	    |	file ';'
	    |	file funcseq
			{ eval_sequence($2);
			  free_sequence($2);
			}
	    |	file inputdef
	    |	file variabledef
	    |	file INCLUDE STRING
			{ char *conv;
			  conv = UstrtoFilename($3);
			  if (strstr(conv, ".so")) {
			      if (!load_library(conv)) {
				if (!lex_open_file(conv)) {
				  yyerror("Include file or library not found.");
				}
			      }
			  } else if (!lex_open_file(conv)) {
			      yyerror("Include file not found");
			  }
			  free($3);
			}
	    ;
functiondef :	FUNCTION IDENTIFIER '(' 
			{ new_local_variables();
			  $<tval>$=malloc(strlen($2)+1);
			  strcpy($<tval>$, $2);
			}
		prototype ')' '{' vardefs sequence '}'
			{ Value *lvlist;
			  char **lnames;
			  int nrlvar;
			  lnames=get_local_names();
			  lvlist=get_local_variables(&nrlvar);
			  user_define($<tval>4, NULL, $5,
				      nrlvar, lvlist, lnames, $9.first);
			}
	    ;
prototype   :	/* empty */
			{ $$=ProtoEmpty; }
	    |	proto
			{ $$=create_prototype(NULL);
			  add_type($$, $1);
			}
	    |	prototype ',' proto
			{ add_type($1, $3); $$=$1;
			}
	    ;
proto	    :	TYPEVAL IDENTIFIER
			{ define_local_variable($1, $2);
			  $$=$1;
			}
	    ;
vardefs	    :	/* empty */
	    |	vardeflist ';'
	    ;
vardeflist  :	variabledef
	    |	vardeflist ';' variabledef
	    ;
variabledef :	VARIABLE TYPEVAL
			{ current_type=$2; }
		defidents
	    ;
defidents   :	IDENTIFIER
			{ define_local_variable(current_type, $1); }
	    |	defidents ',' IDENTIFIER
			{ define_local_variable(current_type, $3); }
	    ;
sequence    :	statement
			{ $$ = $1; }
	    |	block
			{ $$ = $1; }
	    |	sequence ';' statement
			{ if (!($1.first)) $$=$3;
			  else if (!($3.first)) $$=$1;
			  else {
			      $$.first=$1.first;
			      $1.last->next=$3.first;
			      $$.last=$3.last;
			  }
			}
	    ;
block	    :	'{'
			{ new_local_variables(); /* open variable block */ }
		vardefs sequence '}'
			{ get_local_variables(NULL); /* close variable block */
			  $$=$4;
			}
	    ;
funcseq	    :	assignment
			{ $$=$1; }
	    |	functioncall
			{ $$=seq_expression($1); }
	    ;
statement   :   /* empty */
			{ $$.first=$$.last=0; }
	    |	funcseq
			{ $$.first=$$.last=$1; }
	    |	IF '(' expression ')' sequence elseiflist FI
			{ $$.first = seq_if_statement($3, $5.first, $5.last,
						$6.first, $6.last, &($$.last));
			}
	    |	IF '(' expression ')' sequence elseiflist ELSE sequence FI
			{ $$.first = seq_elseif($6.first, $6.last, NULL,
						$8.first, $8.last, &($$.last));
			  $6=$$;
			  $$.first = seq_if_statement($3, $5.first, $5.last,
						$6.first, $6.last, &($$.last));
			} 
	    |	DO '(' expression ')' sequence elsedolist OD
			{ $$.first = seq_do_statement($3, $5.first, $5.last,
						$6.first, $6.last, &($$.last));
			}
	    ;
elseiflist  :	/* empty */
			{ $$.first=$$.last=0; }
	    |	elseiflist ELSEIF '(' expression ')' sequence
			{ $$.first = seq_elseif($1.first, $1.last, $4,
						$6.first, $6.last, &($$.last));
			}
	    ;
elsedolist  :	/* empty */
			{ $$.first=$$.last=0; }
	    |	elsedolist ELSEDO '(' expression ')' sequence
			{ $$.first=seq_elsedo($1.first, $1.last, $4,
					     $6.first, $6.last, &($$.last));
			}
	    ;
assignment  :	identlist ASSIGN assexprlist
			{ if ($1.nr < $3.nr)
				yyerror("Too many expressions in "
					"concurrent assignment");
			  else if ($1.nr > $3.nr)
				yyerror("Too many variables in "
					"concurrent assignment");
			  else
				$$=seq_assign($1.nr, $1.list, $3.first);
			}
	    ;
identlist   :	IDENTIFIER
			{ $$.nr=1;
			  $$.list= (Argument*) malloc(sizeof(Argument));
			  $$.list[0]=lookup_variable($1);
			}
	    |	identlist ',' IDENTIFIER
			{ $$.nr=$1.nr+1;
			  $$.list = (Argument*)
				realloc($1.list, sizeof(Argument)*$$.nr);
			  $$.list[$1.nr]=lookup_variable($3);
			}
	    ;
assexprlist :	expression
			{ $$.first =
			    $$.last =
			    combine_expression($1,0,0);
			  $$.nr=1;
			}
	    |	assexprlist ',' expression
			{ $$.first = $1.first;
			  $$.last = combine_expression($3,0,0);
	                  combine_expression($1.last, $$.last, 0);
			  $$.nr=$1.nr+1;
			}
	    ;
exprlist    :	/* empty */
			{ $$.first = $$.last = 0; $$.nr=0; }
	    |	expression
			{ $$.first = $$.last = $1; $$.nr=1; }
	    |	exprlist ',' expression
			{ $$.first = $1.first;
			  $$.last = $3;
	                  combine_expression($1.last, $3, 0);
			  $$.nr=$1.nr+1;
			}
	    ;
functioncall:	IDENTIFIER '(' 
			{ $<fref>$.func = lookup_user_function($1);
			  if (!($<fref>$.func)) {
				$<fref>$.func = lookup_function($1);
				$<fref>$.is_userfunc=0;
			  } else { $<fref>$.is_userfunc=1; }
			  if (!($<fref>$.func)) {
				yyerror("Undefined function.");
				yyerror($1);
			  }
			}
		exprlist ')' 
			{ $$=func_expression($4.first, $4.nr, $<fref>3.func,
					$<fref>3.is_userfunc);
			  if (!$$) { yyerror("Incorrect functioncall."); }
			}
	    ;
expression  :	INTEGER
			{ $$=make_expression(lookup_int_constant($1)); }
	    |	STRING
			{ $$=make_expression(lookup_string_constant($1)); }
	    |   REAL
			{ $$=make_expression(lookup_real_constant($1)); }
	    |   functioncall
			{ $$=$1; }
	    |	IDENTIFIER
			{ Argument a;
			  a=lookup_variable($1);
			  if (!a) {
			       yyerror("Undefined identifier");
			       yyerror($1);
			  }
			  $$=make_expression(a);
			}
	    |	IDENTIFIER '['
			{ $<aval>$=lookup_variable($1);
			  if (!($<aval>$)) {
				yyerror("Undefined identifier");
				yyerror($1);
				/* conerror=1; */
			  }
			}
		expression ']'
			{
			  $$=combine_expression(make_expression($<aval>3),
					      $4, OPARRAY);
			}
	    |	LAZYREF expression
			{ $$=make_lazy_expression($2); }
	    |	'(' expression ')'
			{ $$=$2; }
	    |	expression ADD expression
			{ $$=combine_expression($1,$3,OPADD); }
	    |	expression MINUS expression
			{ $$=combine_expression($1,$3,OPSUB); }
	    |	expression MULTIPLY expression
			{ $$=combine_expression($1,$3,OPMULT); }
	    |	expression DIVIDE expression
			{ $$=combine_expression($1,$3,OPDIV); }
	    |	expression REMAINDER expression
			{ $$=combine_expression($1,$3,OPREMAIN); }
	    |	expression LOGICAND expression
			{ $$=combine_expression($1,$3,OPLOGICAND); }
	    |	expression LOGICOR expression
			{ $$=combine_expression($1,$3,OPLOGICOR); }
	    |	expression LOGICXOR expression
			{ $$=combine_expression($1,$3,OPLOGICXOR); }
	    |	LOGICNOT expression
			{ $$=combine_expression($2,0,OPLOGICNOT); }
	    |	expression BITAND expression
			{ $$=combine_expression($1,$3,OPAND); }
	    |	expression BITOR expression
			{ $$=combine_expression($1,$3,OPOR); }
	    |	expression BITXOR expression
			{ $$=combine_expression($1,$3,OPXOR); }
	    |	BITNOT expression
			{ $$=combine_expression($2,0,OPNOT); }
	    |	expression EQUAL expression
			{ $$=combine_expression($1,$3,OPEQUAL); }
	    |	expression NOTEQUAL expression
			{ $$=combine_expression($1,$3,OPNOTEQUAL); }
	    |	expression GREATER expression
			{ $$=combine_expression($1,$3,OPGREATER); }
	    |	expression LESS expression
			{ $$=combine_expression($1,$3,OPLESS); }
	    |	expression LESSEQUAL expression
			{ $$=combine_expression($1,$3,OPLESSEQUAL); }
	    |	expression GREATEREQUAL expression
			{ $$=combine_expression($1,$3,OPGREATEREQUAL); }
	    |	expression SHIFTLEFT expression
			{ $$=combine_expression($1,$3,OPSHIFTLEFT); }
	    |	expression SHIFTRIGHT expression
			{ $$=combine_expression($1,$3,OPSHIFTRIGHT); }
	    ;
keyboarddef :	KEYBOARD IDENTIFIER '{'
			{ current_keymap=get_map(LocaletoUstr($2)); }
		keydefs '}'
			{ current_keymap=0; }
	    ;
keydefs     :	keydef
	    |	keydefs ';' keydef
	    ;
keydef      :	/* empty */
	    |	CLEAR
			{ clear_keymap(current_keymap); }
	    |	keylist ':' funcseq
			{ define_keysequence(current_keymap, $1.len, $1.key,
					     $1.mode, $3);
			}
	    |	KEY RANGE KEY ':' funcseq
			{ define_keyrange(current_keymap, $1.key, $3.key,
					  $1.mode, $5);
			}
	    ;
keylist     :   KEY
			{ $$.key=keys; $$.mode=mode; $$.len=1;
			  $$.key[0]=$1.key; $$.mode[0]=$1.mode;
			}
	    |	keylist KEY
			{ $$=$1;
			  $$.key[$$.len]=$2.key;
			  $$.mode[$$.len]=$2.mode;
			  $$.len++;
			}
	    ;
inputdef    :	INPUT '{' inputitems '}'
	    ;
inputitems  :	/* empty */
	    |	inputitems STRING
			{ KeyMap *km = get_map($2); free($2); push_keymap(km);}
	    |   inputitems KEY
			{ $2.mode=($2.mode)>>16;
			  $2.mode=(($2.mode)<<16)|($2.mode);
			  handle_key($2.key, $2.mode);
			}
	    ;
menudef     :	MENU IDENTIFIER '{'
			{ current_menu = popup_define(LocaletoUstr($2));
			}
		options menulines '}'
			{ popup_store(current_menu); }
	    ;
options     :	/* empty */
	    |   OPTIONS opitems ';'
	    ;
opitems     :   opitem
            |   opitems opitem
	    ;
opitem      :	PIN
			{ popup_pinable(current_menu); }
	    |	LEFTRIGHT
			{ popup_direction(current_menu, 0); }
	    |	RIGHTLEFT
			{ popup_direction(current_menu, 1); }
	    ;
menulines   :	menuline
	    |	menulines ';' menuline
	    ;
menuline    :	/* empty */
	    |	SEPARATOR
			{ popup_add_separator(current_menu); }
	    |	TITLE	STRING
			{ popup_set_title(current_menu, $2); }
	    |	STRING ':' funcseq
			{ popup_add_line(current_menu, $1, $3); }
	    |	STRING ':' IDENTIFIER
			{ popup_add_submenu(current_menu, $1, LocaletoUstr($3)); }
	    |	DEFAULT menuline
			{ popup_make_default(current_menu); }
	    ;
translation :	TRANSLATION IDENTIFIER '{' translines '}'
	    ;
translines  :   transline
	    |   translines ';' transline
            ;
transline   :   /* empty */
	    |   STRING   ':'  STRING
			{ set_translation($1, $3); }
	    ;
layoutdef   :	LAYOUT IDENTIFIER '{' layoutlines '}'
	    ;
layoutlines :	layoutline
	    |	layoutlines ';' layoutline
	    ;
layoutline  :	TITLE STRING
	    |	TITLE IDENTIFIER
	    |	BUTTON STRING funcseq
	    |	BUTTON STRING IMAGE STRING funcseq
	    |	SCROLLBAR location
	    |	GEOMETRY INTEGER INTEGER INTEGER
	    |	TYPE  windowtype
	    |	KEYBOARD keyidents
	    |	SEPARATOR
	    |	EDIT STRING IDENTIFIER
	    |	EDIT STRING IDENTIFIER DEFAULT stringlist
	    |	COMMENT STRING
	    |	PROGRAM STRING
	    ;
location    :	LEFT
	    |	RIGHT
	    |	BOTTOM
	    |	TOP
	    ;
windowtype  :	CONSOLE
	    |	EDIT
	    |	BUFFER
	    |	SYMBOL
	    |	STENCIL
	    |	DEFINE
	    |	FINDREPLACE
	    |	ALL
	    |	SHELL
	    |	DEFAULT
	    |	FILESELECT
	    |	REMARK
	    ;
keyidents   :   IDENTIFIER
			{ ; }
	    |	keyidents ',' IDENTIFIER
			{ ; }
stringlist  :	STRING
			{ ; }
	    |	stringlist ',' STRING
			{ ; }
	    ;
%%
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

