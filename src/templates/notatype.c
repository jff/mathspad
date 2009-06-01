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
/*
** File: notatype.c
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "intstack.h"
#include "notatype.h"
#include "message.h"
#include "fileread.h"
#include "output.h"
#include <memory.h>
#include <time.h>

#include "unistring.h"
#include "translate.h"

#define BLOCKSIZE 32
#define BUFSIZE 2048

int edit_fnr=-1;
int use_file_nr=-1;

/*
** Need flexible arrays to enable lots of files, notations etc.
**
** A general type FlexArray and the functions fx_.... are available
** to make all kind of arrays. The defined macros call the correct
** fx_ function and the needed type conversions are made.
*/

static int nf_cmp(const void* a __attribute__((unused)), const void* b __attribute__((unused))) { return 1; }

#define nf_contains(B) fx_contains(&notationfile, (void*) &(B))
#define nf_remove(B)   fx_remove(&notationfile, (void*) &(B))
#define nf_add(B)      fx_add(&notationfile,(void*) &(B))
#define nf_switch(B,C) fx_switch(&notationfile, (void*) &(B), (void*) &(C))
#define nf_init()      fx_init(&notationfile, sizeof(NOTATIONFILE), nf_cmp)
#define nf_clear()     fx_clear(&notationfile)
#define nf_set(B,C)    fx_set(&notationfile, (B), &(C))
#define nf_item(B)     (*((NOTATIONFILE*) fx_item(&notationfile,B))) 
#define nf_max         fx_max(&notationfile)

static FlexArray notationfile;


static int nt_cmp(const void* a __attribute__((unused)), const void* b __attribute__((unused))) { return 1; }

#define nt_contains(B) fx_contains(&allnotas, (void*) &(B))
#define nt_remove(B)   fx_remove(&allnotas, (void*) &(B))
#define nt_add(B)      fx_add(&allnotas,(void*) &(B))
#define nt_switch(B,C) fx_switch(&allnotas, (void*) &(B), (void*) &(C))
#define nt_init()      fx_init(&allnotas, sizeof(NOTATION), nt_cmp)
#define nt_clear()     fx_clear(&allnotas)
#define nt_set(B,C)    fx_set(&allnotas, (B), &(C))
#define nt_item(B)     (*((NOTATION*) fx_item(&allnotas,B))) 
#define nt_max         fx_max(&allnotas)

static FlexArray allnotas;

static int formatnr = 0;

static void (*ifchanged[5])(void);
static int nr_changed=0;

static INTSTACK *hash_number_vnr[256];
static INTSTACK *hash_number_nnr[256];
static INTSTACK *hash_string[256];
static FlexArray savelist;

typedef
struct {
    int nnr;
    int vnr;
} LOOKUPITEM;

static FlexArray lookuptable;

static int lu_cmp(const void* a __attribute__((unused)), const void* b __attribute__((unused))) { return 1; }

#define lu_contains(B) fx_contains(&lookuptable, (void*) &(B))
#define lu_remove(B)   fx_remove(&lookuptable, (void*) &(B))
#define lu_add(B)      fx_add(&lookuptable,(void*) &(B))
#define lu_switch(B,C) fx_switch(&lookuptable, (void*) &(B), (void*) &(C))
#define lu_init()      fx_init(&lookuptable, sizeof(LOOKUPITEM), lu_cmp)
#define lu_clear()     fx_clear(&lookuptable)
#define lu_set(B,C)    fx_set(&lookuptable, (B), &(C))
#define lu_item(B)     (*((LOOKUPITEM*) fx_item(&lookuptable,B))) 
#define lu_max         fx_max(&lookuptable)


static Char buffer[BUFSIZE];
static unsigned long shifted_uid=0;
static unsigned long last_used=0;

static char *kinddesc[MAX_KIND] =
 { "Kind:None", "Kind:Prefix", "Kind:Postfix",
   "Kind:Infix", "Kind:Left", "Kind:Right" };

Char *kind_description(int kind)
{
  if (kind<0 || kind>=MAX_KIND) return 0;
  else return translate(kinddesc[kind]);
}


void set_change_function( void (*cfunc)(void))
{
    ifchanged[nr_changed++] = cfunc;
}
     
void changed_notation(void)
{
    int i;
    for (i=0; i<nr_changed; i++) (*ifchanged[i])();
}

unsigned long new_number(void)
{
    unsigned long nn=time(NULL);
    if (nn<=last_used) nn = last_used+1;
    last_used=nn;
    return nn^shifted_uid;
}

void notatype_init(void)
{
    int i;

    shifted_uid = user_id*2+1;
    while (shifted_uid &&
	   !(shifted_uid&(((unsigned long)1)<<(sizeof(long)*8-1))))
	shifted_uid = shifted_uid<<1;
    nt_init();
    nf_init();
    lu_init();
    int_init(savelist);
    for (i=0; i<256; i++)
	hash_number_vnr[i] = hash_number_nnr[i] = hash_string[i] = NULL;
    new_notation_window();
    rename_notation_window(0,concat(translate("*backup*"),NULL));
    nf_item(0).nr_windows=0;
}

/* This hash function is used to load new templates (from stencil files
** or documents).
*/

#define hash_unique_number(A) ((int) ((A)&0xff))


/* This hash function is used to load old documents and stencil files.
** The screen format was used in every node, which is hopefully solved
** with the new format. The function should not be called too often with
** long strings because the complete string will be used.
*/
static int hash_screen_format(Char *c, int l)
{
    int i=0,j=0,k;
    if (!c) return 0;
    while (l && *c) {
	k =  ((*c)&0xff)<<j;
	i += (k&0xff) + (k>>8);
	if (j<7) j++; else {j=0;i=i&0xff;}
	c++;l--;
    }
    return i&0xff;
}

#define lu_hi(A)  lu_item(head_int(A))
#define nt_lu_hi(A) nt_item(lu_hi(A).nnr)
#define nt_lu_hi_vers(A) nt_lu_hi(A).vers[lu_hi(A).vnr]

static int hash_to_pos_vnr(INTSTACK *h, unsigned long uvnr)
{
    while (h && (nt_lu_hi_vers(h).vnr != uvnr))
	h = tail_stack(h);
    if (h)
	return head_int(h);
    else
	return -1;
}


static int hash_to_pos_nnr(INTSTACK *h, int unnr)
{
    while (h && (int) nt_item(head_int(h)).nnr != unnr)
	h = tail_stack(h);
    if (h)
	return head_int(h);
    else
	return -1;
}

/*
** Functions for reading from a file and writing to a file
*/

/* functions for automatic convertion of old stencil files */
static Bool old_get_char(Char *c, FILE *f)
{
    int fontnr, charnr;

    *c=0;
    if ((fontnr = fgetc(f))==EOF) return MP_False;
    if ((charnr = fgetc(f))==EOF) return MP_False;
    *c = (Char) Font2Char(fontnr, charnr);
    return MP_True;
}

static void make_index(VERSION *vers)
{
    Char *c, *d;
    int n,l;

    c=vers->format[SCREENFORMAT];
    d=vers->format[SCREENFORMAT];
    n=0;
    l=0;
    if (c)
	while (*c) {
	    if (IsPh(*c)) n++;
	    if (IsNewline(*c)) l++;
	    if (Char2Font(*c)==StackFont) *d++=StackC;
	    else if (*c>=CloseGap && *c<=CloseStack)
		switch (*c) {
		case OpenTop:    case OpenGap:
		case OpenBottom: case CloseBottom: break;
		case CloseTop:   *d++=TopGap;      break;
		case CloseGap:   *d++=GapBottom;   break;
		case CloseStack: *d++=StackClose;  break;
		default: *d++=*c;
		}
	    else *d++=*c;
	    c++;
	}
    if (c!=d) {
	vers->max[SCREENFORMAT]=d-vers->format[SCREENFORMAT];
	*d=0;
    }
    vers->nr_plhd = n;
    vers->lines = l;
    if (n) vers->phindex = (int*) malloc(sizeof(int)*n);
    else vers->phindex=NULL;
    if (vers->phindex) {
	int i=0,j=0;
	c=vers->format[SCREENFORMAT];
	while (*c && i<n) {
	    if (IsPh(*c)) vers->phindex[i++] = *c + (j<<16);
	    c++;j++;
	}
    } else
	vers->nr_plhd = 0;
}

