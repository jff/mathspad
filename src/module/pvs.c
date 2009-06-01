#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mathpad.h"
#include "system.h"
#include "message.h"
#include "sources.h"
#include "remark.h"
#include "edit.h"
#include "editor.h"
#include "notatype.h"
#include "latexout.h"
#include "unistring.h"
#include "pvs.h"
#include "language.h"
#include "unitype.h"
#include "refcounting.h"
#include "funcs.h"

#define PVSMESSAGE "[pvs interaction] "

static int
  wrong_mark=0xd7,
  good_mark=0x2713;
static int
  wrong_template, wrong_open_template,
  warning_template, warning_open_template,
  good_template;
static unsigned long  equalhint_uid = 2946556487ul;
static unsigned long
  wrong_uid = 2439819162ul,
  wrong_open_uid = 2439819214ul,
  warning_uid = 2439819290ul,
  warning_open_uid = 2439819391ul,
  good_uid = 2439819443ul;
static int pvs_open_pvs_files=0;
static int pvs_after_init=0;
static int pvs_in_checker=0;
static int pvs_before_realtime=0;
static int pvs_do_leibnitz=0;
static int using_booleans=0;
static Char *pvs_context_dir=0;
static Char *pvs_hint_file=0;
static Char *pvs_lemma_name=0;
static Char *pvs_stencil_file=0;

static int lasthintfile=0;
static int lastlemmanr=0;
static unsigned long firsthintnr=2946556486UL;
static unsigned long lasthintnr=2946556493UL;
static unsigned long pvshiddenstep=1234567890UL;


typedef struct KeywordItem KeywordItem;
struct KeywordItem {
  Char *str;
  int len;
  int induct;
  char *step;
  KeywordItem *next;
};

static KeywordItem *kwlist=0;

#define InitStep 1
#define HiddenStep 2
#define ExpressionStep 3
#define KeywordStep 4
#define FinishStep 5
#define SkipStep 6

typedef struct ProofStep ProofStep;
struct ProofStep {
  KeywordItem *keyword;  /* the matched keyword */
  char *pvsinput;        /* input to be send to pvs */
  Char *comment;         /* a comment as feedback to the user */
  char *result;          /* the PVS result after appliing the step */
  int failed;            /* did the step fail */
  int steptype;          /* what kind of step */
  int use_result;        /* use final result ? */
  ProofStep *next;       /* the next step in the proof */
};

typedef struct PVSProof PVSProof;

struct PVSProof {
  ProofStep *step;
  char *file;
  char *theory;
  char *lemma;
  int linenr;
  void *selection;
  PVSProof *next;
};

static PVSProof *toproof=0;
static ProofStep *curstep=0;


static char *Tstr="T";
static char *NILstr="NIL";
static char *field[40];

static int fields(unsigned char *buffer)
{
  int i,j;
  char *h;
  i=0;
  h=(char*)buffer;
  do {
    field[i]=h;
    h=strchr(h,'&');
    if (h) *h++ = 0;
    i++;
  } while (h);
  for (j=0; j<i; j++) {
    if (!strcmp(field[j],NILstr)) field[j]=NILstr;
    if (!strcmp(field[j],Tstr)) field[j]=Tstr;
  }
  return i;
}

#define parse_position(STR,SL,SK, EL,EK) \
      sscanf(STR,"(%i %i %i %i)", &SL,&SK,&EL,&EK)

static void handle_pvs_msg(unsigned char *buffer)
{
  Char *buf;
  Char *messbuf;
  Char newlinebuf[2];
  if (!strncmp(buffer, "Context changed to ", 19)) {
    char *h=buffer+19;
    int i=strlen(h);
    while (isspace(h[i-1])) i--;
    h[i]=0;
    decrease_refcount(pvs_context_dir);
    pvs_context_dir=standard_dir(Ustrdup(LocaletoUstr(h)));
    increase_refcount(pvs_context_dir, free);
  }
  buf=LocaletoUstr(buffer);
  messbuf=translate("PVS Messages");
  message(MP_MESSAGE, buf);
  string_to_window(messbuf, buf);
  newlinebuf[0]=Newline;
  newlinebuf[1]=0;
  string_to_window(messbuf, newlinebuf);
}

static void handle_pvs_log(unsigned char *buffer)
{
  string_to_window(translate("PVS Log"), LocaletoUstr(buffer));
  string_to_window(translate("PVS Log"), translate("\n"));
}

static void handle_pvs_warn(unsigned char *buffer)
{
  string_to_window(translate("*pvs-warn*"), LocaletoUstr(buffer));
}

static void handle_pvs_err(unsigned char *buffer)
{
  /* buffer contains "file&dir&message&error&place" */
  if (fields(buffer)==5) {
    int sl,sc;
    char fullname[1024];
    char errormessage[10000];
    int errpos=0;
    FILE *f;
    if (pvs_open_pvs_files) {
      strcpy(fullname,field[1]);
      strcat(fullname,field[0]);
      strcat(fullname,".pvs");
      sscanf(field[4], "%i %i", &sl, &sc);
      open_temporary_file(fullname, fullname, 1, sl);
    }
    sprintf((char*)errormessage, "Error in '%s' on line %i, column %i.\n"
	    "%s\n"
	    "\n", field[0], sl, sc, field[2]);
    errpos=strlen(errormessage);
    f=fopen(field[3],"r");
    if (f) {
      int t;
      t=fread(errormessage+errpos, 1, 10000-errpos-1, f);
      errormessage[errpos+t]=0;
      fclose(f);
    }
    message(MP_CLICKREMARK, LocaletoUstr((unsigned char*)errormessage));
    unlink(field[3]);
  }
}

static void handle_pvs_qry(unsigned char *buffer)
{
  /* buffer contains "file&dir&query&mitems&place" */
  if (fields(buffer)==5) {
  }
}

