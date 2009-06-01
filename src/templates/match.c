/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>

#include "mathpad.h"
#include "match.h"
#include "leaftree.h"
#include "intstack.h"
#include "memman.h"

#ifdef MESSAGE
extern int fprintf(FILE *f, char *format, ...);
#define message(A) fprintf(stderr, A)
#define message2(A,B) fprintf(stderr, A, B)
#else
#define message(A) /* */
#define message2(A,B) /* */
#endif

#define set_free_leaf(A) 
#define set_leafsize(A)

/*
** simple version of lex (only fixed strings).
**
** Functions:
**
** int lex_add_string(char* item, int retval)
**
**     Add item to the match tree. retval is the candidate for the return
**     value when the item is matched during a lexical scan, but:
**     1   if item is already available in the match tree, the return value
**         for that item will be used.
**     2   if retval is 0, a new return value is generated.
**     3   otherwise, retval is used as return value.
**     The function return the value that will be generated when the item
**     is matched.
**
** int lex_start(char *text)
**
**     start to match text.
**
** int lex_next()
**
**     returns the coding of the next item that is matched.
**
** int lex_alter()
**
**     returns an alternative match for the last found match.
**
*/

#define SPACEVAL 1

static int space_insens=0;
static int case_insens=0;

static int lex_start(char *text);
static int lex_next(void);
static int lex_alter(void);

static unsigned char *mtext=NULL;
static int lex_pos;
static int *stack=NULL;
static int stacksize=0;
static int stackpos=0;
static INTSTACK *lex_state_stack=NULL;
static PARSERULES current_rules = {{NULL},NULL,0,0x8000 };

#define MT int
typedef struct {
    char kind;
    MT val;
    int retval;
    LeafTree *lxtree;
} Leaf;

/* space insensitive makes the stack a little difficult */

static void make_stack(void)
{
  if (!stacksize || stacksize < current_rules.height+1) {
    int *c;
    if (!stack) 
	c = (int*) malloc((current_rules.height+200)*5*sizeof(int));
    else
	c = (int*) realloc(stack, (current_rules.height+200)*5*sizeof(int));
    if (c) {
      stacksize=current_rules.height+1;
      stack = c;
    }
  }
}

static void free_lexleaf(void *ptr)
{
    Leaf *lptr = (Leaf*) ptr;
    LT_free(lptr->lxtree);
}

static void lex_print_tree(LeafTree *ptr, char *b, int pos)
{
    char c;
    LeafTree *h;
    Leaf *l;
    for (c=1; c<127; c++) {
	if (member(ptr, c)) {
	    l= (Leaf*) LT_found_leaf();
	    h=l->lxtree;
	    b[pos]=c;
	    if (l->retval) {
		b[pos+1]=0;
		fprintf(stderr, "%i:\t>%s<\n", l->retval, b);
	    }
	    if (h) lex_print_tree(h,b,pos+1);
	}
    }
}

static void lex_print_state(void)
{
    int i;
    message("Lex-state\n");
    message2("Pos: %i", lex_pos);
    message2("\tStpos: %i\nStack:", stackpos);
    for (i=0;i<=stackpos; i++)
	message2("%i,", stack[i]);
    message("\nEnd-Lex-state\n");
}

static void lex_push_state()
{
    int i,j=0;
    for (i=0; i<=stackpos; i++)
	if (stack[i]) {
	    j++;
	    push_int(&lex_state_stack, (i<<16)+stack[i]);
	}
    push_int(&lex_state_stack, (j<<16) + stackpos);
    push_int(&lex_state_stack, lex_pos);
}

static void lex_pop_state_skip()
{
    int i;
    pop_int(&lex_state_stack);
    i=pop_int(&lex_state_stack);
    i=i>>16;
    while (i) {
	pop_int(&lex_state_stack);
	i--;
    }
}

static void lex_pop_state_use()
{
    int i,j;
    lex_pos=pop_int(&lex_state_stack);
    stackpos=pop_int(&lex_state_stack);
    i=stackpos>>16;
    stackpos=stackpos&0xffff;
    for (j=stackpos; j>=0; j--)
	stack[j]=0;
    while (i) {
	j=pop_int(&lex_state_stack);
	stack[j>>16]=j&0xffff;
	i--;
    }
}

