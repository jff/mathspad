
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "mathpad.h"

/* make a new node (of type TYPE) on the stack, with TXT as contents.
** For each place holder in TXT, an node from the stack is inserted.
** LEN contains the length of TXT. NNR contains either the number of
** the template (if TYPE is operator) or the fontid (if TYPE is identifier).
** SPACING is used for adjusting the spacing around operaters.
** If TYPE is 0, no new node is added to the stack and nodes are removed
** according to TXT.
*/
extern void *make_node(Char type, Char *txt, int len, int nnr, int spacing);

extern int  notation_with_number(unsigned long number);

typedef struct {
    int iid;                  /* internal number (given at load time) */
    unsigned long uidnumber;  /* unique number (given at creation time) */
} TEMPINFO;

#define TSIGCONTYPE 0
#define TSIGTYPE 1
#define TNOCONTEXT 2
#define TPREDSEP 3
#define TPREDAPP 4
#define TLISTTYPE 5
#define TARROWTYPE 6
#define TTUPLETYPE 7
#define TAPTOPTYPE 8
#define TBADTYPE 9
#define TTYPESEP 10
#define TAPTYPE 11
#define TARROWOP 12
#define TUNPRED 13
#define TBADKIND 14
#define TSTARKIND 15
#define TKINDARROW 16
static TEMPINFO tmplate[] = 
{ { 0, 3003741076UL },
  { 0, 3003741065UL },
  { 0, 3003747843UL },
  { 0, 3003749356UL },
  { 0, 3003749492UL },
  { 0, 3003752905UL },
  { 0, 3003752906UL },
  { 0, 3003752907UL },
  { 0, 3003752908UL },
  { 0, 3003753306UL },
  { 0, 3003755648UL },
  { 0, 3003756613UL },
  { 0, 3003757294UL },
  { 0, 3003759084UL },
  { 0, 3003759972UL },
  { 0, 3003760137UL },
  { 0, 3003766331UL },
  { 0, 0 }
};

typedef void *TEXP;    /* expression node */
typedef void *EXP;     /* expression node */
typedef void *STRING;  /* text node */
typedef void *CONT;    /* expression node */
typedef void *CHAR;    /* identifier node */
typedef void *PRED;    /* expression node */
typedef void *NAT;     /* identifier */
typedef void *KIND;    /* expression node */

static char *parse_dummy(char **t);
static TEXP   parse_texp(char **t);
static EXP    parse_exp(char **t);
static STRING parse_string(char **t);
static CONT   parse_cont(char **t);
static CHAR   parse_char(char **t);
static PRED   parse_pred(char **t);
static NAT    parse_nat(char **t);
static KIND   parse_kind(char **t);
static TEXP parse_tsigtype(char **t);
static CONT parse_tcontext(char **t);
static PRED parse_tpredlist(char **t);
static PRED parse_tpred(char **t);
static TEXP parse_ttype(char **t);
static TEXP parse_ttycon(char **t);
static TEXP parse_ttuple(char **t);
static TEXP parse_toffset(char **t);
static TEXP parse_tintcell(char **t);
static TEXP parse_tap(char **t);
static TEXP parse_tbadtype(char **t);
static TEXP parse_typelist(char **t);
static TEXP parse_taptype(char **t);
static TEXP parse_ttypelist(char **t);
static TEXP parse_ttypearrow(char **t);
static TEXP parse_tistuple(char **t);
static TEXP parse_tclass(char **t);
static TEXP parse_tiscon(char **t);
static TEXP parse_tunpred(char **t);
static TEXP parse_texp(char **t);

#define cfree(A)  if (A) free(A)

/* parse_error_func can be used in a debugger to break at parse errors */
static void parse_error_func(char *w __attribute__((unused)))
{
    int i;
    for (i=0;i<10;i++);
}

/* skip_symbol skip the next symbol if it is correct and calls the
** error function otherwise.  Calling the bail_out macro might be an
** other option.
*/
#define skip_symbol(S,A) if (*(A)==(S)) (A)++; else parse_error_func(A)

/* bail_out is called when something went wrong. Each function that
** calls bail_out contains the variables plist and plen. plist contains
** a list of characters indicating what kind of place holders are parse
** so far. plen contains the number of characters in the list.
*/
#define bail_out()       { make_node(0,plist,plen,0,0); return 0; }

/* macros to make different kind of nodes */
#define make_tnode(A)      make_node(MP_Op,   plist,plen,tmplate[A].iid,0)
static Char dummy[2];
#define make_operator(A)   make_node(MP_Op,   dummy, 0,tmplate[A].iid,0)
#define make_expression()  make_node(MP_Expr, plist, plen, 0,0)
#define make_identifier(T) make_node(MP_Id,   plist, plen, (T),0)
#define make_text()        make_node(MP_Text, plist, plen, 0,0)