static void handle_pvs_buf(unsigned char *buffer)
{
  /* buffer contains "bufname&file&display&read_only" */
  int nf = fields(buffer);
  /* PVS version 2.1 used 4 fields, PVS version 2.2 uses 5 fields */
  if ((nf==4 || nf==5) && field[1]!=NILstr) {
    if (pvs_open_pvs_files) {
      open_temporary_file(field[0], field[1], field[2]!=NILstr,0);
    }
    unlink(field[1]);
  }
}

static void pvs_send_answer(void *data, int n)
{
  char *ans;
  switch (n) {
  case 0:  ans="t\n"; break;
  case 1:  ans="nil\n"; break;
  case 2:  ans=":abort\n"; break;
  default: ans="t\n"; break;
  }
  edit_string_to_proces(LocaletoUstr((unsigned char*)ans),
			translate("PVS-shell"));
}

static void handle_pvs_yn(unsigned char *buffer)
{
  /* buffer contains "message&yesno_or_yn&timeout" */
  if (fields(buffer)==3) {
    /* ignore timeout */
    Char *buttons[4];
    buttons[0]=translate(" Yes ");
    buttons[1]=translate(" No ");
    buttons[2]=translate(" Cancel ");
    buttons[3]=0;
    remark_make(0,0,pvs_send_answer, REMARK_BUTTON, translate(field[0]),
		buttons, 0,0,0);
  }
}

static void handle_pvs_bel(unsigned char *buffer)
{
  /* just beep */
  message(MP_ERROR, 0);
}

static void handle_pvs_loc(unsigned char *buffer)
{
  /* buffer contains "dir&file&place" */
  if (fields(buffer)==3 && pvs_open_pvs_files) {
    int sl,sc,el,ec;
    char filename[1024];
    if (field[0]!=NILstr) {
      strcpy(filename, field[0]);
    } else {
      filename[0]=0;
    }
    strcat(filename,field[1]);
    parse_position(field[2],sl,sc,el,ec);
    open_temporary_file(filename, filename, 0,sl);
  }
}

static void handle_pvs_mod(unsigned char *buffer)
{
  /* buffer contains "dir&file&place&filename" */
  if (fields(buffer)==4) {
    fprintf(stderr, PVSMESSAGE "Modifing commands not supported\n"
	    PVSMESSAGE "%s&%s&%s&%s\n", field[0],field[1],field[2],field[3]);
  }
}

static void handle_pvs_pmt(char *buffer)
{
  /* buffer contains "string"   (ask for directory ???) */
  fprintf(stderr, PVSMESSAGE "Prompt commands not supported\n"
	  PVSMESSAGE "%s\n", buffer);
}

static void handle_pvs_dis(unsigned char *buffer)
{
  /* buffer contains "proof&instance&type&value" */
  if (fields(buffer)==4) {
    fprintf(stderr, PVSMESSAGE "Display commands not supported\n"
	    PVSMESSAGE "%s&%s&%s&%s\n", field[0],field[1],field[2],field[3]);
  }
}


int first_wish_call=1;
static void handle_pvs_wish(char *buffer)
{
  char *f;
  if (first_wish_call) {
    fprintf(stderr, PVSMESSAGE "Wish (tcl) not supported yet.\n");
    first_wish_call=0;
  }
  f=strstr(buffer,"exec rm -f ");
  if (f) {
    f=f+11;
    unlink(f);
  }
}

static void pvs_start_proof(PVSProof *proof);

static void pvs_get_templates(void)
{
  if (!pvs_stencil_file) return;
  load_notation_window(-1, pvs_stencil_file);
  wrong_template=notation_with_number(wrong_uid);
  wrong_open_template=notation_with_number(wrong_open_uid);
  warning_template=notation_with_number(warning_uid);
  warning_open_template=notation_with_number(warning_open_uid);
  good_template=notation_with_number(good_uid);
}

static void handle_pvs_eval(char *buffer)
{
  if (!strcmp(buffer,"(setq *pvs-initialized* t)")) {
    pvs_after_init=1;
  } else if (!strcmp(buffer, "(setq pvs-in-checker nil)")) {
    pvs_in_checker=0;
    edit_string_to_proces(LocaletoUstr((unsigned char*)"\"nil\"\n"),
			  translate("PVS-shell"));
    pvs_before_realtime=1;
    pvs_start_proof(NULL);
  } else if (!strcmp(buffer, "(setq pvs-in-checker t)")) {
    pvs_in_checker=1;
    edit_string_to_proces(LocaletoUstr((unsigned char*)"\"t\"\n"),
			  translate("PVS-shell"));
  } else {
    fprintf(stderr, PVSMESSAGE "Unknown pvs-eval command: %s\n",
	    buffer);
  }
}

static char unknownmess[40];
static char unknownend[50];

#define PVSPREFIX  ":pvs-"
#define PVSPREFLEN 5
#define PVSENDPREF " :end-pvs-"

static void handle_pvs_unknown(char *buffer)
{
  fprintf(stderr, PVSMESSAGE "Unknown pvs message:  " PVSPREFIX "%s %s %s",
	  unknownmess, buffer, unknownend);
}



static struct {
  char *keyval;
  int keylen;
  char *endstr;
  int endlen;
  void (*func)();
} pvsmessage[] = {
  { "msg ",  4, PVSENDPREF "msg\n",  14, handle_pvs_msg  },
  { "log ",  4, PVSENDPREF "log\n",  14, handle_pvs_log  },
  { "warn ", 5, PVSENDPREF "warn\n", 15, handle_pvs_warn },
  { "err ",  4, PVSENDPREF "err\n",  14, handle_pvs_err  },
  { "qry ",  4, PVSENDPREF "qry\n",  14, handle_pvs_qry  },
  { "buf ",  4, PVSENDPREF "buf\n",  14, handle_pvs_buf  },
  { "yn ",   3, PVSENDPREF "yn\n",   13, handle_pvs_yn   },
  { "bel ",  4, PVSENDPREF "bel\n",  14, handle_pvs_bel  },
  { "loc ",  4, PVSENDPREF "loc\n",  14, handle_pvs_loc  },
  { "mod ",  4, PVSENDPREF "mod\n",  14, handle_pvs_mod  },
  { "pmt ",  4, PVSENDPREF "pmt\n",  14, handle_pvs_pmt  },
  { "dis ",  4, PVSENDPREF "dis\n",  14, handle_pvs_dis  },
  { "wish ", 5, PVSENDPREF "wish\n", 15, handle_pvs_wish },
  { "eval ", 5, PVSENDPREF "eval\n", 15, handle_pvs_eval },
  { 0,       0, 0,                  0,  0               },
  { unknownmess, 0, unknownend, 0, handle_pvs_unknown }
};

