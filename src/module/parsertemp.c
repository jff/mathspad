
#include <stdlib.h>
#include <stdio.h>

#include "mathpad.h"
#include "notatype.h"
#include "latexout.h"
#include "match.h"
#include "language.h"
#include "message.h"

static void add_format_to_parser(Char *c, int len, int ivnr, Char mode,
				 int tabbing, int pmode, int prec,
				 int shortop)
{
    int i,j,n=1,llp;
    char *str=NULL;
    Char *tmp;
    j=0;
    for (i=0;i<len;i++) if (IsPh(c[i])) n++;
    n=n*2;
    tmp = (Char*) malloc(n*sizeof(Char));
    tex_set_string(&str);
    if (tabbing) out_latex_char(TabOpen);
    out_latex_char(mode);
    llp=tex_current_pos();
    if (shortop) tex_code(SOpOpen);
    n=0;
    for (i=0; i<len; i++) {
	if (IsPh(c[i]))  {
	    switch (Ph(c[i])) {
	    case MP_Expr: tex_code(ExprOpen); break;
	    case MP_Op:   tex_code(LOpOpen); break;
	    case MP_Id:   tex_code(SIdOpen); break;
	    case MP_Var:  tex_code(VarOpen); break;
	    case MP_Text: tex_code(TextOpen); break;
	    default: break;
	    }
	    out_latex_char(' ');
	    j=tex_current_pos()-1;
	    if (j!=llp && str[j]==' ') {
		str[j]='\0';
		if (aig(tmp[n]=lex_add_string(str+llp, 0)))
		    n++;
		str[j]=' ';
	    }
	    llp=j+1;
	    switch (Ph(c[i])) {
	    case MP_Expr: tex_code(ExprClose); break;
	    case MP_Op:   tex_code(LOpClose); break;
	    case MP_Id:   tex_code(SIdClose); break;
	    case MP_Var:  tex_code(VarClose); break;
	    case MP_Text: tex_code(TextClose); break;
	    default: break;
	    }
	    tmp[n++]=c[i];
	}
	out_latex_char(c[i]);
    }
    if (shortop) tex_code(SOpClose);
    if (llp!=tex_current_pos()) tmp[n++]=lex_add_string(str+llp,0);
    tmp[n]=0;
    tex_unset();
    if (!str)
	free(tmp);
    else
	if (!parse_add_rule(pmode, tmp, n, ivnr, prec))
	    free(tmp);
}

static void add_version_to_parser(VERSION *vers, int pmode, int prec)
{
    Char *c=vers->format[LATEXFORMAT];
    int i=vers->max[LATEXFORMAT];
    int j,n,shortop=0;
    if (!c) {
	c=vers->format[SCREENFORMAT];
	i=vers->max[SCREENFORMAT];
    }
    for (j=n=0;j<i;j++) if (c[j]>SoftNewline && c[j]!=Newline) n++;
    if (i==1 && (pmode&INFIX) && !IsPh(c[0]) && !IsTab(c[0])) shortop=1;
    if (vers->latexmode & LTEXTMODE) {
	add_format_to_parser(c, i, vers->ivnr, InText,n, pmode, prec, 0);
	if (shortop)
	    add_format_to_parser(c, i, vers->ivnr, InText,n, pmode, prec, 1);
    }
    if (vers->latexmode & LMATHMODE) {
	add_format_to_parser(c, i, vers->ivnr, InMath,n, pmode|EXPR, prec, 0);
	if (shortop)
	    add_format_to_parser(c, i, vers->ivnr, InMath,n,
				 pmode|EXPR, prec, 1);
    }
}

void add_template_to_parser(NOTATION *nota)
{
    int i;
    int pmode;
    switch (nota->kind) {
    case None: pmode=TEXT; break;
    case Prefix: pmode = PREFIX; break;
    case Postfix: pmode = POSTFIX; break;
    default: pmode = INFIX; break;
    }
    for (i=0; i<nota->versions; i++)
	add_version_to_parser(nota->vers+i,pmode, nota->prec);
}

void add_file_to_parser(Char *name)
{
    int i,j,n;
    int *c;
    int filenr;

    filenr = load_notation_window(-1, name);
    if (filenr>=0) {
      for (j=0; j<MAX_KIND; j++) {
	NOTATION *nota;
	i=0;
	nota = get_notation_kind(filenr,j,i);
	while (nota) {
	  add_template_to_parser(nota);
	  i++;
	  nota=get_notation_kind(filenr, j,i);
	}
      }
    } else {
      message(MP_MESSAGE,
	      translate("Unable to add the stencils to the parser"));
    }
}

void clear_parser(void)
{
    parse_use_rules(NULL);
}

void parse_selection(void)
{
  char *psel=NULL;
  tex_set_string(&psel);
  tex_placeholders(1);
  tex_mode(4);
  latex_selection(1);
  tex_unset();
  if (psel && parse_text(psel)) include_selection();
  cleanup_nodestack();
  free(psel);
}


/*
extern void add_file_to_parser(Char *name);
extern void clear_parser(void);
*/

static Type tlist[3][4] = {
  { 0, 0},
  { StringType, 0},
  { IntType, 0},
};

static int call_int(int (*fcal)(), void **args)
{
  return (*fcal)(*((int*)args[0]));
}

static int call_string(int (*fcal)(), void **args)
{
  return (*fcal)(*((Uchar**)args[0]));
}

static int call_empty(int (*fcal)(), void **args)
{
  return (*fcal)();
}

int init_library(void)
{
  Prototype *pt;

  pt = define_prototype(tlist[0], 0,0, call_empty);
  define_function("parser_clear", "Clear the parser",
		  pt, clear_parser);
  define_function("parser_init", "Init the parser",
		  pt, parser_init);
  define_function("parse_selection", "Parse the selection",
		  pt, parse_selection);
  pt = define_prototype(tlist[1], 1,0,call_string);
  define_function("parser_add_file", "Add a file with templates to the parser",
		  pt, add_file_to_parser);
  pt = define_prototype(tlist[2],1,0,call_int);
  define_function("parser_case_insensitive",
		  "Set case insensitivity of the parser",
		  pt, set_parser_case);
  define_function("parser_space_insensitive",
		  "Set space insensitivity of the parser",
		  pt, set_parser_space);
}