int lex_add_string(char *item, int retval)
{
    int i;
    int cv;
    char c;
    char *h, *g;
    int spacepre=0,spacepost=0,spacenr=0,spaceempty=0;
    Leaf *lf=NULL;
    LeafTree **lt;

    if (!item || !*item) return 0;
    if (space_insens) {
	h=g=item;
	i=0;
	spacepre=isspace(*h);
	while (*h) {
	    if (isspace(*h)) {
		while (isspace(h[1])) { h++; spacenr++; }
		*h=SPACEVAL;
	    }
	    spacepost=(i>0 && (*h==SPACEVAL));
	    *g++=*h++;
	    i++;
	}
	if (spacepost) { g--;i--; }
	if (i==1 && spacepre) {
	    spaceempty=1;
	    spacepre=0;
	    g--;
	}
	*g='\0';
    }
    set_leafsize(sizeof(Leaf));
    lt = (LeafTree**) &current_rules.lextree;
    h=item;
    i=0;
    while (*h) {
	c=*h;
	h++;i++;
	if (case_insens) c=toupper(c);
	cv=c;
	LT_insert(*lt, (void*)cv);
	lf = (Leaf*) LT_found_leaf();
	lt = &lf->lxtree;
    }
    if (i+1+spacenr >current_rules.height) current_rules.height=i+1+spacenr;
    if (i) {
	if (lf->retval) retval=lf->retval;
	if (!retval) retval = current_rules.nextretval++;
	lf->retval = retval;
    } else
	retval=0;
    if (space_insens && !spaceempty) {
	if (spacepost) {
	    LT_insert(*lt, (void*)SPACEVAL);
	    lf = (Leaf*) LT_found_leaf();
	    if (!lf->retval) lf->retval=retval;
	}
	if (spacepre) {
	    lt=(LeafTree**) &current_rules.lextree;
	    h=item+1;
	    i=0;
	    while (*h) {
		c=*h;
		h++;i++;
		if (case_insens) c=toupper(c);
		cv=c;
		LT_insert(*lt,(void*)cv);
		lf = (Leaf*) LT_found_leaf();
		lt = &lf->lxtree;
	    }
	    if (i && !lf->retval) lf->retval=retval;
	    if (spacepost ) {
		LT_insert(*lt, (void*)SPACEVAL);
		lf = (Leaf*) LT_found_leaf();
		if (!lf->retval) lf->retval=retval;
	    }
	}
    }
    return retval;
}

static int lex_start(char *text)
{
    mtext = (unsigned char*) text;
    make_stack();
    stackpos=0;
    lex_pos=0;
    return 0;
}

int lex_value(char *item)
{
    char c;
    LeafTree* h;
    Leaf *l=NULL;

    h=current_rules.lextree;
    if (!item || !item[0]) return 0;
    while (*item) {
	c=*item;
	if (space_insens && isspace(c)) {
	    while (isspace(item[1])) item++;
	    c=SPACEVAL;
	} else if (case_insens) c=toupper(c);
	if (member(h, c)) {
	    l = (Leaf*) LT_found_leaf();
	    h = l->lxtree;
	    item++;
	} else return 0;
    }
    return l->retval;
}

static int lex_next(void)
{
    LeafTree* h;
    Leaf *l;
    char c;
    if (!mtext || !mtext[lex_pos]) return 0;
    h=current_rules.lextree;
    stackpos=0;
    stack[0]=mtext[lex_pos];
    while (mtext[lex_pos]) {
	c=mtext[lex_pos];
	if (member(h, c)) {
	    l = (Leaf*) LT_found_leaf();
	    h = l->lxtree;
	    stack[++stackpos]=l->retval;
	    lex_pos++;
	} else if (space_insens && isspace(c) && member(h,SPACEVAL)) {
	    l = (Leaf*) LT_found_leaf();
	    h = l->lxtree;
	    while (isspace(mtext[lex_pos])) {
		stack[++stackpos]=0;
		lex_pos++;
	    }
	    stack[stackpos]=l->retval;
	} else if (case_insens && member(h,toupper(c))) {
	    l = (Leaf*) LT_found_leaf();
	    h = l->lxtree;
	    stack[++stackpos]=l->retval;
	    lex_pos++;
	} else if (space_insens && member(h,SPACEVAL)) {
	    l= (Leaf*) LT_found_leaf();
	    h= l->lxtree;
	} else break;
    }
    while (!stack[stackpos]) { stackpos--; lex_pos--; }
    if (!stackpos) lex_pos++;
    return (stack[stackpos]);
}

static int lex_alter(void)
{
  if (!stackpos) return 0;
  do {
    stackpos--; lex_pos--;
  } while (!stack[stackpos]);
  if (!stackpos) lex_pos++;
  return (stack[stackpos]);
}

/*
** simple version of a parser.
** * recursive descent (with closeset)
** * backtracking (to solve ambiguaty)
** (not that simple)
**
** int parse_text(char *text)
**
**     parse text and convert it to a mathspad document
**
** int parse_use_rules(PARSERULES *set)
**
**     use set as parse rules
**
** PARSERULES *parse_save_rules()
**
**     save the current parse rules
**
** int parse_add_rule(int group, Char *parsestring, int template, int prec)
**
**     add a parse rule to a group (TEXT,EXPR,INFIX,POSTFIX,PREFIX)
**     parsestring contains the items that the scanner returns
**
*/

#define MAXIDLEN 16

typedef struct RULE RULE;

struct RULE {
    Char *text;
    int len;
    int rtype;
    int tpnr;
    RULE *next;
};

typedef struct PARRULE PARRULE;

struct PARRULE {
    RULE *rule;
    int pos;
};

typedef struct ParseLeaf ParseLeaf;