static char *stilltoparse=0;
static int stilllen=0;
static int parsemess= -1;

void pvs_use_result(char *pvs_result, int forward)
{
  char *c, *h, bcount;
  PVSProof *prf;
  /* search formula that has to be proven */
  c=strstr(pvs_result, "|----");
  if (!c) { fprintf(stderr, ">> No |----\n");return; }
  h = strchr(c,'\n');
  if (h) c=h+1;
  while (!isspace(*c)) c++;
  while (isspace(*c)) c++;
  /* search for result variable ... */
  h=strstr(c," = Tresult");
  if (!h && !using_booleans) { fprintf(stderr, ">> No = Tresult\n");return;}
  else {
    /* ... and remove it. */ 
    if (h) *h=0;
  }
  h=c;
  /* Make sure the () match */
  bcount=0;
  while (*h) {
    if (*h=='(') bcount++;
    else if (*h==')') bcount--;
    h++;
  }
  while (bcount>0) { *h++=')'; bcount--;}
  *h='\0';
  /* parse the result */
  pvs_parse_string(c);
  /* result is on the stack */
  prf = toproof;
  insert_template_selection(prf->selection,
			    notation_with_number(equalhint_uid));
  /* make buffer with hint text */
  {
    Char buffer[1024];
    Char *bp;
    ProofStep *p;
    p=prf->step;
    bp=buffer;
    while (p) {
      if (p->steptype==KeywordStep &&
	  p->keyword) {
	if (bp!=buffer) { *bp++=','; *bp++=' '; }
	Ustrcpy(bp, p->keyword->str);
	bp=bp+p->keyword->len;
      }
      p=p->next;
    }
    insert_string_selection(prf->selection, buffer);
  }
  next_node_selection(prf->selection);
  if (!forward) {
    commute_selection(prf->selection);
  }
  /* insert_string_selection(prf->selection, translate("NoWay")); */
  insert_parse_result(prf->selection);
  redraw_selection(prf->selection);
}


void pvs_parse_result(char *pvs_result)
{
  /* divide result in premise and target. Use natural language to combine
  ** the expressions.
  */
  char *c, *h;
  Char buffer[512];
  int pos,assump_count;
  Char *part;
  /* fprintf(stderr,"Result:>>>\n%s\n<<<\n", pvs_result); */
  assump_count=0; part=0; pos=0;
  c=pvs_result;
  while (c && strncmp("  |-----", c,8)) {
    /* fprintf(stderr,"Assumption %i, position %i\n", assump_count, c-pvs_result); */
    if (!assump_count) {
      part=translate("Under the assumption of ");
      Ustrcpy(buffer,part);
      pos=Ustrlen(part);
    } else {
      buffer[pos++]=',';
      buffer[pos++]=' ';
    }
    buffer[pos++]=MP_Expr;
    assump_count++;
    while (!isspace(*c)) c++;
    while (isspace(*c)) c++;
    h=c;
    c=strchr(c,'\n');
    while (c && isspace(c[1]) && c[3]!='|') {
      c=strchr(c+1, '\n');
    }
    if (!c) c=h;
    *c='\0';
    /* fprintf(stderr, ">>>\n%s\n<<<\n",h); */
    pvs_parse_string(h);
    *c='\n';
    c++;
  }
  if (assump_count>1) {
    /* replace last ", E" by " and E" */
    pos=pos-3;
    part=translate(" and ");
    Ustrcpy(buffer+pos, part);
    pos=pos+Ustrlen(part);
    buffer[pos++]=MP_Expr;
  }
  if (assump_count) {
    buffer[pos++]=',';
    buffer[pos++]=Newline;
  }
  part=translate("Prove that ");
  Ustrcpy(buffer+pos, part);
  if (assump_count) buffer[pos]=Utolower(buffer[pos]);
  pos=pos+Ustrlen(part);
  buffer[pos++]=MP_Expr;
  if (pvs_do_leibnitz) {
    buffer[pos++]=Newline;
    part=translate("Or: ");
    Ustrcpy(buffer+pos, part);
    pos=pos+Ustrlen(part);
    buffer[pos++]=MP_Expr;
  }
  buffer[pos++]='.';
  buffer[pos++]=Newline;
  buffer[pos]=0;
  h=strchr(c,'\n');
  if (h) c=h+1;
  /* fprintf(stderr, "To prove1:>>>\n%s\n<<<\n", c); */
  while (!isspace(*c)) c++;
  while (isspace(*c)) c++;
  /* fprintf(stderr, "To prove2:>>>\n%s\n<<<\n", c); */
  pvs_parse_string(c);
  if (pvs_do_leibnitz) {
    apply_leibnitz();
  }
  make_node(MP_Text, buffer, pos,0,0);
}

int pvs_parse_prove(unsigned char *buffer, unsigned int *len);

#ifdef DEBUG
#define DEBUGMESSAGE(A) fprintf(stderr, "%i:in_checker: %c, message:\n>>>%s\n<<<\n", __LINE__, (pvs_in_checker?'Y':'N'), A)
#else
#define DEBUGMESSAGE(A)
#endif

static Char busylist[4][2] = { {'-', 0}, {'\\',0},{'|',0},{'/',0}};
static int currentbusy=0;