static Bool old_get_version(VERSION *vers, FILE *f)
{
    int i,j,id[2];
    Char l[2], c;
    Char *s;
    Bool well;

    if (!(well = ((i = fgetc(f))!=EOF))) return MP_False;
    if (formatnr) {
	if (i) {
	    vers->format[NAMEFORMAT] = (Char*) malloc(sizeof(Char) * (i+1));
	    if (!vers->format[NAMEFORMAT]) return MP_False;
	    l[0] = (Char)i; i=0;
	    while (i< (int)l[0] && (well &= old_get_char(&c,f))) {
		vers->format[NAMEFORMAT][i] = c;
		i++;
	    }
	    vers->format[NAMEFORMAT][l[0]] = 0;
	    vers->max[NAMEFORMAT] = l[0];
	} else {
	    vers->format[NAMEFORMAT] = NULL;
	    vers->max[NAMEFORMAT] = 0;
	}
	well &= ((i = fgetc(f))!=EOF);
	well &= old_get_char(&l[1], f);
	well &= old_get_char(&l[0], f);
	l[1] -= l[0];
	c=(Char)i;
    } else {
	c=(Char)i;
	vers->format[NAMEFORMAT]=NULL;
	vers->max[NAMEFORMAT]=0;
	l[0]=EDIT_SIZE;
	if (i) 
	    l[1]=0;
	else
	    l[1]=EDIT_SIZE;
    }
    well &= ((vers->format[SCREENFORMAT] =
	      (Char*) malloc(sizeof(Char)*(l[0]+1)))!=NULL);
    if (c)
	vers->format[LATEXFORMAT] = NULL;
    else
	well &= ((vers->format[LATEXFORMAT] =
		  (Char*) malloc(sizeof(Char)*(l[1]+1)))!=NULL);
    id[0]=SCREENFORMAT;id[1]=LATEXFORMAT;
    for (j=0; j<2; j++) {
	i=0;
	if (aig(s=vers->format[id[j]])) {
	    while (i< (int)l[j] && (well &= old_get_char(s, f)) && *s)
		i++,s++;
	    *s=0;
	    if (i<(int)l[j])
		vers->format[id[j]] = (Char*) realloc(vers->format[id[j]],i+1);
	}
	vers->max[id[j]]=i;
    }
    if (!well) {
	for (i=0; i<MAXFORMAT; i++)
	    free(vers->format[i]);
    } else {
	if ((vers->max[LATEXFORMAT] && vers->format[LATEXFORMAT][0]==AskText)
	    || (vers->max[0] && vers->format[0][0]==AskText))
	    vers->latexmode = LTEXTMODE;
	else vers->latexmode = LMATHMODE;
	vers->vnr = new_number();
	make_index(vers);
    }
    return well;
}

static Bool old_get_notation(NOTATION *nota, FILE *f)
{
    int i, len, d, nd=0;
    Char c;
    Bool well = MP_True;

    nota->name = NULL;
    nota->helpfilename = NULL;
    nota->vers = NULL;
    nota->locks=0;
    nota->fillocks=0;
    if (!formatnr) {
	well &= old_get_char(&c,f);
	well &= ((nd=fgetc(f))!=EOF);
	nd=nd>1;
    }
    well &= ((len = fgetc(f)) != EOF);
    if (len) {
	well &= ((nota->name = (Char *) malloc(sizeof(Char)*(len+1)))!=NULL);
	i=0;
	while (i<len && (well &= ((d=fgetc(f))!=EOF))) {
	    nota->name[i] = (Char)d;
	    i++;
	}
	if (nota->name) nota->name[len]=0;
    } else
	nota->name = NULL;
    well &= ((nota->prec = fgetc(f))!=EOF);
    well &= (((i = fgetc(f)))!=EOF) ; nota->kind=(Opkind) i;
    if (!formatnr) {
	nota->space = i/8;
	nota->kind = (Opkind) (i%8);
	well &= ((nota->versions = fgetc(f))!=EOF);
	well &= old_get_char(&c,f);
    } else {
	well &= ((nota->space = fgetc(f))!=EOF);
	well &= ((nota->versions = fgetc(f))!=EOF);
    }
    well &= ((nota->vers =
	     (VERSION *) malloc(nota->versions * sizeof(VERSION)))!=NULL);
    i=0;
    while (i<nota->versions && (well &= old_get_version(nota->vers+i, f)))
	i++;
    well &= (i==nota->versions);
    if (!well) {
	destroy_version(nota->vers, i);
	free(nota->name);
    } else {
	if (formatnr==1 && nota->vers[0].format[NAMEFORMAT] &&
	    !Ustrcmp(nota->name, nota->vers[0].format[NAMEFORMAT])) {
	    free(nota->vers[0].format[NAMEFORMAT]);
	    nota->vers[0].format[NAMEFORMAT]=0;
	    nota->vers[0].max[NAMEFORMAT]=0;
	} else if ((!formatnr && !nd) ||
		   (formatnr==1 && !nota->vers[0].format[NAMEFORMAT])) {
	    free(nota->name);
	    nota->name=NULL;
	}
	nota->nnr = new_number();
    }
    return well;
}


/* functions for reading and writing new stencils */
typedef struct VERSIONLIST VERSIONLIST;
struct VERSIONLIST {
    VERSION vers;
    VERSIONLIST *next;
};
static VERSIONLIST *versionstack=NULL, *vfreel=NULL;

static void put_version(VERSION *vers)
{
    int i,j,mask;
    Char b[2];

    b[0]=1;b[1]=0;
    push_hidden();
    put_struct(VERSIONTYPE,6);
    put_integer(vers->vnr);
    mask=1;
    i=0;j=0;
    while (i<MAXFORMAT) {
	if (vers->format[i]) j=j|mask;
	i++;
	if (MAXFORMAT>5 && i==5) mask=mask<<2; else mask=mask<<1;
    }
    put_Char((Char)(j|32));
    put_Char((Char)(vers->latexmode+32));
    pop_hidden();
    for (i=0; i<MAXFORMAT; i++) {
	if (vers->format[i])
	    put_String(vers->format[i], vers->max[i]);
    }
    push_hidden();
    put_end_struct();
    pop_hidden();
}

int get_version(Char *s, int *len __attribute__((unused)), int max)
{
    int bad=MP_False;
    VERSIONLIST *h = vfreel;
    if (h)
	vfreel=h->next;
    else
	h = (VERSIONLIST*) malloc(sizeof(VERSIONLIST));
    if (h) {
	int i,m,mask,sel=(1<<(MAXFORMAT-1+(MAXFORMAT>5)));
	h->vers.vnr = (s[0]<<16)+s[1];
	h->vers.latexmode = (max==6 ? s[3]-32 : LDEFMODE);
	if (s[2]&32) mask = s[2]; else mask= 7>>(3-(s[2]&63));
	for (i=0; i<MAXFORMAT; i++) {
	    h->vers.format[i]=NULL;
	    h->vers.max[i]=0;
	}
	for (i=MAXFORMAT-1; sel && !bad; i--) {
	    if (sel&mask) {
		if (!get_String(&(h->vers.format[i]), &(h->vers.max[i]), &m)) {
		    bad=MP_True;
		    h->vers.format[i]=NULL;
		    h->vers.max[i]=0;
		} else if (i!=SCREENFORMAT) {
		    if ((h->vers.max[i]==2 && h->vers.format[i][0]==1 &&
			 !(h->vers.format[i][1])) ||
			(h->vers.max[i]==1 && !(h->vers.format[i][0]))) {
			free(h->vers.format[i]);
			h->vers.format[i]=NULL;
			h->vers.max[i]=0;
		    }
		}
	    }
	    if (MAXFORMAT>5 && i==5) sel=sel>>2; else sel=sel>>1;
	}
	if (bad) {
	    for (i=0; i<MAXFORMAT; i++) {
		free(h->vers.format[i]);
		h->vers.format[i]=NULL;
		h->vers.max[i]=0;
	    }
	    h->next = vfreel;
	    vfreel=h;
	} else {
	    for (i=1; i<MAXFORMAT; i++) {
		if (h->vers.format[i] && h->vers.max[i]==h->vers.max[0]) {
		    m=0;
		    while (m<h->vers.max[i] &&
			   h->vers.format[i][m]==h->vers.format[0][m]) m++;
		    if (m==h->vers.max[i]) {
			free(h->vers.format[i]);
			h->vers.format[i]=NULL;
			h->vers.max[i]=0;
		    }
		}
	    }
	    if (h->vers.latexmode==LDEFMODE) {
		Char *c;
		if (h->vers.max[LATEXFORMAT]) c = h->vers.format[LATEXFORMAT];
		else c = h->vers.format[0];
		if (*c == AskText) h->vers.latexmode=LTEXTMODE;
		else h->vers.latexmode=LMATHMODE;
	    }
	    h->next=versionstack;
	    versionstack=h;
	    make_index(&(h->vers));
	}
    } else
	bad=MP_True;
    return (bad? FAILURE : SUCCESS+FREE_BUFFER);
}