struct ParseLeaf {
    char kind;
    MT val;
    RULE *rlist;
    int nr;
};

static Char nextsym=0;

#define New(A)         ((A *) malloc(sizeof(A)))
#define NewArray(A,B)  ((A *) malloc((B)*sizeof(A)))

int parse_add_rule(int group, Char *parsestring, int len, int template,
		   int prec)
{
    ParseLeaf *plf;
    RULE *rlist;
    int i;
    if (!parsestring[0]) return 0;
    /*
    ** Two place holders next to eachother are not allowed.
    ** Is this reasonable ???
    */
    for (i=1; parsestring[i] && i<len; i++)
	if (IsPh(parsestring[i]) && IsPh(parsestring[i-1])) return 0;
    set_leafsize(sizeof(ParseLeaf));
    if (group==TEXT) {
	i=0;
	while (i<len && Ph(parsestring[i])==MP_Text) i++;
	if (i==len || IsPh(parsestring[i])) return 0;
	if (i) {
	    int j=0;
	    while (aig(parsestring[j]=parsestring[j+i])) j++;
	}
	i=parsestring[0];
	LT_insert(current_rules.optree[TEXTTREE], (void*)i);
    } else if (group==IDFORBID) {
      for (i=0; i<len; i++) {
	    int cv=parsestring[i];
	    LT_insert(current_rules.optree[IDFORBIDTREE],(void*)cv);
      }
      return 0;
    } else if (group==IDOPEN) {
	for (i=0; i<len; i++) {
	    int cv = parsestring[i];
	    LT_insert(current_rules.optree[IDOPENTREE], (void*)cv);
	    plf = (ParseLeaf*) LT_found_leaf();
	    if (plf) {
		rlist = New(RULE);
		rlist->text = NULL;
		rlist->len = 0;
		rlist->rtype = group;
		rlist->tpnr = template;
		rlist->next = plf->rlist;
		plf->rlist = rlist;
		plf->nr++;
	    }
	}
	return 0;
    } else if (group==IDCLOSE) {
	for (i=0; i<len; i++) {
	    int cv = parsestring[i];
	    LT_insert(current_rules.optree[IDCLOSETREE], (void*)cv);
	    plf = (ParseLeaf*) LT_found_leaf();
	    if (plf) {
		rlist = New(RULE);
		rlist->text = NULL;
		rlist->len = 0;
		rlist->rtype = group;
		rlist->tpnr = template;
		rlist->next = plf->rlist;
		plf->rlist = rlist;
		plf->nr++;
	    }
	}
	return 0;
    } else {
	i=len-1;
	while (i>=0 && Ph(parsestring[i])==MP_Expr) i--;
	if (i<0 || (IsPh(parsestring[i]) && Ph(parsestring[i])!=MP_Id)) return 0;
	if (i!=len-1) {
	    group = group|PREFIX;
	    parsestring[i+1]=0;
	}
	i=0;
	while (i<len && Ph(parsestring[i])==MP_Expr) i++;
	if (i==len || IsPh(parsestring[i])) return 0;
	if (i>0) {
	    int j=0;
	    group = group|POSTFIX;
	    while (aig(parsestring[j]=parsestring[j+i])) j++;
	}
	i=parsestring[0];
	LT_insert(current_rules.optree[ALLOPTREE],(void*)i);
	if (group & INFIX)
	    LT_insert(current_rules.optree[prec], (void*)i);
	else
	    LT_insert(current_rules.optree[EXPRTREE], (void*)i);
    }
    plf = (ParseLeaf*) LT_found_leaf();
    rlist = New(RULE);
    rlist->text = parsestring;
    rlist->len = len;
    rlist->rtype = group;
    rlist->tpnr = template;
    rlist->next = plf->rlist;
    plf->rlist = rlist;
    plf->nr++;
    return 1;
}


extern void *make_node(Char type, Char *txt, int len, int nnr, int spacing);
extern void join_parse_stack(void);
extern void *add_parse_stack(Char *txt, int len);

static int parse_microspace(Char *buffer __attribute__((unused)));
static void *parse_node_identifier(Char *buffer, LeafTree *closeset);
static void *parse_node_variable(Char *buffer, LeafTree *closeset);
static void *parse_node_expression(Char *buffer, LeafTree *closeset, int prec);
static void *parse_node_template(Char *buffer, LeafTree *closeset,
				 LeafTree *rules, int *rtype);
static void *parse_node_operator(Char *buffer, LeafTree *closeset,
				 int prec, int *optype);
static void *parse_node_text(Char *buffer, LeafTree *closeset);