static int chartoChar(Char *b, char *c)
{
  int i=0,j=0;
  for (i=0; c[i]; i++) {
    if (c[i]=='%') {
      i++;
      switch (c[i]) {
      case 't': b[j++]=MP_Text; break;
      case 'e': b[j++]=MP_Expr; break;
      case 'i': b[j++]=MP_Id; break;
      case 'o': b[j++]=MP_Op; break;
      default: break;
      }
    } else if (c[i]=='\n') b[j++]=Newline;
    else if (c[i]=='\t') b[j++]=Rtab;
    else b[j++]=c[i];
  }
  return j;
}

static char *parse_dummy(char **t)
{
    char *s=*t;
    char *h=*t;
    while (isalnum(*s)) s++;
    *t=s;
    return h;
}

static TEXP parse_tsigtype(char **t)
{
    /* "TSigType[" [<tcontext>] "," <ttype> "]" */
    char *c=*t;
    Char plist[3];
    int plen=0;
    CONT cont; TEXP ttype;
    if (strncmp("TSigType[", c,9)) return NULL;
    c=c+9;
    cont = parse_tcontext(&c);
    if (cont) plist[plen++]=PhNum2Char(MP_Expr,2);
    skip_symbol(',',c);
    ttype = parse_ttype(&c);
    if (!ttype) bail_out();
    plist[plen++]=PhNum2Char(MP_Expr,1);
    skip_symbol(']',c);
    *t=c;
    return make_tnode((plen>1?TSIGCONTYPE:TSIGTYPE));
}

static TEXP parse_tcontext(char **t)
{
  /* "TContext[" <tpredlist> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP predlist;
    if (*c==',') return NULL;
    if (strncmp("TContext[", c, 9)) return NULL;
    c=c+9;
    if (aig(predlist = parse_tpredlist(&c))) plist[plen++]=PhNum2Char(MP_Expr,1);
    skip_symbol(']',c);
    *t=c;
    if (plen) {
      /* context is on stack. Leave it there, no template needed. */
      /* (problem: if #predlist>1, then add parenthesis */
      return predlist;
    } else {
      /* empty context. Use template */
      return make_tnode(TNOCONTEXT);
    }
}

static TEXP parse_tpredlist(char **t)
{
    /* [ <tpred> { "," <tpred> } ]  */
    char *c=*t;
    Char plist[500];
    int plen=0;
    TEXP pred;
    while (*c!=']') {
	pred=parse_tpred(&c);
	if (pred) {
	  plist[plen++]=MP_Expr;
	  if (*c==',') {
	    make_operator(TPREDSEP);
	    plist[plen++]=MP_Op;
	    c++;
	  }
	} else bail_out();
    }
    if (!plen)
      return NULL;
    else
      return make_expression();
}

static TEXP parse_tpred(char **t)
{
  /*  "TPred["  <tclass>|<tiscon>|<tunpred>| <tpred> "," <ttype> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP tclass; TEXP tiscon; TEXP typelist; TEXP unpred;
    TEXP w=0;
    if (strncmp("TPred[", c, 6)) return NULL;
    c=c+6;
    typelist=NULL;
    if (aig(tclass=parse_tclass(&c))) {
        plist[plen++]=MP_Expr;
	w=tclass;
    } else
    if (aig(tiscon=parse_tiscon(&c))) {
        plist[plen++]=MP_Expr;
	w=tiscon;
    } else
    if (aig(unpred=parse_tunpred(&c))) {
        plist[plen++]=MP_Expr;
	w=unpred;
    } else
    if (aig(w=parse_tpred(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(',',c);
	make_operator(TPREDAPP);
        plist[plen++]=MP_Op;
	if (!(typelist = parse_ttype(&c))) bail_out();
	plist[plen++]=MP_Expr;
    }
    skip_symbol(']',c);
    *t=c;
    return make_expression();
}

static TEXP parse_ttype(char **t)
{
  /* "TType[" <ttycon>|<ttype>|<toffset>|<tintcell>|<tap>|<tbadtype> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP result;
    if (strncmp("TType[", c, 6)) return NULL;
    c=c+6;
    if (aig(result=parse_ttycon(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_ttuple(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_toffset(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_tintcell(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_tap(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_tbadtype(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else return NULL;
}

static TEXP parse_ttycon(char **t)
{
  /* "TTYCON[\"" <string> "\"]" */
    char *c=*t;
    Char *plist;
    int plen=0;
    TEXP res;
    char *h;
    if (strncmp("TTYCON[\"", c,8)) return NULL;
    c=c+8;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    for (plen=0; plen<c-h; plen++) plist[plen]=h[plen];
    plist[c-h]=0;
    c=c+2;
    *t=c;
    res = make_identifier(0);
    free(plist);
    return res;
}