static void put_stencil(NOTATION *nota)
{
    int i;

    push_hidden();
    put_struct(STENCILTYPE,8);
    put_integer(nota->nnr);
    put_char((char)(((int) nota->kind) +32));
    put_char((char)(nota->space+32));
    put_char((char)(nota->prec+32));
    put_char((char)(nota->versions+32));
    i=2;
    if (!nota->helpfilename) {
	i--;
	if (!nota->name) i--;
    }
    put_char((char)('0'+i)); /* i strings follow */
    pop_hidden();
    if (i>=1) {
	if (nota->name) put_String(nota->name, Ustrlen(nota->name));
	else put_String(NULL,0);
    }
    if (i>=2) {
	if (nota->helpfilename)
	    put_String(nota->helpfilename, Ustrlen(nota->helpfilename));
	else put_String(NULL,0);
    }
    for (i=0; i<nota->versions; i++)
	put_version(nota->vers+i);
    for (i=nota->versions-1; i>=0; i--)
	int_add(savelist, nota->vers[i].ivnr);
    push_hidden();
    put_end_struct();
    pop_hidden();
}

static void add_notation_ref(int nnr);

int get_stencil(Char *s,
		int *len __attribute__((unused)),
		int max __attribute__((unused)))
{
    int i,l,m;
    Char *c;
    int oldnr;
    NOTATION nota;

    nota.nnr = (s[0]<<16)+s[1];
    oldnr=hash_unique_number(nota.nnr);
    oldnr=hash_to_pos_nnr(hash_number_nnr[oldnr], nota.nnr);
    if (oldnr>=0 && nt_item(oldnr).fillocks) {
	VERSIONLIST *h;
	NOTATION *ntp = &nt_item(oldnr);
	i=s[6]-'0';
	while (i) {
	    get_string(&c,&l,&m);
	    free(c);
	    i--;
	}
	i=s[5]-33;
	while (i>=0 && versionstack) {
	    h=versionstack;
	    l=0;
	    while (l<ntp->versions && ntp->vers[l].vnr!=h->vers.vnr) l++;
	    if (l==ntp->versions) l=0;
	    int_add(savelist, ntp->vers[l].ivnr);
	    versionstack=h->next;
	    h->next=vfreel;
	    vfreel=h;
	    for (l=0; l<MAXFORMAT; l++) free(h->vers.format[l]);
	    i--;
	}
	add_notation_ref(oldnr);
	return SUCCESS+FREE_BUFFER;
    }
    nota.locks = nota.fillocks=0;
    nota.kind = (Opkind) (s[2]-32);
    nota.space = s[3]-32;
    nota.prec = s[4]-32;
    nota.versions= s[5]-32;
    nota.helpfilename=nota.name=NULL;
    i=s[6]-'0';
    while (i) {
	c=NULL;
	get_string(&c,&l,&m);
	if (c && !*c) {
	    free(c);
	    c=NULL;
	}
	switch (i) {
	case 1:
	    nota.name=c;
	    break;
	case 2:
	    nota.helpfilename=c;
	    break;
	default:
	    free(c);
	    break;
	}
	i--;
    }
    nota.vers = (VERSION *) malloc(sizeof(VERSION)*nota.versions);
    if (nota.vers) {
	VERSIONLIST *h;
	i=nota.versions-1;
	while (i>=0 && versionstack) {
	    nota.vers[i]= versionstack->vers;
	    h=versionstack->next;
	    versionstack->next = vfreel;
	    vfreel = versionstack;
	    versionstack=h;
	    i--;
	}
    }
    add_notation(oldnr,&nota);
    for (i=nota.versions-1; i>=0; i--)
	int_add(savelist, nota.vers[i].ivnr);
    return SUCCESS+FREE_BUFFER;
}

void  add_version(VERSION **list, int place, int *max)
{
    VERSION *newlist;
    int i=0 , delta=0;

    newlist = (VERSION*) malloc( sizeof(VERSION) * (*max+1));
    for (; i< *max; i++) {
	if (i==place) delta=1;
	newlist[i+delta] = (*list)[i];
    }
    for (i=0; i<MAXFORMAT; i++) {
	newlist[place].format[i]=NULL;
	newlist[place].max[i]=0;
    }
    newlist[place].phindex=NULL;
    newlist[place].latexmode= LDEFMODE;
    newlist[place].vnr = new_number();
    make_size_version(&newlist[place], NAMEFORMAT, 1);
    make_size_version(&newlist[place], SCREENFORMAT, 1);
    (*max)++;
    free(*list);
    *list = newlist;
}

void  remove_version(VERSION **list, int place, int *max)
{
    if (*max <=1) {
	if (*list) {
	    destroy_version(*list, *max);
	    *list = NULL;
	}
	*max = 0;
    } else {
	if (place>=0 && place< *max) {
	    int i;
	    for (i=0; i<MAXFORMAT; i++)
		if ((*list)[place].format[i]) free((*list)[place].format[i]);
	    i=place;
	    (*max)--;
	    while (i<*max) {
		(*list)[i] = (*list)[i+1];
		i++;
	    }
	}
    }
}

/* expand size with BLOCKSIZE items at a time (with at least A items) */

#define NEWSIZE(A) (BLOCKSIZE*(((A)+1)/BLOCKSIZE+1))

Bool make_size_version(VERSION *vers, int fmnr, int newmax)
{
    if (newmax>= vers->max[fmnr]) {
	Char *newf;

	if (vers->format[fmnr])
	    newf = (Char *) realloc(vers->format[fmnr],
				    sizeof(Char)*NEWSIZE(newmax));
	else
	    newf = (Char *) malloc(sizeof(Char)*NEWSIZE(newmax));
	if (!newf) return MP_False;
	if (!vers->format[fmnr]) newf[0]=0;
	vers->max[fmnr] = NEWSIZE(newmax);
	vers->format[fmnr] = newf;
	return MP_True;
    } else
	return MP_True;
}

VERSION *maximize_version(VERSION *list, int max)
{
    VERSION *newlist;
    int i,j,k;

    newlist = (VERSION *) malloc(sizeof(VERSION) * max);
    for (i=0; i<max; i++) {
	newlist[i] = list[i];
	newlist[i].phindex = NULL;
	for (j=0; j<MAXFORMAT; j++) {
	    if (list[i].format[j]) {
		newlist[i].format[j] =
		    (Char*) malloc(sizeof(Char)*(list[i].max[j]+1));
		for (k=0; k<list[i].max[j]; k++)
		    newlist[i].format[j][k] = list[i].format[j][k];
		newlist[i].format[j][k]=0;
	    }
	}
    }
    return newlist;
}

void  destroy_version(VERSION *list, int max)
{
    int i,j;

    for (i=0; i<max; i++) {
	for (j=0; j<MAXFORMAT; j++)
	    if (list[i].format[j]) free(list[i].format[j]);
	free(list[i].phindex);
    }
    free(list);
}

static Bool nomem;

VERSION *minimize_version(VERSION *list, int max)
{
    VERSION *newlist;
    int i,j,k;

    newlist = (VERSION *) malloc( sizeof(VERSION) * max);
    for (i=0; i<max; i++) {
	newlist[i]=list[i];
	for (j=0; j<MAXFORMAT; j++) {
	    if (list[i].format[j]) {
		k = Ustrlen(list[i].format[j]);
		newlist[i].format[j] = (Char*) malloc(sizeof(Char)*(k+1));
		if (newlist[i].format[j]) {
		    k=0;
		    while (aig(newlist[i].format[j][k]=list[i].format[j][k]))
			k++;
		    newlist[i].max[j]=k;
		} else {
		    newlist[i].max[j]=0;
		    nomem=MP_True;
		}
	    }
	}
	/*  make an index on the place holders */
	make_index(newlist+i);
    }
    return newlist;
}