static int parse_microspace(Char *buffer __attribute__((unused)))
{
    Char oldsym=nextsym;
    int value,i;
    Char msval1, msval2;

    lex_push_state();
    msval1 = lex_value("\\ms{");
    msval2 = lex_value("\\,");
    while (nextsym && nextsym != msval1 && nextsym != msval2)
	nextsym=lex_alter();
    if (nextsym) {
	if (nextsym==msval1) {
	    value=0;i=lex_pos;
	    while (isdigit(mtext[i]))
		value=value*10+mtext[i++]-'0';
	    if (mtext[i]=='}') {
		lex_pos=i+1;
		nextsym=lex_next();
		lex_pop_state_skip();
		return value;
	    }
	} else if (nextsym == msval2) {
	    value=1;
	    i=lex_pos;
	    while (mtext[i]=='\\' && mtext[i+1]==',') {
		value++;
		i=i+2;
	    }
	    lex_pos=i;
	    nextsym=lex_next();
	    lex_pop_state_skip();
	    return value;
	}
    }
    nextsym=oldsym;
    lex_pop_state_use();
    return 0;
}

static void *parse_node_identifier(Char *buffer, LeafTree *closeset)
{
    Char oldsym=nextsym;
    int idop=0,i=0;
    int idfont=0;
    LeafTree *cs=NULL;
    ParseLeaf *plf;

    lex_push_state();
    message("(ID:");
    while (nextsym && !member(current_rules.optree[IDOPENTREE], nextsym))
	nextsym = lex_alter();
    if (nextsym) {
	idop=1;
	plf = (ParseLeaf*) LT_found_leaf();
	idfont= plf->rlist->tpnr;
	cs = current_rules.optree[IDCLOSETREE];
	nextsym = lex_next();
	message("()");
    } else {
	lex_pop_state_use();
	lex_push_state();
	nextsym=oldsym;
    }
    while (nextsym && i<MAXIDLEN && !member(closeset, nextsym) &&
	   !member(cs,nextsym) &&
	   !member(current_rules.optree[ALLOPTREE],nextsym) &&
	   !member(current_rules.optree[IDFORBIDTREE], nextsym)) {
	if (nextsym<0x8000) {
	    buffer[i++]=nextsym;
	    nextsym=lex_next();
	} else
	    nextsym=lex_alter();
    }
    if (nextsym && i>0 && i<MAXIDLEN && (!idop || member(cs,nextsym))) {
	if (idop)
	    nextsym=lex_next();
	lex_pop_state_skip();
	message("OK)");
	return make_node(MP_Id, buffer, i,idfont,0);
    } else {
	lex_pop_state_use();
	nextsym=oldsym;
	message("??)");
	return make_node(0, buffer, i, idfont,0);
    }
}

static int identsep=0;

static void *parse_node_variable(Char *buffer, LeafTree *closeset)
{
    Char oldsym=nextsym;
    int i=0, identadd=0,correct=1;
    void *nn;

    lex_push_state();
    identsep = lex_value(",");
    message("(VAR:");
    if (!identsep) identsep=',';
    if (!member(closeset,identsep)) {
	set_leafsize(0);
	LT_insert(closeset,(void*)identsep);
	identadd=1;
    }
    while (1) {
	if (!(nn=parse_node_identifier(buffer+i,closeset))) { correct=0;break; }
	buffer[i++]=MP_Id;
	if (member(closeset, nextsym)) {
	    if (nextsym==identsep) {
		buffer[i++]=',';
		nextsym=lex_next();
	    } else break;
	} else {
	    correct=0;
	    break;
	}
    }
    if (identadd) {
	set_free_leaf(NULL);
	LT_delete(closeset, (void*)identsep);
    }
    if (correct) {
	lex_pop_state_skip();
	message("OK)");
	return make_node(MP_Var, buffer, i, 0,0);
    } else {
	lex_pop_state_use();
	nextsym=oldsym;
	message("??");
	return make_node(0, buffer, i, 0, 0);
    }
}

static void *parse_node_expression(Char *buffer, LeafTree *closeset, int prec)
{
    Char oldsym=nextsym;
    int i=0,j;
    void *nn=NULL;

    if (prec==21) {
	lex_push_state();
	message("(EXP:");
	while (nextsym) {
	    if (member(closeset, nextsym)) {
		lex_pop_state_skip();
		message("--)");
		return 0;
	    } else {
		j=EXPR;
		if (member(current_rules.optree[EXPRTREE],nextsym))
		    nn = parse_node_template(buffer+i, closeset,
					current_rules.optree[EXPRTREE],&j);
		if (!nn)
		    nn = parse_node_identifier(buffer+i,closeset);
		if (nn) {
		    lex_pop_state_skip();
		    message("OK)");
		    return nn;
		} else
		    nextsym=lex_alter();
	    }
	}
	lex_pop_state_use();
	nextsym=oldsym;
	message("??)");
	return 0;
    } else {
	lex_push_state();
	j=PREFIX;
	if (aig(nn=parse_node_operator(buffer+i,closeset, prec, &j))) {
	    buffer[i++]=MP_Op;
	    message2("-%i-",prec);
	}
	while (!member(closeset, nextsym)) {
	    if (aig(nn=parse_node_expression(buffer+i,closeset, prec+1))) {
		buffer[i++]=MP_Expr;
		j=POSTFIX;
		if (aig(nn=parse_node_operator(buffer+i,closeset, prec,&j))) {
		    buffer[i++]=MP_Op;
		    message2("-%i-",prec);
		    if (!(j&PREFIX)) break;
		} else break;
	    } else break;
	}
	if (i>0) {
	    lex_pop_state_skip();
	    if (i==1 && buffer[0]==MP_Expr)
		/* make_node not needed and result not used
		** (otherwise a stack of expressions is made
		*/
		return (void*) buffer;
	    else
		return make_node(MP_Expr, buffer, i, 0, 0);
	} else {
	    lex_pop_state_use();
	    nextsym=oldsym;
	    return make_node(0, buffer, i, 0, 0);
	}
    }
}