int pvs_parse(unsigned char *buffer, unsigned int *len)
{
  char *pos, *lastpos;
  unsigned int reslen;
  reslen=0;
  buffer[*len]=0;
  lastpos=(char*)buffer;
  DEBUGMESSAGE(buffer);
  pos = strstr((char*)buffer, PVSPREFIX);
  while (pos) {
    char *t;
    int i;
    int savereslen;
    DEBUGMESSAGE(pos);
    /* copy plain content */
    savereslen=reslen;
    if (pvs_after_init) {
      while (lastpos<pos) {
	buffer[reslen++] = *lastpos++;
      }
    }
    if (pvs_in_checker) {
      unsigned char cv;
      int nres;
      nres = reslen-savereslen;
      if (nres) {
	cv = buffer[reslen];
	buffer[reslen]=0;
	pvs_parse_prove(buffer+savereslen, &nres);
	buffer[reslen]=cv;
	reslen=savereslen+nres;
      }
    }
    i=0;
    while (pvsmessage[i].keyval &&
	   strncmp(pos+PVSPREFLEN,pvsmessage[i].keyval,pvsmessage[i].keylen))
      i++;
    if (!pvsmessage[i].keyval) {
      /* unknown pvs message */
      char *b;
      i++;
      b=pos+PVSPREFLEN;
      while (*b!=' ') b++;
      *b=0;
      strcpy(pvsmessage[i].keyval,pos+PVSPREFLEN);
      pvsmessage[i].keylen = strlen(pos+PVSPREFLEN);
      sprintf(pvsmessage[i].endstr, PVSENDPREF "%s\n", pos+PVSPREFLEN);
      pvsmessage[i].endlen = strlen(pvsmessage[i].endstr);
    }
    t=strstr(pos, pvsmessage[i].endstr);
    if (t) {
      *t=0;
      (*pvsmessage[i].func)(pos+PVSPREFLEN+pvsmessage[i].keylen);
      t=t+pvsmessage[i].endlen;
    } else {
      int k;
      parsemess=i;
      k=strlen(pos);
      if (k) {
	if (stilllen<k) {
	  if (stilltoparse)
	    stilltoparse = realloc(stilltoparse, (k+1)*sizeof(char));
	  else
	    stilltoparse = malloc((k+1)*sizeof(char));
	  stilllen=k;
	}
	strcpy(stilltoparse, pos);
	t=pos+k;
      }
    }
    lastpos=t;
    pos=strstr(lastpos,PVSPREFIX);
  }
  {
    int savereslen;
    savereslen=reslen;
    if (pvs_after_init) {
      while (*lastpos) {
	buffer[reslen++]=*lastpos++;
      }
    }
    buffer[reslen]=0;
    DEBUGMESSAGE(buffer);
    if (strstr(buffer, "(Yes or No)")) {
      edit_string_to_proces(translate("no\n"), translate("PVS-shell"));
    }
    if (pvs_before_realtime &&
	strstr(buffer, "Real time =")) {
      pvs_before_realtime=0;
      pvs_start_proof(NULL);
    }      
    if (pvs_in_checker) {
      int nres;
      nres = reslen-savereslen;
      if (nres) {
	pvs_parse_prove(buffer,&nres);
	reslen=savereslen+nres;
      }
    }
    DEBUGMESSAGE(buffer);
  }
  message(MP_MESSAGE,busylist[currentbusy]);
  currentbusy=(currentbusy+1)%4;
  *len=reslen;
  return (reslen==0);
}

void pvs_start(Char *title)
{
  pvs_after_init=0;
  pvs_in_checker=0;
  open_program(translate("pvsscript %i &"), title, pvs_parse);
  pvs_get_templates();
}

void pvs_add_keyword(Char *keyword, Char *step, int induct)
{
  KeywordItem *kwi;
  kwi=malloc(sizeof(KeywordItem));
  kwi->str=keyword;
  kwi->len=Ustrlen(keyword);
  kwi->step=strdup((char*)UstrtoLocale(step));
  kwi->induct=induct;
  kwi->next=kwlist;
  kwlist=kwi;
}

#define InRubbish 1
#define InExpr 2

static KeywordItem *get_pvskeyword(Char *word);

static void finish_current_proof(int pvs_proof_still_open)
{
  PVSProof *prf;
  Index marks[6];
  curstep=0;
  prf=toproof;
  toproof=toproof->next;
  marks[0]=wrong_template;
  marks[1]=wrong_open_template;
  marks[2]=warning_template;
  marks[3]=warning_open_template;
  marks[4]=good_template;
  marks[5]=0;
  filter_template_selection(prf->selection, marks);
  if (pvs_proof_still_open) {
    KeywordItem *kwi;
    insert_parse_result(prf->selection);
    kwi = get_pvskeyword(translate("STOPPVSPROOF"));
    if (kwi) edit_string_to_proces(LocaletoUstr((unsigned char*)kwi->step),
				   translate("PVS-shell"));
  } else {
    insert_template_selection(prf->selection, good_template);
    /* insert_selection(prf->selection, good_mark); */
  }
  redraw_selection(prf->selection);
  destruct_selection(prf->selection);
  /* free prf */
}

static void step_clear_result(ProofStep *ps)
{
  if (ps->result) {
    free(ps->result);
    ps->result=0;
  }
}

static void step_add_to_result(ProofStep *ps, char *res)
{
  int i;
  if (ps->result) i=strlen(ps->result); else i=0;
  i=i+strlen(res)+1;
  if (ps->result) {
    char *h;
    h=realloc(ps->result, sizeof(char)*i);
    ps->result=h;
  } else {
    ps->result=malloc(sizeof(char)*i);
    ps->result[0]=0;
  }
  strcat(ps->result, res);
}

static int prove_pos=InRubbish;
static int after_newline=0;