static void destroy_notation(NOTATION *nota)
{
    if (nota->locks>1) {
	nota->locks--;
    } else {
	int i,j;
	for (i=0; i<nota->versions; i++) {
	    j = hash_screen_format(nota->vers[i].format[SCREENFORMAT],
				   nota->vers[i].max[SCREENFORMAT]);
	    remove_int(&hash_string[j], nota->vers[i].ivnr);
	    remove_int(&hash_number_vnr[hash_unique_number(nota->vers[i].vnr)],
		       nota->vers[i].ivnr);
	}
	remove_int(&hash_number_nnr[hash_unique_number(nota->nnr)],nota->innr);
	destroy_version(nota->vers, nota->versions);
	nota->versions = 0;
	nota->nnr = 0;
	nota->vers = NULL;
	free(nota->helpfilename);
	free(nota->name);
	nota->helpfilename = nota->name = NULL;
    }
}

static int add_nota(int fnr, int onr, NOTATION *nota)
{
    int i,j,nnr;
    NOTATION *onota;
    NOTATIONFILE *nfile;

    nota->innr = nt_max;
    nt_add(*nota);
    nnr=nt_max-1;
    if (nnr != nota->innr) nt_item(nnr).innr = nota->innr = nnr;
    nfile = &nf_item(fnr);
    nfile->autosaved = MP_False;
    nfile->saved = MP_False;
    /*
    ** updating lookup table
    */
    for (i=0; i<nota->versions; i++) nota->vers[i].ivnr = -1;
    if (onr>=0) {
	LOOKUPITEM *loi;
	NOTATIONFILE *fli;
	int remap[200];
	onota = &nt_item(onr);
	for (i=0; i<onota->versions; i++) {
	    for (j=0;
		 j<nota->versions && nota->vers[j].vnr!=onota->vers[i].vnr;
		 j++);
	    if (j==nota->versions) j=0;
	    remap[i]=j;
	}
	loi = &lu_item(0);
	i=0;j=lu_max;
	while (i<j) {
	    if (loi->nnr==onr) {
		loi->nnr = nnr;
		loi->vnr = remap[loi->vnr];
		nota->vers[loi->vnr].ivnr = i;
	    }
	    i++;
	    loi++;
	}
	/*
	**  replace old occurances in files with new ones
	*/
	for (i=0; i<nf_max; i++) {
	    fli = &nf_item(i);
	    if (int_max(fli->nkind[onota->kind]) &&
		int_contains(fli->nkind[onota->kind], onr)) {
		if (nota->kind==onota->kind)
		    int_switch(fli->nkind[onota->kind],onr,nnr);
		else {
		    int_remove(fli->nkind[onota->kind],onr);
		    int_add(fli->nkind[nota->kind],nnr);
		}
	    } else if (i==fnr) {
		fli->nrnt++;
		int_add(fli->nkind[nota->kind],nnr);
		onota->locks++;
		onota->fillocks++;
	    }
	}
	nt_item(nnr).locks = nota->locks = onota->locks;
	nt_item(nnr).fillocks = nota->fillocks = onota->fillocks;
	onota->locks = onota->fillocks = 0;
	destroy_notation(onota);
    } else {
	nfile->nrnt++;
	int_add(nf_item(fnr).nkind[nota->kind],nnr);
	nt_item(nnr).locks = 1;
	nt_item(nnr).fillocks = 1;
    }
    push_int(&hash_number_nnr[hash_unique_number(nota->nnr)], nota->innr);
    for (i=0; i<nota->versions; i++) {
	if (nota->vers[i].ivnr<0) {
	    LOOKUPITEM newit;
	    newit.nnr=nnr;
	    newit.vnr=i;
	    nota->vers[i].ivnr = lu_max;
	    lu_add(newit);
	}
	j=hash_screen_format(nota->vers[i].format[SCREENFORMAT],
			     nota->vers[i].max[SCREENFORMAT]);
	push_int(&hash_string[j],nota->vers[i].ivnr);
	push_int(&hash_number_vnr[hash_unique_number(nota->vers[i].vnr)],
		 nota->vers[i].ivnr);
    }
    return nnr;
}

static Bool read_all_notations(int fnr, FILE *f)
{
    int i;
    Char nr;
    Bool good= MP_True;
    NOTATION *nota;

    i = fgetc(f);
    if (i==EOF) return MP_False;
    ungetc(i,f);
    if (i=='B') {
	/* old style stencil file */
	if (!skip_fontpart(f)) return MP_False;
	if (!old_get_char(&nr, f)) return MP_False;
	if (nr==Newline || nr==Settab) {
	    formatnr = (nr==Settab?2:1);
	    if (!old_get_char(&nr, f)) return MP_False;
	} else
	    formatnr = 0;
	i=0;
	while (good && i< (int)nr)
	    if (aig(nota = (NOTATION *) malloc(sizeof(NOTATION)))) {
		if (aig(good = old_get_notation(nota, f))) {
		    add_nota(fnr, -1, nota);
		    i++;
		}
	    } else
		good = MP_False;
	int_clear(savelist);
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_False;
	return good;
    } else {
	i = edit_fnr;
	edit_fnr = fnr;
	read_file(f,STENCILFILE);
	edit_fnr = i;
	cleanup_filestack();
	cleanup_stencilstack();
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_True;
	return MP_True;
    }
}

static void write_all_notations(int fnr, FILE *f)
{
    int i,j,n;
    int *c;

    set_file(f);
    put_filecode(STENCILFILE);
    for (j=0; j<MAX_KIND; j++) {
	n = int_max(nf_item(fnr).nkind[j]);
	c = &int_item(nf_item(fnr).nkind[j],0);
	for (i=0; i<n; i++,c++)
	    save_stencil(nt_item(*c).vers[0].ivnr);
    }
    int_clear(savelist);
    unset_file();
}

Char backupname[1000];

static void remove_backup(int fnr)
{
    Char *h;

    backupname[0]=0;
    Ustrncat(backupname, translate("#"),1000);
    if (nf_item(fnr).name) {
	Ustrncat(backupname, nf_item(fnr).name,1000);
    } else {
      Char *s;
      Ustrncat(backupname, translate("stencil"),1000);
      h=backupname+Ustrlen(backupname);
      h[20]=0;
      s=Ultostr(fnr,h+20);
      while (aig(*h++ = *s++));
    }
    Ustrncat(backupname,translate("#.mps"),1000);
    h = nf_item(fnr).dirname;
    if (h) {
      buffer[0]=0;
      Ustrcat(buffer,h);
      Ustrcat(buffer, translate("/"));
      Ustrcat(buffer, backupname);
      remove_file(buffer);
    } else {
      remove_file(backupname);
    }
}

static void make_backup(int fnr, int dump)
{
    FILE *f;

    if (nf_item(fnr).autosaved) return;
    buffer[0]=0;
    if (dump) {
      Ustrcat(buffer,translate("dump"));
      if (nf_item(fnr).name)
	Ustrcat(buffer, nf_item(fnr).name);
      else {
	Char *h;
	Char *s;
	Ustrcat(buffer, translate("stencil"));
	h=backupname+Ustrlen(buffer);
	h[20]=0;
	s=Ultostr(fnr,h+20);
	while (aig(*h++ = *s++));
      }
      Ustrcat(buffer,translate(".mps"));
    } else {
      Ustrcat(buffer,translate("#"));
      if (nf_item(fnr).name)
	Ustrcat(buffer, nf_item(fnr).name);
      else {
	Char *h;
	Char *s;
	Ustrcat(buffer, translate("stencil"));
	h=backupname+Ustrlen(buffer);
	h[20]=0;
	s=Ultostr(fnr,h+20);
	while (aig(*h++ = *s++));
      }
      Ustrcat(buffer, translate("#.mps"));
    }
    f = open_dirfile(nf_item(fnr).dirname?nf_item(fnr).dirname:translate(""), buffer, "wb");
    if (!f) f = open_dirfile(translate("/tmp"),buffer, "wb");
    if (f) {
	write_all_notations(fnr, f);
	fclose(f);
	nf_item(fnr).autosaved = MP_True;
    }
}