static Char next_place_holder(Char n)
{
    switch (n) {
    case MP_Id:   return MP_Var;
    case MP_Var:  return MP_Op;
    case MP_Op:   return MP_Expr;
    case MP_Expr: return MP_Text;
    default:   return 0;
    }
}

static void *parse_node_template(Char *buffer, LeafTree *closeset,
				 LeafTree *rules, int *optype)
{
    void *nn;
    Char oldsym=nextsym;
    int i=0;
    Char ph;
    int found=0;
    RULE *frule=NULL;

    lex_push_state();
    message("(NOT:");
#ifdef MESSTEMP
#undef message
#undef message2
#define message(A) fprintf(stderr, A)
#define message2(A,B) fprintf(stderr, A, B)
#else
#undef message
#undef message2
#define message(A)  /* */
#define message2(A,B) /* */
#endif
    message("\nPush0");
    while (nextsym && !found) {
	if (member(rules,nextsym)) {
	    ParseLeaf *plf = LT_found_leaf();
	    RULE *rl;
	    PARRULE *prl;
	    int n,j,k,lta;
	    LeafTree *cls=NULL;
	    prl = NewArray(PARRULE, plf->nr);
	    rl = plf->rlist;
	    message2("\nSym:%i\n", nextsym);
	    for (n=0; n<plf->nr && rl;) {
		if (rl->rtype & *optype) {
		    prl[n].rule=rl;
		    prl[n].pos=1;
		    n++;
		}
		rl=rl->next;
	    }
	    buffer[0]=nextsym;
	    /* prl contains all possible rules. use the pos field to
	    ** determine which rules are still valid.
	    ** backtracking is a little difficult in this situation
	    */
	    lex_push_state();
	    message("Push1\n");
	    nextsym=lex_next();
	    ph=MP_Id;
	    i=1;
	    while (i && !found) {
		if (i%2) { /* match place holder */
		    message2("\tPos:%i,", i);
		    message2("\tPh:%x,", ph&0xff);
		    set_free_leaf(NULL);
		    set_leafsize(0);
		    LT_free(cls);cls=NULL;
		    lta=0;
		    k=0;
		    for (j=0; j<n; j++) {
			if (prl[j].pos == i && Ph(prl[j].rule->text[i])==ph) {
			    prl[j].pos++;k++;
			    if (prl[j].rule->text[i+1]) {
			        int cv = prl[j].rule->text[i+1];
				LT_insert(cls, (void*)cv);
			    } else if (!lta) {
				/* the template ends with a place holder
				** add the current closeset
				*/
				LT_add_leaftree(cls, closeset);
				/* no nested templates ending with a
				** text place holder
				*/
				if (ph==MP_Text) {
				    int cv = prl[j].rule->text[0];
				    LT_insert(cls,(void*)cv);
				}
				lta=1;
			    }
			}
		    }
		    i++;
		    message2("\tmatch:%i\n",k);
		    if (k) {
			switch (ph) {
			case MP_Id:
			    nn = parse_node_identifier(buffer+i, cls);
			    break;
			case MP_Var:
			    nn = parse_node_variable(buffer+i, cls);
			    break;
			case MP_Op:
			    j=INFIX|EXPR;
			    nn = parse_node_operator(buffer+i, cls, 21, &j);
			    break;
			case MP_Expr:
			    nn = parse_node_expression(buffer+i, cls, 0);
			    break;
			case MP_Text:
			    /* lex_print_state(); */
			    nn = parse_node_text(buffer+i, cls);
			    /* lex_print_state(); */
			    break;
			default:
			    found=1;
			    nn=NULL;
			    continue;
			    break;
			}
		    } else nn=NULL;
		    if (nn) buffer[i-1]=ph;
		    else {
			for (j=0; j<n; j++)
			    if (prl[j].pos == i) prl[j].pos--;
			i--;
			if (ph)
			    ph = next_place_holder(ph);
			else {
			    for (j=0; j<n; j++)
				if (prl[j].pos == i) prl[j].pos--;
			    i--;
			    message("Pop1\n");
			    lex_pop_state_use();
			    nextsym = lex_alter();
			    /* break; */
			}
		    }
		} else { /* match symbol */
		    Char oldsym2=nextsym;
		    message2("\tPos: %i\nPush6\n",i);
		    lex_push_state();
		    while (!found) {
			k=0;
			message2("\t\tSym: %i",nextsym);
			for (j=0; j<n; j++) {
			    if (prl[j].pos == i &&
				prl[j].rule->text[i] == nextsym) {
				prl[j].pos++;
				k++;
			    }
			}
			message2("\tmatch: %i\n",k);
			if (k) {
			    buffer[i++]=nextsym;
			    if (!nextsym) {
                                /* matched rule ending in place holder */
				lex_pop_state_use();
				lex_push_state();
				nextsym=oldsym2;
				found=1;
			    } else {
				lex_pop_state_skip();
				lex_push_state();
				nextsym=lex_next();
				ph=MP_Id;
				break;
			    }
			} else if (!nextsym) {
			    i--;
			    message("Pop6\n");
			    if (i) make_node(0, buffer+i, 1,0,0);
			    lex_pop_state_skip();
			    ph = next_place_holder(ph);
			    break;
			} else
			    nextsym = lex_alter();
		    }
		}
	    }
	    if (found) {
		/* cleanup everything */
		int ja;
		message("Found something\n");
		for (ja=0;
		     ja<n && (prl[ja].pos != i ||  prl[ja].rule->text[i-1]!=0);
		     ja++);
		message2("ja:%i,",ja);
		message2("\tRule:%i\n", prl[ja].rule->tpnr);
		frule=prl[ja].rule;
		*optype=prl[ja].rule->rtype;
		ja=i;
		while (ja>0) {
		    message("Pop2\n");
		    lex_pop_state_skip();
		    ja=ja-2;
		}
		message("Pop3\n");
		lex_pop_state_skip();
	    } else {
		message("Fail\n");
		nextsym = lex_alter();
	    }
	    LT_free(cls);
	    free(prl);
	} else {
	    message2("Fail on symbol %i\n", nextsym);
	    nextsym=lex_alter();
	}
    }
#ifdef MESSAGE
#undef message2
#define message2(A,B) fprintf(stderr, A,B)
#undef message
#define message(A) fprintf(stderr, A)
#else
#undef message2
#define message2(A,B) /* */
#undef message
#define message(A) /* */
#endif
    if (frule) {
	int j;
	message("OK)");
	for (j=0; j<i; j++)
	    if (IsPh(buffer[j])) buffer[j]=frule->text[j];
	return make_node(MP_Op, buffer, i-1, frule->tpnr,0);
    } else {
	message("Pop5\n");
	lex_pop_state_use();
	nextsym=oldsym;
	message("??)");
	return NULL;
    }
}