static TEXP parse_ttuple(char **t)
{
  /* "TTUPLE[" { "," } "]" */
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    TEXP res;
    int i;
    if (strncmp("TTUPLE[", c,7)) return NULL;
    c=c+7;
    i=1;
    while (*c==',') { i++;c++; }
    skip_symbol(']',c);
    *t=c;
    plist = malloc(sizeof(Char)*(i*2));
    plen=0;
    while (plen<i*2) {
      plist[plen++]=' ';
      plist[plen++]=',';
    }
    plen--;
    plist[plen]=0;
    res = make_identifier(0);
    free(plist);
    return res;
}

static TEXP parse_toffset(char **t)
{
  /* "TOFFSET[\"" <string> "\"]"  */
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    TEXP res;
    char *h;
    if (strncmp("TOFFSET[\"", c,9)) return NULL;
    c=c+9;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    while (plen<c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    c=c+2;
    *t=c;
    res = make_identifier(0);
    free(plist);
    return res;
}

static TEXP parse_tintcell(char **t)
{
  /* "TINTCELL[\"_" <integer> "\"]" */
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    TEXP res;
    char *h;
    if (strncmp("TINTCELL[\"_", c,11)) return NULL;
    c=c+11;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    while (plen<c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    c=c+2;
    *t=c;
    res = make_identifier(0);
    free(plist);
    return res;
}

static TEXP parse_tap(char **t)
{
  /* "TAP["  <ttypelist>|<ttypearrow>|<tistuple>|<taptype> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP res;
    int notnum;
    if (strncmp("TAP[",c,4)) return NULL;
    c=c+4;
    if (aig(res=parse_ttypelist(&c))) {
        plist[plen++]=PhNum2Char(MP_Expr,1);
	notnum=TLISTTYPE;
	skip_symbol(']',c);
    } else
    if (aig(res=parse_ttypearrow(&c))) {
        plist[plen++]=PhNum2Char(MP_Expr,1);
	notnum=TARROWTYPE;
	skip_symbol(']',c);
    } else
    if (aig(res=parse_tistuple(&c))) {
        plist[plen++]=PhNum2Char(MP_Expr,1);
	notnum=TTUPLETYPE;
	skip_symbol(']',c);
    } else
    if (aig(res=parse_taptype(&c))) {
        plist[plen++]=PhNum2Char(MP_Expr,1);
	notnum=TAPTOPTYPE;
	skip_symbol(']',c);
    } else return NULL;
    *t=c;
    return make_tnode(notnum);
}

static TEXP parse_tbadtype(char **t)
{
  /* "Tbadtype[]"  */
    char *c=*t;
    Char plist[10];
    int plen=0;
    if (strncmp("Tbadtype[]", c,10)) return NULL;
    c=c+10;
    *t=c;
    return make_tnode(TBADTYPE);
}

static TEXP parse_typelist(char **t)
{
  /*  [ <ttype> { "," <ttype> } ] */
    char *c=*t;
    Char plist[500];
    int plen=0;
    char resbuf[5000];
    TEXP pos;
    TEXP type;
    pos=resbuf;
    while (*c!=']') {
	type=parse_ttype(&c);
	if (type) {
	  plist[plen++]=MP_Expr;
	  if (*c==',') {
	    make_operator(TTYPESEP);
	    plist[plen++]=MP_Op;
	    c++;
	  }
	} else bail_out();
    }
    *t=c;
    if (!plen)
      return NULL;
    else
      return make_expression();
}

static TEXP parse_taptype(char **t)
{
  /* "TApType[" [ <taptype> "," ] <typelist> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP res1,res2;
    if (strncmp("TApType[", c,8)) return NULL;
    c=c+8;
    if (aig(res1=parse_taptype(&c))) {
      plist[plen++]=MP_Expr;
      skip_symbol(',',c);
      make_operator(TAPTYPE);
      plist[plen++]=MP_Op;
    }
    if (!(res2=parse_typelist(&c))) bail_out();
    plist[plen++]=MP_Expr;
    skip_symbol(']',c);
    *t=c;
    return make_expression();
}

static TEXP parse_ttypelist(char **t)
{
  /* "TtypeList[" <typelist> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP res;
    if (strncmp("TtypeList[", c, 10)) return NULL;
    c=c+10;
    if (aig(res=parse_typelist(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return res;
    } else return NULL;
}

static TEXP parse_ttypearrow(char **t)
{
  /* "TtypeArrow[" <ttype> "," [ <ttype> ] "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP tfor,tafter;
    if (strncmp("TtypeArrow[",c,11)) return NULL;
    c=c+11;
    if (!(tfor=parse_ttype(&c))) return NULL;
    plist[plen++]=MP_Expr;
    skip_symbol(',',c);
    make_operator(TARROWOP);
    plist[plen++]=MP_Op;
    if (aig(tafter=parse_ttype(&c))) plist[plen++]=MP_Expr;
    skip_symbol(']',c);
    *t=c;
    return make_expression();
}

static TEXP parse_tistuple(char **t)
{
  /* "TisTuple[" <typelist> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    TEXP res;
    if (strncmp("TisTuple[", c,9)) return NULL;
    c=c+9;
    if (!(res=parse_typelist(&c))) return NULL;
    plist[plen++]=MP_Expr;
    skip_symbol(']',c);
    *t=c;
    return res;
}

static TEXP parse_tclass(char **t)
{
  /* "TClass[\"" <string> "\"]" */
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    TEXP res;
    char *h;
    if (strncmp("TClass[\"", c,8)) return NULL;
    c=c+8;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    while (plen < c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    res = make_identifier(0);
    *t=c+2;
    free(plist);
    return res;
}

static TEXP parse_tiscon(char **t)
{
  /* "TCon[\"" <string> "\"]" */
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    TEXP res;
    char *h;
    if (strncmp("TCon[\"", c,6)) return NULL;
    c=c+6;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    while (plen < c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    res = make_identifier(0);
    *t=c+2;
    free(plist);
    return res;
}

static TEXP parse_tunpred(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    if (strncmp("TunknownPredicate[]", c,19)) return NULL;
    c=c+19;
    *t=c;
    return make_tnode(TUNPRED);
}

static TEXP parse_texp(char **t)
{
    TEXP res;
    if (!(res=parse_tsigtype(t))) {
	parse_error_func(*t);
	return NULL;
    } else
	return res;
}

static EXP parse_exp(char **t)
{
    char *c=parse_dummy(t);
    Char plist[10];
    int plen=0;
    if (strncmp("Exp",c,3)) {
	parse_error_func(c);
	return NULL;;
    }
    plist[plen++]='E';
    plist[plen++]='x';
    plist[plen++]='p';
    return make_identifier(0);
}

static STRING parse_string(char **t)
{
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    char *h;
    STRING res;
    if (strncmp("String[\"",c,8)) return NULL;
    c=c+8;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist = malloc(sizeof(Char)*(c-h+1));
    while (plen < c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    res = make_text();
    *t=c+2;
    free(plist);
    return res;
}

static CONT parse_cont(char **t)
{
    char *c=*t;
    CONT res;
    if (!(res=parse_tcontext(t))) {
	parse_error_func(c);
	return NULL;
    } else
	return res;
}

static CHAR parse_char(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    char *h;
    if (strncmp("Char[\"",c,6)) return NULL;
    c=c+6;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    while (plen<c-h && plen<10) {
      plist[plen]=h[plen];
      plen++;
    }
    *t=c+2;
    return make_identifier(0);
}

static PRED parse_pred(char **t)
{
    char *c=*t;
    PRED res;
    if (!(res=parse_tpred(t))) {
	parse_error_func(c);
	return NULL;
    } else
	return res;
}

static NAT parse_nat(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    char *h;
    if (strncmp("Nat[\"",c,5)) return NULL;
    c=c+5;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    while (plen<c-h && plen<10) {
      plist[plen]=h[plen];
      plen++;
    }
    *t=c+2;
    return make_identifier(0);
}

static KIND parse_kbadkind(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    if (strncmp("Kbadkind[]", c,10)) return NULL;
    c=c+10;
    *t=c;
    return make_tnode(TBADKIND);
}

static KIND parse_kstar(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    if (strncmp("KSTAR[]", c,7)) return NULL;
    c=c+7;
    *t=c;
    return make_tnode(TSTARKIND);
}

static KIND parse_koffset(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    char *h;
    if (strncmp("KOFFSET[\"",c,9)) return NULL;
    c=c+9;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    while (plen<c-h && plen<10) {
      plist[plen]=h[plen];
      plen++;
    }
    *t=c+2;
    return make_identifier(0);
}

static KIND parse_kintcell(char **t)
{
    char *c=*t;
    Char *plist=NULL;
    int plen=0;
    char *h;
    KIND res;
    if (strncmp("KINTCELL\"",c,11)) return NULL;
    c=c+11;
    h=c;
    c=strstr(c,"\"]");
    if (!c) return NULL;
    plist= malloc(sizeof(Char)*(c-h+1));
    while (plen<c-h) {
      plist[plen]=h[plen];
      plen++;
    }
    *t=c+2;
    res = make_identifier(0);
    free(plist);
    return res;
}

static KIND parse_kind(char **t);
static KIND parse_kap(char **t)
{
    char *c=*t;
    Char plist[10];
    int plen=0;
    KIND k1,k2;
    if (strncmp("KAP[",c,4)) return NULL;
    c=c+4;
    if (!(k1=parse_kind(&c))) return NULL;
    plist[plen++]=MP_Expr;
    skip_symbol(',',c);
    make_operator(TKINDARROW);
    plist[plen++]=MP_Op;
    if (!(k2=parse_kind(&c))) bail_out();
    plist[plen++]=MP_Expr;
    skip_symbol(']',c);
    *t=c;
    return make_expression();
}

static KIND parse_kind(char **t)
{
  /* "Kind[" <kap> | <kstar> | <koffset> | <kintcell> | <kbadkind> "]" */
    char *c=*t;
    Char plist[10];
    int plen=0;
    KIND result;
    if (strncmp("Kind[", c, 5)) return NULL;
    c=c+5;
    if (aig(result=parse_kap(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_kstar(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_koffset(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_kintcell(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else
    if (aig(result=parse_kbadkind(&c))) {
        plist[plen++]=MP_Expr;
	skip_symbol(']',c);
	*t=c;
	return result;
    } else return NULL;
}

char buffer[5000];
char *parse_error(char **t)
{
  EXP exp1,exp2,exp3,exp4;
  TEXP texp1,texp2,texp3,texp4,texp5,texp6;
  STRING string1,string2,string3,string6,string7;
  CONT cont3,cont4;
  CHAR char1,char2;
  PRED pred2,pred3,pred4,pred5;
  NAT nat1,nat2,nat3;
  KIND kind4,kind5,kind6;
  Char plist[5000];
  int plen=0;
  char *w=*t;
  char *s=*t;
  switch (s[0]) {
  case '1':
	w++;
    switch (s[1]) {
    case '1':
	w++;
      switch (s[2]) {
      case '1':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Multiple project filenames on command line");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '2':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  char1=parse_char(&w);
	  plen=chartoChar(plist,"Unknown toggle `%e'");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Cannot change heap size");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Missing integer in option setting \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Option setting \"%e\" is too large");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Option setting \"%e\" is too large");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Unwanted characters after option setting \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Option string must begin with `+' or `-'");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '3':
	w++;
	skip_symbol('[',w);
	string1=parse_string(&w);
	plen=chartoChar(plist,"Unable to change to directory \"%e\"");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '4':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Empty project file");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"Too many script files (maximum of %e allowed)");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"Recursive import dependency between \"%e\" and \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Too many project files");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"No project filename specified");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '5':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Multiple filenames not permitted");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"No name specified");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Multiple names not permitted");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	  w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"No current definition for name \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '6':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  texp1=parse_texp(&w);
	  skip_symbol(',',w);
	  exp2=parse_exp(&w);
	  plen=chartoChar(plist,"Unresolved overloading\n*** type : %e\n*** expression : %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  exp1=parse_exp(&w);
	  skip_symbol(',',w);
	  texp2=parse_texp(&w);
	  plen=chartoChar(plist,"Cannot find \"show\" function for:\n"
		  "*** expression : %e\n"
		  "*** of type    : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
        }
	break;
      case '7':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"No names selected");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '8':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"String storage space exhausted");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '9':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Hugs is not configured to use an editor");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      }
      break;
    case '2':
      w++;
      switch (s[2]) {
      case '1':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Unable to open project file \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Unable to open file \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Program line next to comment");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  char2=parse_char(&w);
	  plen=chartoChar(plist,"%e: Empty script - perhaps you forgot the `%e's?");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '2':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);	  
	  plen=chartoChar(plist,"Maximum token length (%e) exceeded");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);	  
	  plen=chartoChar(plist,"%e: Integer literal out of range");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Integer literal out of range");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);	  
	  plen=chartoChar(plist,"%e: Missing digits in exponent");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: No floating point numbers in this implementation");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal character constant");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Improperly terminated character constant");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Improperly terminated string");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  nat2=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Non printable character `\\%e' in constant");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '3':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal use of `\\&' in character constant");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal escape sequence");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal use of gap in character constant");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Illegal character escape sequence \"\\%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Missing `\\' terminating string literal gap");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  char2=parse_char(&w);
	  plen=chartoChar(plist,"%e: Unrecognised escape sequence `\\^%e'");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Empty octal character escape");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Octal character escape out of range");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Empty hexadecimal character escape");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'A':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Hexadecimal character escape out of range");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'B':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Decimal character escape out of range");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '4':
	w++;
	skip_symbol('[',w);
	  nat1=parse_nat(&w);
	plen=chartoChar(plist,"%e: a closing quote, '\"', was expected");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '5':
	w++;
	skip_symbol('[',w);
	  nat1=parse_nat(&w);
	plen=chartoChar(plist,"%e: Too many levels of program nesting");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '6':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Unterminated nested comment {- ...");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Misplaced `}'");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  nat2=parse_nat(&w);
	  skip_symbol(',',w);
	  nat3=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Unrecognised character `\\%e' in column %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Cannot use %e without any previous input");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '7':
	w++;
	skip_symbol('[',w);
	nat1=parse_nat(&w);
	plen=chartoChar(plist,"%e: Parser overflow");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '8':
	w++;
	switch(s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Syntax error in %e (unexpected %e)");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  nat2=parse_nat(&w);
	  skip_symbol(',',w);
	  nat3=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Precedence value must be an integer in the range [%e..%e]");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Class \"%e\" must have exactly one argument");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Argument of class \"%e\" must be a variable");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Class \"%e\" must have exactly one argument");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Type variable expected in instance type");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal type expression in instance declaration");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Last generator in do {...} must be an expression");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal left hand side in type definition");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'A':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Ambiguous use of operator \"%e\" with \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
      }
      break;
    case '3':
	w++;
      switch (s[2]) {
      case '1':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated definition of type constructor \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: \"%e\" used as both class and type constructor");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated type variable \"%e\" on left hand side");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Recursive type synonym \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated definition for constructor function \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated field name \"%e\" for constructor \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated definition for selector \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Undefined type constructor \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Undefined type variable \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'A':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Type synonyms \"%e\" and \"%e\" are mutually recursive");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'B':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  exp2=parse_exp(&w);
	  skip_symbol(',',w);
	  cont3=parse_cont(&w);
	  skip_symbol(',',w);
	  pred4=parse_pred(&w);
	  plen=chartoChar(plist,"%e: Illegal datatype strictness annotation:\n"
		         "*** Constructor : %e\n"
		         "*** Context     : %e\n"
	                 "*** Required    : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '2':
	w++;
	skip_symbol('[',w);
	nat1=parse_nat(&w);
	skip_symbol(',',w);
	string2=parse_string(&w);
	skip_symbol(',',w);
	texp3=parse_texp(&w);
	skip_symbol(',',w);
	exp4=parse_exp(&w);
	plen=chartoChar(plist,"%e: Ambiguous type signature in %e\n"
	               "*** ambiguous type : %e\n"
		       "*** assigned to    : %e");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '3':
	w++;
	skip_symbol('[',w);
	nat1=parse_nat(&w);
	skip_symbol(',',w);
	string2=parse_string(&w);
	plen=chartoChar(plist,"%e: Too many type variables in %e\n");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '4':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated definition of class \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: \"%e\" used as both class and type constructor");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Undefined class \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Illegal constraints on class variable \"%e\"\n"
	                 " in type of member function \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated definition for member function \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Class hierarchy for \"%e\" is not acyclic");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '5':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated type variable \"%e\" in instance predicate");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Instances of class \"%e\" are generated automatically");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Simple type required in instance declaration");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Type synonym \"%e\" not permitted in instance of \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  pred2=parse_pred(&w);
	  plen=chartoChar(plist,"%e: Repeated instance declaration for %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Type signature decls not permitted in instance decl");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '6':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  pred2=parse_pred(&w);
	  skip_symbol(',',w);
	  pred3=parse_pred(&w);
	  plen=chartoChar(plist,"%e: Definition of %e requires superclass instance %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  pred2=parse_pred(&w);
	  skip_symbol(',',w);
	  pred3=parse_pred(&w);
	  skip_symbol(',',w);
	  cont4=parse_cont(&w);
	  skip_symbol(',',w);
	  pred5=parse_pred(&w);
	  plen=chartoChar(plist,"%e: Cannot build superclass instance %e of %e\n"
	                 "*** Context  : %e\n"
	                 "*** Required : %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '7':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Pattern binding illegal in %e declaration");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: No member \"%e\" in class \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '8':
	w++;
	switch (s[3]) {
	case '1':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Unknown class \"%e\" in derived instance");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Duplicate derived instance for class \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  pred2=parse_pred(&w);
	  skip_symbol(',',w);
	  pred3=parse_pred(&w);
	  plen=chartoChar(plist,"%e: An instance of %e is required to derive %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Cannot derive instances of class \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Can only derive instances of Enum for enumeration types");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Can only derive instances of Ix for enumeration or product types");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Can only derive instances of Bounded for enumeration and product types");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '9':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Redeclaration of primitive \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Multiple default declarations are not permitted in\n"
		         "a single script file.");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Default types must be instances of the Num class");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case 'A':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal pattern syntax");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Second argument in (n+k) pattern must be an integer");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Integer k in (n+k) pattern must be > 0");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal tuple pattern");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal pattern syntax");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated variable \"%e\" in pattern");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Undefined constructor function \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: \"%e\" is not a constructor function");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  nat3=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Constructor function \"%e\" needs %e args in pattern");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case 'B':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Equations give different arities for \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: No variables defined in lhs pattern");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Improper left hand side");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: \"%e\" multiply defined");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Type declaration for variable \"%e\" with no body");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated type declaration for \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case 'C':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal `@' in expression");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal `~' in expression");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Illegal `_' in expression");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Undefined variable \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  plen=chartoChar(plist,"%e: Constructor \"%e\" does not have selected fields in %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  exp2=parse_exp(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  plen=chartoChar(plist,"%e: Construction does not define strict field\n"
	                 "Expression : %e\n"
	                 "Field      : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Empty field list in update");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: \"%e\" is not a selector function/field name");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Repeated field name \"%e\" in field list");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'A':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  exp2=parse_exp(&w);
	  plen=chartoChar(plist,"%e: No constructor has all of the fields specified in %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case 'D':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: No top level definition for operator symbol \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Attempt to redefine variable \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: No top level binding of \"%e\" for restricted synonym \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      }
      break;
    case '4':
	w++;
      switch (s[2]) {
      case '1':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  skip_symbol(',',w);
	  exp4=parse_exp(&w);
	  skip_symbol(',',w);
	  texp5=parse_texp(&w);
	  skip_symbol(',',w);
	  texp6=parse_texp(&w);
	  plen=chartoChar(plist,"%e: Type error in %e\n"
		         "*** expression     : %e\n"
	                 "*** term           : %e\n"
	                 "*** type           : %e\n"
	                 "*** does not match : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  skip_symbol(',',w);
	  texp5=parse_texp(&w);
	  plen=chartoChar(plist,"%e: Type error in %e\n"
	                 "*** term           : %e\n"
	                 "*** type           : %e\n"
	                 "*** does not match : %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  skip_symbol(',',w);
	  exp4=parse_exp(&w);
	  skip_symbol(',',w);
	  texp5=parse_texp(&w);
	  skip_symbol(',',w);
	  texp6=parse_texp(&w);
	  skip_symbol(',',w);
	  string7=parse_string(&w);
	  plen=chartoChar(plist,"%e: Type error in %e\n"
	                 "*** expression     : %e\n"
	                 "*** term           : %e\n"
	                 "*** type           : %e\n"
	                 "*** does not match : %e\n"
	                 "*** because        : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  exp3=parse_exp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  skip_symbol(',',w);
	  texp5=parse_texp(&w);
	  skip_symbol(',',w);
	  string6=parse_string(&w);
	  plen=chartoChar(plist,"%e: Type error in %e\n"
	                 "*** term           : %e\n"
	                 "*** type           : %e\n"
	                 "*** does not match : %e\n"
	                 "*** because        : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  cont4=parse_cont(&w);
	  plen=chartoChar(plist,"%e: Unresolved top-level overloading\n"
	                 "*** Binding             : %e\n"
	                 "*** Inferred type       : %e\n"
		         "*** Outstanding context : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  cont3=parse_cont(&w);
	  plen=chartoChar(plist,"%e: Unresolved top-level overloading\n"
	                 "*** Binding             : %e\n"
	                 "*** Outstanding context : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Explicit overloaded type for \"%e\"\n"
	                 " not permitted in restricted binding");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  cont3=parse_cont(&w);
	  skip_symbol(',',w);
	  pred4=parse_pred(&w);
	  plen=chartoChar(plist,"%e: Insufficient class constraints in %e\n"
	                 "*** Context  : %e\n"
	                 "*** Required : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '9':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  exp2=parse_exp(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  plen=chartoChar(plist,"%e: Declared type too general\n"
	                 "*** Expression    : %e\n"
	                 "*** Declared type : %e\n"
	                 "*** Inferred type : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'A':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  plen=chartoChar(plist,"%e: Mismatch in field types for selector \"%e\"\n"
	                 "*** Field type     : %e\n"
	                 "*** Does not match : %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'B':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define standard types");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'C':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define standard classes");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'D':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define numeric classes");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'E':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define monad classes");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'F':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define IO monad constructor");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'G':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define standard constructors");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case 'H':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Prelude does not define standard members");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '2':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Too many type variables in type checker");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Substitution expanding too quickly");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"Too many variables (%e) in type checker");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Too many quantified type variables");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '3':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  texp2=parse_texp(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Illegal type \"%e\" in %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Illegal type in %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  skip_symbol(',',w);
	  kind5=parse_kind(&w);
	  skip_symbol(',',w);
	  kind6=parse_kind(&w);
	  plen=chartoChar(plist,"%e: Kind error in %e\n"
	                 "*** expression     : %e\n"
	                 "*** constructor    : %e\n"
	                 "*** kind           : %e\n"
	                 "*** does not match : %e\n");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  kind4=parse_kind(&w);
	  skip_symbol(',',w);
	  kind5=parse_kind(&w);
	  plen=chartoChar(plist,"%e: Kind error in %e\n"
	                 "*** constructor    : %e\n"
	                 "*** kind           : %e\n"
 	                 "*** does not match : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  texp4=parse_texp(&w);
	  skip_symbol(',',w);
	  kind5=parse_kind(&w);
	  skip_symbol(',',w);
	  kind6=parse_kind(&w);
	  skip_symbol(',',w);
	  string7=parse_string(&w);
	  plen=chartoChar(plist,"%e: Kind error in %e\n"
	                 "*** expression     : %e\n"
	                 "*** constructor    : %e\n"
	                 "*** kind           : %e\n"
	                 "*** does not match : %e\n"
	                 "*** because        : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  texp3=parse_texp(&w);
	  skip_symbol(',',w);
	  kind4=parse_kind(&w);
	  skip_symbol(',',w);
	  kind5=parse_kind(&w);
	  skip_symbol(',',w);
	  string6=parse_string(&w);
	  plen=chartoChar(plist,"%e: Kind error in %e\n"
	                 "*** constructor    : %e\n"
	                 "*** kind           : %e\n"
	                 "*** does not match : %e\n"
	                 "*** because        : %e");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '7':
	  w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Not enough arguments for type synonym \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '8':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  skip_symbol(',',w);
	  string3=parse_string(&w);
	  plen=chartoChar(plist,"%e: Kind of class \"%e\" does not match superclass \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '4':
	w++;
	skip_symbol('[',w);
	nat1=parse_nat(&w);
	skip_symbol(',',w);
	texp2=parse_texp(&w);
	skip_symbol(',',w);
	string3=parse_string(&w);
	plen=chartoChar(plist,"%e: %e is not an instance of class \"%e\"");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      }
      break;
    case '5':
	w++;
      switch (s[2]) {
      case '1':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Compiled code too complex");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '2':
	w++;
	skip_symbol('[',w);
	nat1=parse_nat(&w);
	skip_symbol(',',w);
	string2=parse_string(&w);
	plen=chartoChar(plist,"%e: Unknown primitive reference \"%e\"");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      }
      break;
    case '6':
	w++;
      switch (s[2]) {
      case '1':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Character string storage space exhausted");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '2':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  skip_symbol(',',w);
	  string2=parse_string(&w);
	  plen=chartoChar(plist,"%e: Attempt to redefine syntax of operator \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"%e: Too many fixity declarations");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '3':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Program code storage space exhausted");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '4':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Type constructor storage space exhausted");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '5':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Name storage space exhausted");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '6':
	w++;
	skip_symbol('[',w);
	string1=parse_string(&w);
	plen=chartoChar(plist,"%e in pattern");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '7':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Class storage space exhausted");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Instance storage space exhausted");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case '8':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Control stack overflow");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case '9':
	w++;
	skip_symbol('[',w);
	plen=chartoChar(plist,"Too many script/module files in use");
	make_text(); buffer[0]='#';buffer[1]='\0';
	break;
      case 'A':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Flat resource space exhausted");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Garbage collection fails to reclaim sufficient space");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  string1=parse_string(&w);
	  plen=chartoChar(plist,"Cannot open profile log file \"%e\"");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      case 'B':
	w++;
	skip_symbol('[',w);
        string1=parse_string(&w);
	plen=chartoChar(plist,"Too many handles open; cannot open \"%e\"");
	break;
      case 'C':
	w++;
	switch (s[3]) {
	case '1':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Cannot allocate run-time tables");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '2':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"Cannot allocate heap storage (%e cells)");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '3':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Cannot allocate profiler storage space");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '4':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Unable to allocate gc markspace");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '5':
	w++;
	  skip_symbol('[',w);
	  nat1=parse_nat(&w);
	  plen=chartoChar(plist,"Cannot allocate flat space (%e cells)");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	case '6':
	w++;
	  skip_symbol('[',w);
	  plen=chartoChar(plist,"Cannot allocate instance tables");
	  make_text(); buffer[0]='#';buffer[1]='\0';
	  break;
	}
	break;
      }
    case '8':
      w++;
	skip_symbol('[',w);
	skip_symbol('"',w);
	if (aig(string1=strstr(w,"\"]"))) {
	    *((char*)string1)='\0';
	    sprintf(buffer,"?In file %s", w);
	    *((char*)string1)='"';
	    w=((char*)string1)+1;
	}
	break;
    }
  }
  if (*w==']') w++; else parse_error_func(w);
  *t=w;
  return buffer;
}

void hugsparse_init(void)
{
  int i=0;
  while (tmplate[i].uidnumber) {
    tmplate[i].iid = notation_with_number(tmplate[i].uidnumber);
    i++;
  }
}