int notation_with_name(Char *name)
{
    int i=nt_max;
    Bool found = MP_False;
    NOTATION *nota = &nt_item(0);

    while (i>0 && !(found= (nota->name && !Ustrcmp(name,nota->name)))) {
	i--;
	nota++;
    }
    if (found)
	return nota->innr;
    else
	return -1;
}

int notation_with_number(unsigned long number)
{
    int hnum;
    hnum= hash_unique_number(number);
    return hash_to_pos_vnr(hash_number_vnr[hnum], number);
}

int add_notation(int oldnr, NOTATION *nota)
{
    return add_nota(edit_fnr, oldnr, nota);
}

static void add_notation_ref(int nnr)
{
    NOTATIONFILE *nfile;

    nfile = &nf_item(edit_fnr);
    nfile->autosaved = MP_False;
    nfile->saved = MP_False;
    if (!int_contains(nfile->nkind[nt_item(nnr).kind],nnr)) {
	nfile->nrnt++;
	int_add(nfile->nkind[nt_item(nnr).kind],nnr);
	nt_item(nnr).fillocks++;
	nt_item(nnr).locks++;
    }
}

int nr_visible(int fnr)
{
    return nf_item(fnr).nrnt;
}

NOTATION *which_notation(int inr)
{
    return &nt_item(lu_item(inr).nnr);
}

VERSION *which_version(int inr)
{
    return &(nt_item(lu_item(inr).nnr).vers[lu_item(inr).vnr]);
}

Index which_version_nr(int inr)
{
    return lu_item(inr).vnr;
}

void lock_stencil(Index inr)
{
    nt_item(lu_item(inr).nnr).locks++;
}

void unlock_stencil(Index inr)
{
    destroy_notation(&nt_item(lu_item(inr).nnr));
}

int load_stencil(int nr)
{
    if (nr<0 || nr>=int_max(savelist)) nr = int_max(savelist)-1;
    return int_item(savelist, nr);
}

int save_stencil(Index innr)
{
    int i=0,max = int_max(savelist),inr;
    inr = which_version(innr)->ivnr;
    while (i<max && int_item(savelist,i) != inr) i++;
    if (i==max) put_stencil(&nt_item(lu_item(inr).nnr));
    while (i<int_max(savelist) && int_item(savelist,i) != inr) i++;
    if (i==int_max(savelist)) return -1;
    else return i;
}

static Bool version_equal(VERSION *a, VERSION *b)
{
    int i,j;
    if (a->nr_plhd != b->nr_plhd) return MP_False;
    for (i=0; i<MAXFORMAT; i++) {
	if (i!=NAMEFORMAT) {
	    if (a->max[i]!=b->max[i]) return MP_False;
	    for (j=0; j<a->max[i]; j++)
		if (a->format[i][j]!=b->format[i][j]) return MP_False;
	}
    }
    return MP_True;
}

static Bool stencil_equal(NOTATION *a, NOTATION *b)
{
    /* it does not check the versions */
    return (a->space == b->space &&
	    a->prec == b->prec &&
	    a->kind == b->kind &&
	    !a->name == !b->name &&
	    !a->helpfilename == !b->helpfilename &&
	    (!a->name || !Ustrcmp(a->name, b->name)) &&
	    (!a->helpfilename || !Ustrcmp(a->helpfilename, b->helpfilename)));
}

static Bool replaceable(NOTATION *a, NOTATION *b, int *remap)
{
    /* Can notation a be replaced by notation b? If so, remap will
    ** show how the versions have to be remapped.
    */
    Bool matched;
    int k,l;
    if (!a->versions || !b->versions || !stencil_equal(a,b)) return MP_False;
    matched=MP_True;
    for (k=0; k<a->versions && matched; k++) {
	l=0;
	while (l<b->versions &&
	       !(matched=version_equal(a->vers+k,b->vers+l)))
	    l++;
	remap[k]=l;
    }
    return matched;
}

static void replace_stencil(NOTATION *a, NOTATION *b, int *remap)
{
    /* replace notation a with notation b. */
    LOOKUPITEM *loi=&lu_item(0);
    FlexArray *fli;
    int k,l;
    k=0;
    l=lu_max;
    while (k<l) {
	if (loi->nnr == a->innr) {
	    loi->nnr = b->innr;
	    loi->vnr = remap[loi->vnr];
	}
	k++;loi++;
    }
    l = a->innr;
    for (k=0; k<nf_max; k++) {
	fli= &(nf_item(k).nkind[a->kind]);
	if (int_max(*fli) && int_contains(*fli,l)) {
	    if (int_contains(*fli, b->innr)) {
		int_remove(*fli, l);
		a->locks--;
		a->fillocks--;
		nf_item(k).nrnt--;
	    } else
		int_switch(*fli,l, b->innr);
	}
    }
    b->locks += a->locks;
    b->fillocks += a->fillocks;
    a->locks=0;
    a->fillocks=0;
    for (k=0; k<a->versions; k++) {
	if (a->vers[k].format[NAMEFORMAT] &&
	    !b->vers[remap[k]].format[NAMEFORMAT]) {
	    b->vers[remap[k]].format[NAMEFORMAT]=
		a->vers[k].format[NAMEFORMAT];
	    b->vers[remap[k]].max[NAMEFORMAT]=
		a->vers[k].max[NAMEFORMAT];
	    a->vers[k].max[NAMEFORMAT]=0;
	    a->vers[k].format[NAMEFORMAT]=NULL;
	}
    }
    destroy_notation(a);
}

Bool remove_double_template(int innr)
{
    NOTATION *all;
    int i,m;
    int remap[50];
    Bool done=MP_False;

    all = (NOTATION*) allnotas.arr;
    m = nt_max;
    for (i=0; i<m && !done; i++) {
	if (i!=innr && all[i].versions &&
	    replaceable(all+innr,all+i,remap)) {
	    done=MP_True;
	    replace_stencil(all+innr, all+i, remap);
	}
    }
    return done;
}

void remove_double_file(int sfnr)
{
    NOTATION *all;
    FlexArray *fli;
    int *stl;
    int i,j,ki,m;
    int remap[50];
    Bool done;

    if (sfnr<0 || sfnr>= nf_max) return;
    all= (NOTATION*) allnotas.arr;
    m= nt_max;
    for (i=0; i<MAX_KIND; i++) {
	fli = &(nf_item(sfnr).nkind[i]);
	stl = fli->arr;
	for (ki=0; ki<int_max(*fli); ki++) {
	    done=MP_False;
	    for (j=0; j<m && !done; j++) {
		if (j!=stl[ki] && replaceable(all+stl[ki],all+j, remap)) {
		    int n = int_max(*fli);
		    done=MP_True;
		    replace_stencil(all+stl[ki], all+j, remap);
		    if (n-int_max(*fli)) ki--;
		}
	    }
	}
    }
}

void remove_multiple_files(int sfnr)
{
    NOTATION *all;
    FlexArray *fli;
    int *stl;
    int i,ki;

    if (sfnr<0 || sfnr>= nf_max) return;
    all= (NOTATION*) allnotas.arr;
    for (i=0; i<MAX_KIND; i++) {
	fli = &(nf_item(sfnr).nkind[i]);
	stl = fli->arr;
	for (ki=0; ki<int_max(*fli);) {
	    if (all[stl[ki]].fillocks>1) {
		all[stl[ki]].fillocks--;
		all[stl[ki]].locks--;
		nf_item(sfnr).nrnt--;
		int_remove(*fli, stl[ki]);
	    } else ki++;
	}
    }
}

void remove_double(void)
{
    int i,j,m;
    NOTATION *all;
    int remap[50];

    all = (NOTATION*) allnotas.arr;
    m = nt_max;
    for (i=0; i<m; i++) {
	if (all[i].versions) {
	    for (j=0; j<m; j++) {		
		if (i!=j && replaceable(all+i,all+j,remap) &&
		    (all[i].versions!=all[j].versions ||
		     all[i].fillocks <= all[j].fillocks))
		    replace_stencil(all+i,all+j, remap);
	    }
	}
    }
}