static void *parse_node_operator(Char *buffer, LeafTree *closeset,
			    int prec, int *optype)
{
    void *nn;
    Char oldsym=nextsym;
    int i=0;
    
    if (prec==21) {
	lex_push_state();
	nn=NULL;
	while (nextsym && !nn) {
	    int cv=nextsym;
	    while (nextsym && !LT_member(current_rules.optree[ALLOPTREE],
					 (void*)cv))
		cv=nextsym=lex_alter();
	    for (i=0;i<=20 && !nn; i++) {
		if (LT_member(current_rules.optree[prec],(void*)cv))
		    nn = parse_node_template(buffer,closeset,
					current_rules.optree[prec], optype);
	    }
	}
	if (!nn) {
	    lex_pop_state_use();
	    nextsym=oldsym;
	} else
	    lex_pop_state_skip();
    } else {
	lex_push_state();
	i=parse_microspace(buffer);
	nn=parse_node_template(buffer, closeset,
			       current_rules.optree[prec], optype);
	if (nn) {
	    i=parse_microspace(buffer);
	    lex_pop_state_skip();
	} else {
	    nextsym=oldsym;
	    lex_pop_state_use();
	}
    }
    return nn;
}

static int maxparse=1;
static int parsecount=0;

static void *parse_node_text(Char *buffer, LeafTree *closeset)
{
    void *nn;
    int i=0;
    Char oldsym=nextsym;
    int cv;

    lex_push_state();
    message("(TXT:");
    cv=nextsym;
    while (nextsym && !LT_member(closeset, (void*)cv)) {
	if (LT_member(current_rules.optree[TEXTTREE], (void*)cv)) {
	    int j=TEXT;
	    nn = parse_node_template(buffer+i, closeset,
				current_rules.optree[TEXTTREE], &j);
	    if (nn) buffer[i++]=MP_Expr;
	    else nextsym=lex_alter();
	} else if (nextsym<0x8000) {
	    if (nextsym=='\n') nextsym=Newline; else
		if (nextsym=='\t') nextsym=Rtab;
	    buffer[i++]=nextsym;
	    nextsym=lex_next();
	} else
	    nextsym=lex_alter();
	cv=nextsym;
    }
    if (!nextsym && closeset) {
	/*
	** !nextsym, so we are at the end of the text we want to parse.
	** we have a number of options to proceed.
	** 1 return an error and parse everything again.
	**   if there are $n$  parse errors, the text could be parsed 2^n
	**   times if we are unlucky
	** 2 move the parsed text to a stack, change the text we want to
	**   parse and return an error.
	**   By adding a 0 at the position where this text starts, the
	**   parser will not parse this part again. You could add a special
	**   symbol at the position of the error.
	** 3 use a combination of these two strategies.
	**   By default use strategie 1 and use a counter to check how often
	**   the parser reaches the end without success. If the counter
	**   exceeds a certain value maxparse, use strategie 2 once and change
	**   the values of the counter and maxparse.
	**   Maxparse could depend on: length of the text, kind of templates,
	**   user preference, stack depth of text placeholders.
	** We use strategie 3, with maxparse=1
	*/ 
	parsecount++;
	if (parsecount<maxparse) {
	    message("??)");
	    lex_pop_state_use();
	    nextsym=oldsym;
	    return make_node(0, buffer, i, 0,0);
	} else {
	    message("BR)");
	    /* adjust maxparse and parsecount */
	    lex_pop_state_use();
	    nextsym=oldsym;
	    /* adjust text if text could be added */
	    if (add_parse_stack(buffer,i)) {
		while (aig(nextsym=lex_alter()));
		lex_pos--;
		mtext[lex_pos]=0;
	    }
	    return 0;
	}
    } else {
	message("OK)");
	lex_pop_state_skip();
	return make_node(MP_Text, buffer, i, 0, 0);
    }
}