int pvs_parse_prove(unsigned char *buffer, unsigned int *len)
{
  char *pos, *startat;
  if (!toproof || !curstep) {
    /* end of proof, expect a 'really want to quit?' prompt */
    if (strstr(buffer,"want to quit?")) {
      edit_string_to_proces(translate("y\n"), translate("PVS-shell"));
    }
    return 0;
  }
  startat=(char*)buffer;
  while (startat && *startat) {
    switch (prove_pos) {
    case InRubbish:
      pos = strstr(startat, "Q.E.D.");
      if (pos) {
	Char *pvsres;
	Char txt[10240];
	pvsres = translate("PVS Results");
	string_to_window(pvsres, LocaletoUstr((unsigned char*)toproof->lemma));
	string_to_window(pvsres, translate(" is correct according to PVS.\n"));
	curstep = curstep->next;
	txt[0]=0;
	while (curstep) {
	  if (curstep->steptype == KeywordStep) {
	    string_to_window(pvsres, translate("Warning: keyword \""));
	    string_to_window(pvsres, curstep->keyword->str);
	    string_to_window(pvsres, translate("\" is not needed\n"));
	    Ustrcat(txt, translate("Warning: keyword \""));
	    Ustrcat(txt, curstep->keyword->str);
	    Ustrcat(txt, translate("\" is not needed.\n"));
	  }
	  curstep=curstep->next;
	}
	if (txt[0]) {
	  int i=Ustrlen(txt);
	  make_node(MP_Text, txt, i, 0,0);
	  txt[0]=PhNum2Char(MP_Text,1);
	  make_node(MP_Op, txt, 1, warning_template,0);
	} else {
	  make_node(MP_Op, txt, 0, good_template,0);
	}
	finish_current_proof(0);
	startat=0;
	break;
      }
      pos = strstr(startat, "this yields ");
      if (pos && strstr(pos, "subgoals")) {
	Char *pvsres;
	Char *split_error;
	Char txt[1024];
	pvsres = translate("PVS Results");
	split_error =
	  translate(": PVS split the proof into subgoals. Unable to proceed.");
	string_to_window(pvsres, LocaletoUstr((unsigned char*)toproof->lemma));
	string_to_window(pvsres, split_error);
	string_to_window(pvsres, LocaletoUstr("\n"));
	message2(MP_ERROR, LocaletoUstr(toproof->lemma), split_error);
	Ustrcpy(txt, split_error);
	make_node(MP_Text, txt, Ustrlen(txt),0,0);
	txt[0]=PhNum2Char(MP_Text,1);
	make_node(MP_Op, txt, 1, wrong_template, 0);
	finish_current_proof(1);
	startat=0;
	break;
      }
      pos = strstr(startat, "want to quit?");
      if (pos) {
	edit_string_to_proces(translate("y\n"), translate("PVS-shell"));
	startat=0;
	/* no matter what happened, the proof is finished */
	break;
      }
      pos = strstr(startat, toproof->lemma);
      if (pos) {
	pos=pos+strlen(toproof->lemma);
	if (*pos!='.') {
	  /* perhaps could use strchr(pos,'\n') here. */
	  while (*pos && *pos!=':') pos++;
	  if (*pos==':') pos++;
	  while (*pos==' ' || *pos=='\t') pos++;
	  while (*pos=='\n') pos++;
	  step_clear_result(curstep);
	  prove_pos = InExpr;
	  startat = pos;
	  break;
	}
      }
      pos = strstr(startat, "Rule? ");
      if (pos) {
	if (!pos[6]) {
	  /* !pos[6] indicates that there is no more input yet. */
	  if (curstep->next) {
	    ProofStep *h=curstep;
	    /* send next step */
	    curstep=curstep->next;
	    if (curstep->pvsinput) {
	      edit_string_to_proces(LocaletoUstr(curstep->pvsinput),
				    translate("PVS-shell"));
	    }
	    if (curstep->keyword) {
	      edit_string_to_proces(LocaletoUstr(curstep->keyword->step),
				    translate("PVS-shell"));
	    }
	    if (curstep->use_result) {
	      PVSProof *prf;
	      pvs_use_result(h->result, curstep->use_result-1);
	      curstep=0;
	      prf=toproof;
	      toproof=toproof->next;
	      destruct_selection(prf->selection);
	      /* free prf */
	    }
	  } else {
	    Char *pvsres;
	    Char *insuf_error;
	    pvsres= translate("PVS Results");
	    insuf_error= translate(": Insufficient information to prove the selected hint.");
	    message2(MP_ERROR, LocaletoUstr(toproof->lemma), insuf_error);
	    string_to_window(pvsres, LocaletoUstr(toproof->lemma));
	    string_to_window(pvsres, insuf_error);
	    string_to_window(pvsres, LocaletoUstr("\n"));
	    string_to_window(pvsres, translate("The proof attempt ends with the following PVS expression:\n"));
	    /* add parse - diff - analyse routine here */
	    if (1)
	    {
	      Char txt[2];
	      pvs_parse_result(curstep->result);
	      txt[0]=PhNum2Char(MP_Text,1);
	      make_node(MP_Op, txt, 1, wrong_template,0);
	      /* structure_to_window(pvsres); */
	    } else {
	      string_to_window(pvsres, LocaletoUstr(curstep->result));
	    }
	    string_to_window(pvsres, LocaletoUstr("\n"));
	    finish_current_proof(1);
	  }
	}
	startat=pos+6;
	break;
      }
      pos = strstr(startat, ";;; GC:");
      if (pos) {
	message(MP_MESSAGE, translate("PVS collects garbage. One moment please."));
	startat = pos+7;
	break;
      }
      pos = strstr(startat, ";;; Finished GC");
      if (pos) {
	message(MP_MESSAGE, translate("PVS finished collecting garbage. Sorry for the inconvenience."));
	startat = pos+15;
	break;
      }
      /* nothing usefull found. Just skip everything. */
      startat=0;
      break;
    case InExpr:
      /* search for the PVS prompt */
      if (after_newline && startat[0]=='\n') {
	int stepreslen;
	stepreslen = strlen(curstep->result);
	curstep->result[stepreslen-1]=0;
	startat++;
	after_newline=0;
	prove_pos=InRubbish;
      } else {
	pos = strstr(startat, "\n\n");
	if (pos) {
	  *pos=0;
	  step_add_to_result(curstep,startat);
	  after_newline=0;
	  startat=pos+2;
	  prove_pos=InRubbish;
	} else {
	  step_add_to_result(curstep,startat);
	  if (startat[strlen(startat)-1]=='\n') {
	    after_newline=1;
	  }
	  startat = 0;
	}
      }
      break;
    default:
      fprintf(stderr, "Bad case in pvs_parse_prove.\n");
      startat=0;
      break;
    }
  }
  *len=0;
  return 0;
}