static int make_stencil(Char *str, int len, int kind, int prec, int spac)
{
    int i,j;
    NOTATION nota;
    Char b[1000];
    int phidx[15];

    nota.vers = (VERSION*) malloc(sizeof(VERSION));
    nota.prec = prec;
    nota.kind = (Opkind) kind;
    nota.space = spac;
    nota.nnr = new_number();
    nota.versions = 1;
    nota.name = NULL;
    nota.helpfilename=NULL;
    nota.locks=nota.fillocks=nota.innr=0;
    nota.vers[0].vnr = new_number();
    nota.vers[0].max[0]=len;
    nota.vers[0].format[0]=str;
    nota.vers[0].nr_plhd=0;
    nota.vers[0].phindex=NULL;
    nota.vers[0].lines=0;
    j=0;
    for (i=0; i<len && str[i]; i++) {
	if (IsPh(str[i])) phidx[j++]=i;
	if (IsNewline(str[i])) nota.vers[0].lines++;
    }
    nota.vers[0].max[0]=i;
    if (j) {
	nota.vers[0].phindex = (int*) malloc(j*sizeof(int));
	for (i=0; i<j; i++)
	    nota.vers[0].phindex[i]=(phidx[i]<<16)+str[phidx[i]];
	nota.vers[0].nr_plhd=j;
    }
    for (i=1;i<MAXFORMAT; i++) {
	nota.vers[0].max[i]=0;
	nota.vers[0].format[i]=NULL;
    }
    for (i=0; i<len; i++) {
        if (str[i]>32 && str[i]<127) b[i]=str[i];
        else if (str[i]>(Char)(0xffff-37))
	    b[i]="\n^><+-[]\n     {}       -.((::) [][]|~"[0xffff-str[i]];
	else
	    b[i]=' ';
    }
    b[i]='\0';
    message2(MP_MESSAGE, translate("Generating new stencil: "), b);
    add_notation(-1, &nota);
    return nota.vers[0].ivnr;
}

int match_format_or_make(Char *str, int len, int kind, int prec, int spac)
{
    INTSTACK *h;
    int match[8];
    int i=0,j=0;
    if (!str) return -1;
    while (i<len) {
	if (Char2Font(str[i])==StackFont) str[j++]=StackC;
	else if (str[i]>=CloseGap && str[i]<=CloseStack)
	    switch (str[i]) {
	    case OpenTop:    case OpenGap:
	    case OpenBottom: case CloseBottom: break;
	    case CloseTop:   str[j++]=TopGap;      break;
	    case CloseGap:   str[j++]=GapBottom;   break;
	    case CloseStack: str[j++]=StackClose;  break;
	    default: str[j++]=str[i]; break;
	    }
	else str[j++]=str[i];
	i++;
    }
    if (i!=j) str[len=j]=0;
    h = hash_string[hash_screen_format(str,len)];
    for (i=0;i<8;match[i++]=-1);
    while (h && match[0]<0) {
	Char *c = nt_lu_hi_vers(h).format[SCREENFORMAT];
	int lm = nt_lu_hi_vers(h).max[SCREENFORMAT];
	Char *d = str;
	if (len == lm) {
	    while (lm && (*d == *c)) c++,d++, lm--;
	    if (!lm) {
		Index nr = head_int(h);
		int pr = nt_item(lu_item(nr).nnr).prec;
		int kn = nt_item(lu_item(nr).nnr).kind;
		int sp = nt_item(lu_item(nr).nnr).space;
		j=0;
		if (pr != prec) j=j+1;
		if (kn != kind) j=j+2;
		if (sp != spac) j=j+4;
		if (match[j]<0) match[j]=nr;
	    }
	}
	h = tail_stack(h);
    }
    for (i=0; i<7 && match[i]<0; i++);
    if (match[i]<0)
	return make_stencil(str, len, kind, prec, spac);
    else {
	free(str);
	return match[i];
    }
}

static void remove_all_notations(int fnr);

void cleanup_stencilstack(void)
{
    VERSIONLIST *h=versionstack, *g;
    int i;

    int_clear(savelist);
    while (h) {
	free(h->vers.phindex);
	h->vers.phindex=0;
	for (i=0; i<MAXFORMAT; i++) {
	    free(h->vers.format[i]);
	    h->vers.format[i]=NULL;
	    h->vers.max[i]=0;
	}
	g = h->next;
	free(h);
	h=g;
    }
    h=vfreel;
    while (h) {
	g = h->next;
	free(h);
	h=g;
    }
    versionstack=vfreel=NULL;
    remove_all_notations(0);
    for (i=0; i<MAX_KIND; i++)
	int_clear(nf_item(0).nkind[i]);
    nf_item(0).nrnt=0;
    nf_item(0).saved = nf_item(0).autosaved = MP_True;
}

int nnr_vnr2innr(int nnr, int vnr)
{
    if (!nt_item(nnr).versions) return -1;
    return nt_item(nnr).vers[vnr].ivnr;
}

int position(VERSION *vers, Char placeh)
{
    int i=0;
    while (i<vers->nr_plhd) {
	if ((vers->phindex[i]&0xffff)==placeh) return vers->phindex[i]>>16;
	i++;
    }
    i=0;
    while (i<vers->nr_plhd) {
	if (Num(vers->phindex[i])==Num(placeh)) return vers->phindex[i]>>16;
	i++;
    }
    return -1;
}

int stencil_position_right(Index innr, Char phnr)
{
    VERSION *vers = &(nt_item(lu_item(innr).nnr).vers[lu_item(innr).vnr]);
    int i=0;
    while (i<vers->nr_plhd && ((vers->phindex[i]&0xffff)!=phnr)) i++;
    if (i<vers->nr_plhd-1)
	return vers->phindex[i+1]>>16;
    else
	return -1;
}

int stencil_position_left(Index innr, Char phnr)
{
    VERSION *vers = &(nt_item(lu_item(innr).nnr).vers[lu_item(innr).vnr]);
    int i=0;
    while (i<vers->nr_plhd && ((vers->phindex[i]&0xffff)!=phnr)) i++;
    if (i && i!=vers->nr_plhd)
	return vers->phindex[i-1]>>16;
    else
	return -1;
}

NOTATION *get_notation_kind(int fnr, int kind, int nr)
{
    if (kind<0 || kind>=MAX_KIND || fnr<0 || fnr>=nf_max ||
	nr<0 || nr >= int_max(nf_item(fnr).nkind[kind])) return NULL;
    return &nt_item(int_item(nf_item(fnr).nkind[kind],nr));
}

int get_notation_nr(int fnr, int kind, int nr)
{
    if (nr<0 || kind<0 || fnr<0 || kind>=MAX_KIND || fnr>=nf_max ||
	nr >= int_max(nf_item(fnr).nkind[kind])) return -1;
    return int_item(nf_item(fnr).nkind[kind],nr);
}

/*
int get_new_version(int inr)
{
    if (nt_item(lu_item(inr).nnr).versions == lu_item(inr).vnr+1)
	return nt_item(lu_item(inr).nnr).vers[0].ivnr;
    else
	return nt_item(lu_item(inr).nnr).vers[lu_item(inr).vnr+1].ivnr;
}
*/
void remove_notation(int fnr, int nr)
{
    int i = nt_item(nr).kind;
    if (int_contains((nf_item(fnr).nkind[i]), nr)) {
	int_remove((nf_item(fnr).nkind[i]), nr);
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_False;
	nt_item(nr).fillocks--;
	destroy_notation(&nt_item(nr));
	nf_item(fnr).nrnt--;
    }
}

static void remove_all_notations(int fnr)
{
    int i,j,k;
    int *c;

    for (j=0; j<MAX_KIND; j++) {
	k = int_max(nf_item(fnr).nkind[j]);
	c = &int_item(nf_item(fnr).nkind[j], 0);
	for (i=0; i<k; i++,c++) {
	    nt_item(*c).fillocks--;
	    destroy_notation(&nt_item(*c));
	}
	int_clear(nf_item(fnr).nkind[j]);
    }
    nf_item(fnr).nrnt=0;
    nf_item(fnr).saved = nf_item(fnr).autosaved = MP_True;
}