int parse_text(char *text)
{
    void *nd;
    Char *buffer;
    int i;
    if (!text || !(i=strlen(text))) return 0;
    lex_start(text);
    buffer = NewArray(Char,i);
    if (!buffer) return 0;
    maxparse=1;
    parsecount=0;
    nextsym=lex_next();
    nd = parse_node_text(buffer, NULL);
    if (nd) join_parse_stack();
    free(buffer);
    return (nd!=0);
}

static void free_rules(void *rule)
{
    RULE *r= ((ParseLeaf*) rule)->rlist;
    RULE *h;
    while (r) {
	free(r->text);
	h=r->next;
	free(r);
	r=h;
    }
}

static void free_parser(void)
{
    int i;
    set_free_leaf(free_rules);
    for(i=0; i<MAXTREES; i++) {
	if (current_rules.optree[i])
	    LT_free(current_rules.optree[i]);
	current_rules.optree[i]=NULL;
    }
    set_free_leaf(free_lexleaf);
    if (current_rules.lextree) {
      LT_free(current_rules.lextree);
    }
    current_rules.lextree=NULL;
    current_rules.height=0;
    current_rules.nextretval=0x8000;
}

int parse_use_rules(PARSERULES *set)
{
    free_parser();
    if (set) current_rules = *set;
    return 0;
}

int parse_save_rules(PARSERULES *set)
{
    int i;
    *set=current_rules;
    for (i=0; i<MAXTREES; i++)
	current_rules.optree[i]=NULL;
    current_rules.lextree=NULL;
    current_rules.height=0;
    current_rules.nextretval=0x8000;
    return 0;
}

extern char *font_openmathtex(int,int);
extern char *font_opentexttex(int,int);
extern char *font_closetex(int,int); 
extern char *char_latex_next(Char *data, int *math);
void parser_init(void)
{
    Char *list;
    char *c;
    char cb[256];
    Char buffer[512];
    Char d;
    int i,n,m;
    int old_space=space_insens;
    int old_case=case_insens;
    space_insens=0;
    case_insens=0;

    /* output generated by mathspad:  Display, MP_Expr, MP_Text switches */
    if (aig(list = (Char*) malloc(15*sizeof(Char)))) {
	list[0]= lex_add_string("\\begin{mpdisplay}{",0);
	list[1]=list[3]=list[5]=list[9]=MP_Text;
	list[7]=MP_Expr;
	list[2]=list[4]=list[6]=lex_add_string("}{",0);
	list[8]=lex_add_string("}\n\t",0);
	list[10]=lex_add_string("\n\\end{mpdisplay}", 0);
	list[11]=0;
	if (!parse_add_rule(TEXT, list, 11, INTERNAL_DISPLAY, 0))
	    free(list);
    }
    if (aig(list = (Char*) malloc(4*sizeof(Char)))) {
	list[0]=list[2]=lex_add_string("$", 0);
	list[1]=MP_Expr;
	list[3]=0;
	if (!parse_add_rule(TEXT, list, 3, INTERNAL_EXPRESSION,0))
	    free(list);
    }
    if (aig(list = (Char*) malloc(4*sizeof(Char)))) {
	list[0]=lex_add_string("\\mbox{", 0);
	list[2]=lex_add_string("}", 0);
	list[1]=MP_Text;
	list[3]=0;
	if (!parse_add_rule(EXPR, list, 3, INTERNAL_TEXT,0))
	    free(list);
    }
    if (aig(list = (Char*) malloc(4*sizeof(Char)))) {
	list[0]=lex_add_string("(", 0);
	list[2]=lex_add_string(")", 0);
	list[1]=MP_Expr;
	list[3]=0;
	if (!parse_add_rule(EXPR, list, 3, INTERNAL_BRACES,0))
	    free(list);
    }

    /* identifier fonts */
    for (i=0; i<256; i++) {
	n=0;
	if (aig(c=font_openmathtex(i,0))) {
	    strncpy(cb,c,256);
	    buffer[n++]=lex_add_string(cb,0);
	}
	if (aig(c=font_opentexttex(i,0))) {
	    strncpy(cb,c,256);
	    buffer[n++]=lex_add_string(cb,0);
	}
	if (n) parse_add_rule(IDOPEN,buffer,n,i,0);
	if (aig(c=font_closetex(i,0))) {
	    strncpy(cb,c,256);
	    buffer[0]=lex_add_string(cb,0);
	    parse_add_rule(IDCLOSE, buffer, 1, i,0);
	}
    }
    buffer[0]=' ';buffer[1]='\n'; buffer[2]=',';
    /* micro spacing */
    buffer[3]= lex_add_string("\\ms{",0);buffer[4]=0;
    parse_add_rule(IDFORBID, buffer, 4, 0,0);
    /*
    ** symbol macros. m indicates if it is used in mathmode or in textmode.
    ** d is increased by char_latex_next to move to the next character.
    ** If two characters map to the same string, the first will be used.
    */
    m=0; d=0;
    while (aig(c=char_latex_next(&d,&m)))
	lex_add_string(c,d);
    case_insens=old_case;
    space_insens=old_space;
}