static void pvs_start_proof(PVSProof *proof)
{
  if (proof) {
    PVSProof **np;
    np = &toproof;
    while (*np) np = &(*np)->next;
    *np=proof;
  }
  if (!pvs_in_checker && !pvs_before_realtime && pvs_after_init
      && !curstep && toproof) {
    /* send message to pvs to check the given lemma */
    char buffer[2000];
    sprintf(buffer, "(prove-file-at \"%s\" %i nil \"pvs\" \"%s\" 0 nil nil)\n",
	    toproof->theory, toproof->linenr, toproof->file);    
    edit_string_to_proces(LocaletoUstr((unsigned char*)buffer),
			  translate("PVS-shell"));
    curstep=toproof->step;
  }
}

static KeywordItem *get_pvskeyword(Char *hintstr)
{
  KeywordItem *kwi= kwlist;
  while (kwi) {
    if (!Ustrncmp(hintstr, kwi->str, kwi->len))  return kwi;
    kwi=kwi->next;
  }
  return 0;
}

/* create temporary file with PVS header */
PVSProof *pvs_temporary_file(int nr, FILE **f)
{
  char buffer[1024];
  char *head;
  int clnr;
  PVSProof *pvsproof;
  sprintf(buffer, "%s/%s%i.pvs", UstrtoFilename(pvs_context_dir),
	  UstrtoFilename(pvs_hint_file), nr);
  *f=fopen(buffer, "wb");
  if (!*f) {
    message2(MP_ERROR, translate("Unable to create temporary PVS file "),
	     LocaletoUstr((unsigned char*)buffer));
    return NULL;
  }
  pvsproof = malloc(sizeof(PVSProof));
  if (!pvsproof) { return NULL; }
  pvsproof->file = strdup(buffer);
  {
    char *theoryname;
    theoryname = strrchr(buffer, '.');
    *theoryname=0;
    theoryname = strrchr(buffer,'/');
    theoryname++;
    pvsproof->theory = strdup(theoryname);
  }
  head = (char*)UstrtoLocale(translate("PVS_HEADER"));
  fprintf(*f, "%s%s\n", pvsproof->theory, head);
  clnr=0;
  while (*head) {
    if (*head=='\n') clnr++;
    head++;
  }
  clnr++;
  pvsproof->linenr = clnr;
  pvsproof->lemma=0;
  pvsproof->step=0;
  pvsproof->next=0;
  return pvsproof;
}
  
void pvs_construct_hint(int selection, int forward, Char *hint)
{
  PVSProof *p;
  FILE *f;
  Char *hintstr;
  ProofStep **cstep;
  KeywordItem *kwi;
  p = pvs_temporary_file(lasthintfile,&f);
  copy_selection(&p->selection, get_selection(selection));
  {
    char lemmabuf[500];
    sprintf(lemmabuf, "%s%i", UstrtoLocale(pvs_lemma_name), lastlemmanr);
    lastlemmanr++;
    p->lemma = strdup(lemmabuf);
    fprintf(f,"%s: THEOREM\n\t\t", lemmabuf);
  }
  cstep = &p->step;
  /* add an empty proof step to store the starting expression */
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=0;
  (*cstep)->pvsinput=0;
  (*cstep)->comment=translate("starting formula");
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->steptype=SkipStep;
  (*cstep)->use_result=0;
  cstep = &((*cstep)->next);  
  /* add the initial proof step to start the hints */
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=get_pvskeyword(translate("INITSTEP"));
  (*cstep)->pvsinput=0;
  (*cstep)->comment=translate("initialisation steps");
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->use_result=0;
  (*cstep)->steptype=InitStep;
  cstep = &((*cstep)->next);
  kwi = get_pvskeyword(hint);
  if (kwi) {
    if (kwi->induct) {
      void *node;
      char *str=0;
      char *h;
      void *upsel;
      upsel=0;
      copy_selection(&upsel, p->selection);
      up_selection(upsel);
      tex_set_string(&str);
      tex_placeholders(0);
      latex_all_parens(MP_True);
      tex_mode(ASCII);
      node = first_node(upsel);
      latex_node(node);
      out_latex_char('=');
      node = last_node(upsel);
      latex_node(node);
      tex_unset();
      latex_all_parens(MP_False);
      destruct_selection(upsel);
      /*
      ** Filter out '+(1)' and '+1'.
      ** This should be adjusted for allow induction on other types
      */
      h=str;
      while (h && ((h=strstr(h,"+(1)")))) {
	int i;
	for (i=0; i<4;i++) h[i]=' ';
	h=h+4;
      }
      h=str;
      while (h && ((h=strstr(h,"+1")))) {
	int i;
	for (i=0; i<2;i++) h[i]=' ';
	h=h+2;
      }
      fprintf(f, "(%s) IMPLIES\n",str);
      free(str);
    }
    *cstep = malloc(sizeof(ProofStep));
    (*cstep)->next=0;
    (*cstep)->keyword=kwi;
    (*cstep)->pvsinput=0;
    (*cstep)->comment=0;
    (*cstep)->result=0;
    (*cstep)->failed=0;
    (*cstep)->steptype=KeywordStep;
    (*cstep)->use_result=0;
    cstep = &((*cstep)->next);
  }
  {
    char *str=0;
    tex_set_string(&str);
    tex_placeholders(0);
    latex_all_parens(MP_True);
    tex_mode(ASCII);
    latex_selection(selection);
    tex_unset();
    latex_all_parens(MP_False);
    if (using_booleans) {
      fprintf(f,"%s\n\n", str);
    } else {
      fprintf(f, "(%s) = Tresult\n\n", str);
    }
  }
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=get_pvskeyword(translate("RESULTSTEP"));
  (*cstep)->pvsinput=0;
  (*cstep)->comment=0;
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->steptype=FinishStep;
  (*cstep)->use_result=(forward? 2:1);
  cstep = &((*cstep)->next);
  fprintf(f, "\nEND %s\n", p->theory);
  fclose(f);
  lasthintfile++;
  pvs_start_proof(p);
}