Bool move_nota_left(int fnr, int kind, int anr)
{
    int i;
    int *c;

    if (anr > 0) {
	c = &int_item(nf_item(fnr).nkind[kind], anr-1);
	i = *c;
	*c = *(c+1);
	*(c+1) = i;
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_False;
	return MP_True;
    } else
	return MP_False;
}

Bool move_nota_right(int fnr, int kind, int anr)
{
    int i;
    int *c;
    if (anr < int_max(nf_item(fnr).nkind[kind])-1) {
	c = &int_item(nf_item(fnr).nkind[kind], anr);
	i = *c;
	*c = *(c+1);
	*(c+1) = i;
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_False;
	return MP_True;
    } else
	return MP_False;
}

int new_notation_window(void)
{
    if (use_file_nr>=0) {
	nf_item(use_file_nr).nr_windows++;
	return use_file_nr;
    } else {
	int i,j,slen;
	NOTATIONFILE tfile;
	slen=Ustrlen(translate("stencil"));
	tfile.name = (Char *) malloc((slen+15)*sizeof(Char));
	Ustrcpy(tfile.name, translate("stencil"));
	i=1;
	do {
	  Char lbuf[20];
	  Char *s;
	  lbuf[19]=0;
	  s= Ultostr(i,lbuf+19);
	  Ustrcpy(tfile.name+slen, s);
	  i++;
	  for (j=0; j<nf_max && (!nf_item(j).name ||
				 Ustrcmp(nf_item(j).name,tfile.name)); j++);
	} while (j!=nf_max);
	tfile.nr_windows = 1;
	tfile.nrnt=0;
	tfile.samename=0;
	tfile.dirname = NULL;
	for (i=0; i<MAX_KIND; i++)
	    int_init(tfile.nkind[i]);
	tfile.saved = tfile.autosaved = MP_True;
	i=nf_max;
	nf_add(tfile);
	return i;
    }
}

static void remove_window_ref(int fnr)
{
    if (fnr<0 || fnr>=nf_max || !nf_item(fnr).nr_windows) return;
    if (nf_item(fnr).nr_windows>1) {
	nf_item(fnr).nr_windows--;
    } else {
	int i;

	make_backup(fnr, 0);
	nf_item(fnr).nr_windows=0;
	free(nf_item(fnr).name);
	free(nf_item(fnr).dirname);
	nf_item(fnr).name =  nf_item(fnr).dirname = NULL;
	remove_all_notations(fnr);
	for (i=0; i<MAX_KIND; i++)
	    int_clear(nf_item(fnr).nkind[i]);
	nf_item(fnr).nrnt = 0;
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_True;
	use_file_nr = -1;
    }
}

int clear_notation_window(int fnr)
{
    remove_window_ref(fnr);
    use_file_nr = -1;
    return new_notation_window();
}

static void extract_filename(Char *fullname, Char ** dirname, Char **filename)
{
    Char *c,*h;
    int i;

    c = strip_name(fullname);
    i = Ustrlen(fullname) - Ustrlen(c);
    *dirname = (Char *) malloc((sizeof(Char)*(i+1)));
    Ustrncpy(*dirname, fullname, i);
    (*dirname)[i] = '\0';
    i = Ustrlen(c);
    h = Ustrrchr(c,'.');
    if (h && !Ustrcmp(h,translate(".mps")))
	i -= Ustrlen(translate(".mps"));
    else if (h && !Ustrcmp(h,translate(".nota")))
        i -= Ustrlen(translate(".nota"));
    *filename = (Char *) malloc(sizeof(Char)*(i+1));
    Ustrncpy(*filename, c, i);
    (*filename)[i] = '\0';
}

static int file_loaded(Char *filename)
{
    int i;
    Char *c,*h;
    Char t;

    c = strip_name(filename);
    h = Ustrrchr(c,'.');
    if (h) {
	if (Ustrcmp(h, translate(".nota")) && Ustrcmp(h, translate(".mps"))) h=NULL;
	if (h) *h=0;
    }
    t=*c;
    for (i=0; i<nf_max; i++) {
	if (nf_item(i).nr_windows && nf_item(i).name && nf_item(i).dirname &&
	    !Ustrcmp(nf_item(i).name, c)) {
	    *c=0;
	    if (!Ustrcmp(nf_item(i).dirname, filename)) {
		*c=t;
		if (h) *h='.';
		return i;
	    }
	    *c=t;
	}
    }
    if (h) *h='.';
    return -1;
}

static void set_notation_filename(int fnr, Char *filename)
{
    int i,n;
    free(nf_item(fnr).name);
    free(nf_item(fnr).dirname);
    extract_filename(filename, &nf_item(fnr).dirname, &nf_item(fnr).name);
    n=0;
    for (i=0; i<nf_max; i++)
	if (i!=fnr && nf_item(i).name &&
	    !Ustrcmp(nf_item(i).name, nf_item(fnr).name))
	    n=n|(0x1<<nf_item(i).samename);
    for (i=0;n&0x1; i++,n=n>>1);
    nf_item(fnr).samename=i;
}

int load_notation_window(int fnr, Char *filename)
{
    int nfnr;
    FILE *f;

    nfnr = file_loaded(filename);
    if (nfnr<0) {
	use_file_nr = -1;
	if (aig(f=fopen((char*)UstrtoFilename(filename), "rb"))) {
	    nfnr = new_notation_window();
	    remove_window_ref(fnr);
	    set_notation_filename(nfnr, filename);
	    read_all_notations(nfnr,f);
	    fclose(f);
	    message(MP_MESSAGE, translate("Loaded stencils from disk."));
	    return nfnr;
	} else {
	    message2(MP_CLICKREMARK, translate("Unable to load templates from file "), filename);
	    failure=MP_True;
	    return fnr;
	}
    }
    if (nfnr>=0 && fnr!=nfnr) {
	nf_item(nfnr).nr_windows++;
	make_backup(fnr,0);
	remove_window_ref(fnr);
	message(MP_MESSAGE, translate("Using stencils from memory."));
	return nfnr;
    }
    if (nfnr>=0 && fnr==nfnr) {
	make_backup(fnr,0);
	if (aig(f=fopen((char*)UstrtoFilename(filename), "rb"))) {
	    remove_all_notations(fnr);
	    read_all_notations(fnr,f);
	    fclose(f);
	    message(MP_MESSAGE, translate("Reloaded stencils from disk."));
	} else {
	    message2(MP_CLICKREMARK, translate("Unable to reload stencils from file "),
		     filename);
	    failure=MP_True;
	}
	return fnr;
    }
    /* unable to reach this statement */
    message(MP_MESSAGE, translate("Sorry, don't know what to do in this situation."));
    return fnr;
}

void save_notation_window(int fnr, Char *filename)
{
    FILE *f;
    int namefnr;

    namefnr = file_loaded(filename);
    if (namefnr>=0 && namefnr!=fnr) {
	make_backup(fnr,0);
	message(MP_CLICKREMARK, translate("Unable to save file: new name already used."));
	failure=MP_True;
    }
    if (aig(f = fopen((char*)UstrtoFilename(filename), "wb"))) {
	write_all_notations(fnr, f);
	fclose(f);
	remove_backup(fnr);
	set_notation_filename(fnr, filename);
	nf_item(fnr).saved = nf_item(fnr).autosaved = MP_True;
	message(MP_MESSAGE, translate("Stencil saved."));
    } else {
	make_backup(fnr,0);
	message2(MP_CLICKREMARK, translate("Unable to save stencil in file "),filename);
	failure=MP_True;
    }
}

void free_notation_window(int fnr)
{
    remove_window_ref(fnr);
}

void saved_notation_file(int fnr)
{
    nf_item(fnr).saved=MP_True;
}

void auto_save_window(int fnr, int dump)
{
    make_backup(fnr, dump);
}

int notation_not_saved(int fnr)
{
    int i=0;

    if (fnr) {
	if (fnr < nf_max && nf_item(fnr).name && !nf_item(fnr).saved)
	    return fnr;
	else
	    return 0;
    }
    if (!nf_max) return 0;
    while (i<nf_max && (!nf_item(i).name || nf_item(i).saved)) i++;
    if (i<nf_max)  return i;
    else return 0;
}


Bool last_window(int fnr)
{
    return (nf_item(fnr).nr_windows==1);
}