void set_parser_case(int on)
{
    case_insens=on;
}

void set_parser_space(int on)
{
    space_insens=on;
}



#ifdef STAND_ALONE

int main(int argc, char **argv)
{
    int i,j,k;
    char c;
    char buf[1000];
    Char bufC[1000];
    char *h;
    struct stat stbuf;
    Char *ch;
    FILE *f;

    if (argc<3) {
	printf(stderr, "Usage: %s item-file scan-file\n", argv[0]);
	return -1;
    }
    f= fopen(argv[1], "r");
    i=1;k=1;
    ch=bufC;
    while (fgets(buf, 1000, f)) {
	j=0;
	h=buf;
	if (!strncmp(buf, "Rule:",5)) {
	    Char *p = ch;
	    int rtype=0,rprec=0;
	    h=buf+5;
	    switch (*h) {
	    case 'T': rtype=TEXT; break;
	    case 'P': rtype=EXPR|PREFIX; break;
	    case 'I': rtype=EXPR|INFIX; break;
	    case 'p': rtype=EXPR|POSTFIX; break;
	    case 'E': rtype=EXPR; break;
	    case 'O': rtype=IDOPEN; break;
	    case 'C': rtype=IDCLOSE; break;
	    case 'F': rtype=IDFORBID; break;
	    default: rtype=TEXT; break;
	    }
	    h++;
	    rprec=0;
	    if ('0'< *h && *h<='9') rprec=*h++ -'0';
	    if (rprec<3 && '0'<= *h && *h<='9') rprec=rprec*10 + *h++ -'0';
	    h++;
	    j=0;
	    while (*h) {
		switch (*h) {
		case 'T': *ch++ = MP_Text; j++;h++; break;
		case 'E': *ch++ = MP_Expr; j++;h++; break;
		case 'I': *ch++ = MP_Id;   j++;h++; break;
		case 'V': *ch++ = MP_Var;  j++;h++; break;
		case 'O': *ch++ = MP_Op;   j++;h++; break;
		case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9':
		    *ch=*h++ -'0';
		    while ('0'<=*h && *h<='9') *ch = *ch*10+(*h++)-'0';
		    *ch=*ch+0x8000;
		    ch++;j++;
		    break;
		default: h++; break;
		}
	    }
	    *ch++ =0;
	    if (!parse_add_rule(rtype,p, j, 5000+k++,rprec))
		fprintf(stderr, "Bad rule: %s", buf);
	} else {
	    while (buf[j] && buf[j]!='\n') {
		if (buf[j]=='\\') {
		    j++;
		    if (buf[j]=='n') buf[j]='\n';
		}
		*h++=buf[j++];
	    }
	    *h='\0';
	    lex_add_string(buf, 0x8000+ i++);
	}
    }
    fclose(f);
    if (stat(argv[2], &stbuf) != -1) i=stbuf.st_size; else i=0;
    if (!i) {
	printf(stderr, "Unable to determine the size of %s.\n", argv[2]);
	return -1;
    }
    h = NewArray(char, i);
    f=fopen(argv[2], "r");
    j=fread(h, i, 1, f);
    fclose(f);
    buffer = NewArray(Char, i);
    lex_start(h);
    while (aig(i=lex_next())) {
	if (i>0x8000)
	    printf(" [%i] ", i-0x8000);
	else
	    printf("%c" ,i);
    }
    lex_start(h);
    nextsym=lex_next();
    parse_node_text(buffer, NULL);
}

#endif