void pvs_check_hint(int selection)
{
  int nr,vnr;
  unsigned long uvnr=0;
  Char *hintstr;
  PVSProof *pvsproof;
  ProofStep **cstep;
  char *head;
  int assumptions=0;
  int np=0;
  int curlinenr=0;
  FILE *f;
  char buffer[1024];
  /* check if hint is selected */
  nr = selected_notation(selection, &vnr);
  if (nr>=0) uvnr = which_notation(nnr_vnr2innr(nr,0))->vers[vnr].vnr;
  if (nr<0 || uvnr<firsthintnr || uvnr> lasthintnr) {
    message(MP_ERROR, translate("No valid hint selected"));
    return;
  }
  pvsproof=pvs_temporary_file(lasthintfile, &f);
  {
    int pos[2];
    copy_selection(&pvsproof->selection, get_selection(selection));
    pos[0]=1;pos[1]=0;
    change_selection(pvsproof->selection, pos,2);
  }
  {
    char lemmabuf[500];
    sprintf(lemmabuf, "%s%i", UstrtoLocale(pvs_lemma_name), lastlemmanr);
    lastlemmanr++;
    pvsproof->lemma = strdup(lemmabuf);
    fprintf(f,"%s: THEOREM\n\t\t", lemmabuf);
    curlinenr++;
  }
  assumptions=0;
  cstep = &pvsproof->step;
  /* add an empty proof step to store the starting expression */
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=0;
  (*cstep)->pvsinput=0;
  (*cstep)->comment=translate("starting formula");
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->steptype=SkipStep;
  (*cstep)->use_result=0;
  cstep = &((*cstep)->next);  
  /* add the initial proof step to start the hints */
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=get_pvskeyword(translate("INITSTEP"));
  (*cstep)->pvsinput=0;
  (*cstep)->comment=translate("initialisation steps");
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->use_result=0;
  (*cstep)->steptype=InitStep;
  cstep = &((*cstep)->next);
  /* parse text in hint:
  ** - expression   ->  add to assumptions
  ** - "induction"  ->  add 'Ef = El' to assumptions
  **                    (Ef: first expr., El: last expr,  =: weakest operator)
  **                    add (inst?)(ground)(try-triv-step (replace*)) to proof
  ** - "name"       ->  add (modulo-assoc (bidi-rewrite "name")) to proof
  ** - "definition" ->  add (expand-simp* "def1" ... "defn") to proof
  **                    (def1 ... defn extracted from expressions)
  ** - hidden PVS   ->  add content to proof
  */
  {
    int pos[2];
    pos[0]=1;
    pos[1]=0;
    hintstr = get_subnode_string(selection, pos,2);
  }
  if (!hintstr || !hintstr[0]) {
    /* empty hints usually indicate trivial steps */
    hintstr = translate("trivial");
  }
  while (*hintstr) {
    if (IsPh(hintstr[0])) {
      /* An expression, identifier, operator of text */
      void *node;
      int pos[3];
      char *str=0;
      pos[0]=1;
      pos[1]=0;
      pos[2]=np++;
      node = get_subnode(selection, pos, 3);
      nr = node_notation(node, &vnr);
      if (nr!= -1) uvnr = which_notation(nnr_vnr2innr(nr,0))->vers[0].vnr;
      tex_set_string(&str);
      tex_placeholders(0);
      latex_all_parens(MP_True);
      tex_mode(ASCII);
      latex_node(node);
      tex_unset();
      latex_all_parens(MP_False);
      if (str && str[0]) {
	/* the expression produces output */
	if (uvnr==pvshiddenstep) {
	  /* a special PVS related template to add steps to the proof list */
	  *cstep = malloc(sizeof(ProofStep));
	  (*cstep)->next=0;
	  (*cstep)->keyword=0;
	  (*cstep)->pvsinput=str;
	  (*cstep)->comment=translate("hidden proof steps");
	  (*cstep)->result=0;
	  (*cstep)->failed=0;
	  (*cstep)->use_result=0;
	  (*cstep)->steptype=HiddenStep;
	  cstep = &((*cstep)->next);
	} else {
	  /* A normal expression. For now, an assumption, but it could be
	  ** an identifier  (as in "definition of $fold$") or 
	  ** a single operator (as in "$\times$ distributes over $\plus$")
	  */
	  if (!assumptions) {
	    fprintf(f,"(\t");
	  } else {
	    fprintf(f,"\t\t    AND ");
	  }
	  fprintf(f, "%s\n",str);
	  curlinenr++;
	  {
	    char *h;
	    for (h=str; *h; h++) { if (*h=='\n') curlinenr++; }
	  }
	  free(str);
	  assumptions++;
	  *cstep = malloc(sizeof(ProofStep));
	  (*cstep)->next=0;
	  (*cstep)->keyword=get_pvskeyword(translate("EXPRESSIONSTEP"));
	  (*cstep)->pvsinput=0;
	  (*cstep)->comment=translate("assumption step");
	  (*cstep)->result=0;
	  (*cstep)->failed=0;
	  (*cstep)->use_result=0;
	  (*cstep)->steptype=ExpressionStep;
	  cstep = &((*cstep)->next);
	}
      }
      hintstr++;
    } else {
      KeywordItem *kwi= kwlist;
      kwi = get_pvskeyword(hintstr);
      if (kwi) {
	if (kwi->induct) {
	  void *node;
	  char *str=0;
	  char *h;
	  tex_set_string(&str);
	  tex_placeholders(0);
	  latex_all_parens(MP_True);
	  tex_mode(ASCII);
	  node = first_node(get_selection(selection));
	  latex_node(node);
	  out_latex_char('=');
	  node = last_node(get_selection(selection));
	  latex_node(node);
	  tex_unset();
	  latex_all_parens(MP_False);
	  /*
	  ** Filter out '+(1)' and '+1'.
	  ** This should be adjusted for allow induction on other types
	  */
	  h=str;
	  while (h && ((h=strstr(h,"+(1)")))) {
	    int i;
	    for (i=0; i<4;i++) h[i]=' ';
	    h=h+4;
	  }
	  h=str;
	  while (h && ((h=strstr(h,"+1")))) {
	    int i;
	    for (i=0; i<2;i++) h[i]=' ';
	    h=h+2;
	  }
	  if (!assumptions) {
	    fprintf(f,"(\t");
	  } else {
	    fprintf(f,"\t\t    AND ");
	  }
	  fprintf(f, "%s\n",str);
	  curlinenr++;
	  {
	    char *nlc;
	    for (nlc=str; *nlc; nlc++) { if (*nlc=='\n') curlinenr++; }
	  }
	  free(str);
	  assumptions++;
	}
	if (kwi->step && kwi->step[0]) {
	  *cstep = malloc(sizeof(ProofStep));
	  (*cstep)->next=0;
	  (*cstep)->keyword=kwi;
	  (*cstep)->pvsinput=0;
	  (*cstep)->comment=0;
	  (*cstep)->result=0;
	  (*cstep)->failed=0;
	  (*cstep)->use_result=0;
	  (*cstep)->steptype=KeywordStep;
	  cstep = &((*cstep)->next);
	}
	hintstr=hintstr+kwi->len;
      }
      /* might want to skip to the next place holder/word boundary */
      hintstr++;
    }
  }
  if (assumptions) {
    fprintf(f,"\t\t) IMPLIES\n\t\t\t");
    curlinenr++;
  }
  /* generate lemma */
  {
    char *str=0;
    tex_set_string(&str);
    tex_placeholders(0);
    latex_all_parens(MP_True);
    tex_mode(ASCII);
    latex_selection(selection);
    tex_unset();
    latex_all_parens(MP_False);
    {
      char *nlc;
      for (nlc=str; *nlc; nlc++) if (*nlc=='\n') curlinenr++;
    }
    fprintf(f, "%s\n\n", str);
    curlinenr += 2;
  }
  *cstep = malloc(sizeof(ProofStep));
  (*cstep)->next=0;
  (*cstep)->keyword=get_pvskeyword(translate("FINISHSTEP"));
  (*cstep)->pvsinput=0;
  (*cstep)->comment=0;
  (*cstep)->result=0;
  (*cstep)->failed=0;
  (*cstep)->use_result=0;
  (*cstep)->steptype=FinishStep;
  cstep = &((*cstep)->next);
  fprintf(f, "\nEND %s\n", pvsproof->theory);
  fclose(f);
  lasthintfile++;
  /* start PVS proof of temporary lemma */
  pvs_start_proof(pvsproof);
  /* pvs_start_proof starts the proof of the selected hint(s)
  ** and checks if each step is correct, by sending the step,
  ** parse the output, check for errors, send next step, etc.
  ** At the end, Q.E.D. should appear and the 
  ** delivery.
  */
  if (pvsproof) {
    ProofStep *ps;
    for (ps=pvsproof->step; ps; ps=ps->next) {
      if (ps->pvsinput) {
	string_to_window(translate("PVS Generated Proof"),
			 LocaletoUstr((unsigned char*)ps->pvsinput));
      } else if (ps->keyword && ps->keyword->step) {
	string_to_window(translate("PVS Generated Proof"),
			 LocaletoUstr((unsigned char*)ps->keyword->step));
      }
    }
  }
  /* set PVS parse function correct */
}