void rename_notation_window(int fnr, Char *name)
{
    int nnr = file_loaded(name);

    if (nnr>=0 && nnr!= fnr) {
	message(MP_ERROR, translate("Name is already used for another loaded file."));
    } else
	set_notation_filename(fnr, name);
}

static INTSTACK *filelocks=0;

void add_file_ref(int fnr)
{
    push_int(&filelocks, fnr);
}

void clear_file_ref(void)
{
    int i;
    while (filelocks) {
	i=pop_int(&filelocks);
	remove_window_ref(i);
    }
}

int get_notation_number(int fnr)
{
    return nf_item(fnr).samename;
}

Char *get_notation_filename(int fnr)
{
    return nf_item(fnr).name;
}

int get_next_filename(int fnr, Char **name, Bool *opened)
{
    fnr++;
    while (fnr<nf_max)
	if (nf_item(fnr).name) {
	    *name = nf_item(fnr).name;
	    *opened = (nf_item(fnr).nr_windows>0);
	    return fnr;
	} else fnr++;
    return -1;
}

Char *get_notation_dirname(int fnr)
{
    return nf_item(fnr).dirname;
}

void view_notation_filenames(FILE *f)
{
    Bool finished = MP_False;
    char buf[BUFSIZE];

    while (!finished)
        finished = (!fgets(buf, BUFSIZE, f) || begins_with("STOP",buf));
}

Bool load_notation_filenames(FILE *f)
{
    Char name[128];
    Char *g;
    char linebuf[256];
    Char *filename, *h;
    Char *swpos;
    Bool not_finished, new_loaded = MP_False, open_fault = MP_False,
	name_clash = MP_False;
    Char *dirs[6];
    int i,n=BUFSIZE,nr;

    g=buffer;
    concat_in(g,notationdir,translate("/%.mps"));
    dirs[2]=g;
    i=Ustrlen(g)+1; n=n-i; g=g+i;
    concat_in(g,notationdir,translate("/%.nota"));
    dirs[3]=g;
    i=Ustrlen(g)+1; n=n-i; g=g+i;
    concat_in(g,program_notationdir,translate("%.mps"));
    dirs[4]=g;
    i=Ustrlen(g)+1; n=n-i; g=g+i;
    concat_in(g,program_notationdir,translate("%.nota"));
    dirs[5]=g;
    i=Ustrlen(g)+1; n=n-i; g=g+i;
    not_finished = (fgets(linebuf, 255, f) && !begins_with("STOP", linebuf));
    while (not_finished) {
        /* copy name */
        {
	  int tnp;
	  for (tnp=0; linebuf[tnp]; tnp++) g[tnp]=linebuf[tnp];
	  g[tnp]=0;
	}
        g[Ustrlen(g)-1]='\0';
	swpos = strip_name(g);
	Ustrcpy(name, swpos);
	*swpos = '\0';
	dirs[0] = g;
	h=g+Ustrlen(g);
	concat_in(h+6, g,translate("%.nota"));
	concat_in(h,translate("%.mps"),NULL);
	dirs[1]=h+6;
	h=Ustrrchr(name,'.');
	if (h && (!Ustrcmp(h,translate(".nota")) || !Ustrcmp(h,translate(".mps")))) *h='\0';
	filename = search_through_dirs(dirs,6,name);
	if (filename) {
	    if (file_loaded(filename)<0) {
		nr = load_notation_window(-1, filename);
		/*
		** if you remove the window reference, the templates are
		** removed and new templates are generated.
		** instead, use a list of files which are to be removed.
		** after loading all the files, (p.e. from a project file)
		** this list will be used to remove the references.
		** remove_window_ref(nr);
		*/
		add_file_ref(nr);
		new_loaded = MP_True;
	    }
	} else
	    open_fault = MP_True;
        not_finished = (fgets(linebuf,255,f) && !begins_with("STOP",linebuf));
    }
    if (!begins_with("STOP", linebuf)) return MP_False;
    if (open_fault)
        message(MP_CLICKREMARK, translate("Unable to load all the missing stencils."));
    if (name_clash)
	message(MP_CLICKREMARK, translate("Used stencils already loaded under another name."));
    if (new_loaded) {
        message(MP_MESSAGE, translate("Loaded missing stencils."));
        changed_notation();
    }
    return MP_True;
}

static char *info_template=
"%[Template Info    %=\n\n"
"Name\t%n\n"
"Help File\t%h\n"
"Kind\t%o\n"
"  Precedence\t%p\n"
"  Spacing\t%s\n"
"Versions\t%v\n"
"  Numbers\t%N\n"
"Used\t%u times\n"
"Files\t%f%]";

Char *make_info(int innr)
{
    int i,j,k;
    char s[60];
    Char *h;
    Char *c;
    NOTATION *nota = which_notation(innr);
    if (!nota) return NULL;
    c = (Char*) malloc(2000*sizeof(Char));
    i=0;j=0;
    while (info_template[i]) {
	switch (info_template[i]) {
	case '\n': c[j++]=Newline; break;
	case '\t': c[j++]=Rtab; break;
	case '%':
	    i++;
	    switch (info_template[i]) {
	    case '[': c[j++]=TabOpen; break;
	    case ']': c[j++]=TabClose; break;
	    case '=': c[j++]=Settab; break;
	    case 'n':
		if (nota->name)
		    for (k=0; (aig(c[j]=nota->name[k++])); j++);
		else {
		    Char *name;
		    name=nota->vers[0].format[NAMEFORMAT];
		    if (!name) name=nota->vers[0].format[SCREENFORMAT];
		    if (name)
			while (*name) {
			    if (!IsTab(*name) || *name<SoftNewline)
				c[j++]=(Char)Char2Ph(*name);
			    name++;
			}
		    else c[j++]='-';
		}
		break;
	    case 'h':
		if (nota->helpfilename)
		    for (k=0; (aig(c[j]=nota->helpfilename[k++])); j++);
		else c[j++]='-';
		break;
	    case 'o':
		h=kind_description(nota->kind);
		if (h)
		    for (k=0; (aig(c[j]=h[k++])); j++);
		else
		    c[j++]='-';
		break;
	    case 'p':
		if (!nota->kind)
		    c[j++]='-';
		else {
		    sprintf(s,"%i",nota->prec);
		    for (k=0; (aig(c[j]=s[k++])); j++);
		}
		break;
	    case 's':
		if (!nota->kind)
		    c[j++]='-';
		else {
		    sprintf(s,"%i",nota->space);
		    for (k=0; (aig(c[j]=s[k++])); j++);
		}
		break;
	    case 'v':
		sprintf(s,"%i",nota->versions);
		for (k=0; (aig(c[j]=s[k++])); j++);
		break;
	    case 'N':
		for (k=0; k < nota->versions; k++) {
		    int w;
		    sprintf(s, "%lu", nota->vers[k].vnr);
		    for (w=0; (aig(c[j]=s[w++])); j++);
		    if (k+1<nota->versions) {
			c[j++]=Newline;
			c[j++]=Rtab;
		    }
		}
		break;
	    case 'u':
		sprintf(s,"%i",nota->locks-nota->fillocks-1);
		for (k=0; (aig(c[j]=s[k++])); j++);
		break;
	    case 'f':
		if (!nota->fillocks)
		    c[j++]='-';
		else {
		    int n=nota->fillocks, l=0;
		    while (n && l<nf_max) {
			if (int_contains(nf_item(l).nkind[nota->kind],
					 nota->innr)) {
			    h=nf_item(l).name;
			    n--;
			    if (h) {
				for (k=0; (aig(c[j]=h[k++])); j++);
				k = nf_item(l).samename;
				if (k) {
				    c[j++]=' ';
				    c[j++]='<';
				    if (k>9) c[j++]=(Char)('0'+k/10);
				    c[j++]=(Char)('0'+k%10);
				    c[j++]='>';
				}
			    } else
				c[j++]='-';
			    if (n) { c[j++]=Newline;c[j++]=Rtab; }
			}
			l++;
		    }
		}
		break;
	    default:
		c[j++]=info_template[i];
		break;
	    }
	    break;
	default:
	    c[j++]=info_template[i];
	    break;
	}
	i++;
    }
    c[j]=0;
    return c;
}