static Type tlist[4][4] = {
  { IntType, 0},
  { StringType, 0},
  { StringType, StringType, IntType, 0 },
  { IntType, IntType, StringType,0}
};

static int call_int(int (*fcal)(), void **args)
{
  return (*fcal)(*((int*)args[0]));
}

static int call_str(int (*fcal)(), void **args)
{
  return (*fcal)(*((Uchar**)args[0]));
}

static int call_strstrint(int (*fcal)(), void **args)
{
  return (*fcal)(*((Uchar**)args[0]),*((Uchar**)args[1]),*((int*)args[2]));
}

static int call_intintstr(int (*fcal)(), void **args)
{
  return (*fcal)(*((int*)args[0]),*((int*)args[1]),*((Uchar**)args[2]));
}

int init_library(void)
{
  Prototype *pt;

  pt = define_prototype(tlist[0],1, 0, call_int);
  define_function("pvs_check_hint", "Check the selected hint with PVS.",
		  pt, pvs_check_hint);
  pt = define_prototype(tlist[3],3,0,call_intintstr);
  define_function("pvs_construct_hint", "Construct a hint with PVS.",
		  pt, pvs_construct_hint);
  pt = define_prototype(tlist[1],1, 0, call_str);
  define_function("pvs_start", "Start the PVS system.",
		  pt, pvs_start);
  pt = define_prototype(tlist[2],3, 0, call_strstrint);
  define_function("pvs_add_keyword",
	      "Add a keyword to the list of keywords recognized in a hint.",
		  pt, pvs_add_keyword);
  define_program_variable(IntType, "pvs_initialized", &pvs_after_init);
  define_program_variable(IntType, "pvs_in_checker", &pvs_in_checker);
  define_program_variable(IntType, "pvs_before_realtime",
			  &pvs_before_realtime);
  define_program_variable(IntType, "pvs_do_leibnitz", &pvs_do_leibnitz);
  define_program_variable(IntType, "pvs_on_booleans", &using_booleans);
  define_program_variable(StringType, "pvs_context_dir", &pvs_context_dir);
  define_program_variable(StringType, "pvs_hint_file", &pvs_hint_file);
  define_program_variable(StringType, "pvs_lemma_name", &pvs_lemma_name);
  define_program_variable(StringType, "pvs_stencil_file", &pvs_stencil_file);
  return 1;
}

