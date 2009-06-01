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
**  fileread.c
**
**  Functions to read different file types with the same functions.
**
**  NOTE: The file structure will change more dramatically in the
**        final version.  At the moment, only a small change has
**        been made, just to enable saving documents.
**        Old versions of mathpad will still be able to load the files,
**        but they will corrupt the content, especially of templates.
**
**  Mathpad uses 2-byte strings a lot, so that will be the default return
**  type. If other types are needed (integers, normal characters), a
**  convertion is needed in the called functions or a different loading
**  function is needed.
**
**  Integers are added in a kind of hexadecimal type. The escape sequence
**  #25 is used for 4-bytes integer, #24 for 2-bytes integers. The encoding
**  is rather simple:
**
**  cod(n)  =  n < 16     ->   'A' + n
**             n > 15     ->   hex(n div 16) ++ cod(n mod 16)
**  hex(n)  =  'normal hexadecimal print routine'
**
**  (cod changes the last character of the normal hexadecimal representation.
**   The closing 'h' is not needed).
**
**  A different approach would be to use the UTF encoding and encode
**  4-byte integers in 2 UTF values.
**  UTF(n):
**         n<128   -> n
**    128<=n<2048  -> (192+ n div 64) ++ (128+ n mod 64) 
**   2048<=n<65536 -> (224+ n div 4096) ++ (128+ (n dvi 64) mod 64) ++
**                      (128+ n mod 64)
**  Or:
**  00000000.0bbbbbbb -> 0bbbbbbb
**  00000bbb.bbbbbbbb -> 110bbbbb 10bbbbbb
**  bbbbbbbb.bbbbbbbb -> 1110bbbb 10bbbbbb 10bbbbbb
**
**  This way, no escape sequences are needed for special values.
**
**  List of escape sequences (and there meaning):
**
**  Note:  many characters in the range 0-31 have a special meaning. The
**         algorithm does not depend on the used character, but it would
**         be nice if documents don't mess up terminals if you use 'cat',
**         'more' or 'less' to see the content.
**
**  #27 [4m     use bold font on terminal. The sequence is the first sequence
**              in a document and must be followed by the unique document
**              identification number. After the number a description
**              follows ('Mathpad Document' etc.).
**              Old versions, with the font-based encoding use "[1m".
**              The loading algorithm detects whether an old or new
**              document is loaded and reencodes the content when
**              necessary.
**
**  #27 [0m     use normal font on terminal.
**
**  #27 ^       start hidden information (ignored).
**
**  #27 \       end hidden information (ignored).
**
**  #16 ....    character (1 byte) represented by ....
**
**  #17 ....    short int (2 bytes) represented by ....
**
**  #18 ....    long int  (4 bytes) represented by ....
**
**  #19 ....    short int (2 bytes) 65536 - .... (mathpad uses high-numbers
**              for special coding characters).
**
**  #20 a       structured encoding of type 'a' follows. Every type has its
**              own encoding and decoding functions. An entry in a lookup
**	        table determines which function is called after the structure
**	        is loaded in a buffer.
**
**  #20#18 ..a  same as '#20 a', but the size of the buffer is specified to
**              speed up the file reading algorithm.
**
**  #21         closes the last #20 sequence. if new structures are added, old
**              versions of the algorithm can savely ignore that part of the
**	        file (with a warning).
**
**  #13         is ignored. If the file is transported via DOS, #13 is added at
**              each newline. #13 can still be entered after #16.
**
*/

#include <stdlib.h>
#include <stdio.h>

#include "mathpad.h"
#include "fileread.h"
#include "memman.h"
#include "unimap.h"

static int loading_old=0;
static MapUchar oldfileremap=0;
#define AutoLoad(A) do { if (!A) MapUcharLoadFile(A,"mptouchar");} while (0)

/* OldNewline will be mapped to Newline if it is part of a string. */

#define OldNewline 0xFFFF

static int get_Str(Char*,int*,int);
static int get_str(Char*,int*,int);
static int get_dtc(Char*,int*,int);

static GETFUNC getfunc[128] = {
/* #0  */  NULL, get_node, get_ascii_node, NULL, NULL, NULL, NULL, NULL,
/* #8  */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* #16 */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* #24 */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* ' ' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* '(' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* '0' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* '8' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* '@' */  NULL, NULL, NULL, get_Str, get_defaults, NULL, get_fonts, NULL,
/* 'H' */  NULL, NULL, NULL, get_keyboard, NULL, get_dtc, get_node, NULL,
/* 'P' */  NULL, NULL, NULL, get_stencil, NULL, NULL, get_version, NULL,
/* 'X' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
/* '`' */  NULL, NULL, NULL, get_str, NULL, NULL, get_findrep, NULL,
/* 'h' */  NULL, NULL, NULL, NULL, NULL, get_smacro, NULL, NULL,
/* 'p' */  NULL, NULL, NULL, get_symbol, NULL, NULL, NULL, NULL,
/* 'x' */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

static int start_state[MAXFILE] = { 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1 };
static char *file_type_use[MAXFILE] = { "",
					      "M",
					      "",
					      "CFNSTVc",
					      "CFSVc",
					      "K",
					      "m",
					      "ms",
					      "Fms",
					      "CDFKMNSTVcfms",
					      "CFNSTVcf" };

static char *type_desc[MAXFILE] = { "",
					  "Detect\n",
					  "Binary\n",
					  "Document\n",
					  "Stencils\n",
					  "Keyboard\n",
					  "LaTeX Macros\n",
					  "Symbol Palettes\n",
					  "Font Definitions\n",
					  "Project File\n",
					  "Find & Replace Rules\n" };

static char enabled[128];
static int state;

typedef struct READSTACK READSTACK;
struct READSTACK {
    READSTACK *next;
    Char *buf;
    int pos;
    int max;
    int funcnr;
};

/*
**  GETFUNC(buffer, len, max)  is used to convert buffer to another type
**  which depends ont GETFUNC. The buffer contains len valid items and
**  max items are allocated (if the function would use buffer).
**  The return value indicates what as to be done after calling the function.
**  The function can change len to return the new buffer size.
**
**  -1     Conversion Error.
**   0     no action.
**   1     insert len
**   2                    insert buffer
**   3     insert len     insert buffer
**   4                                      free buffer
**   5     insert len                       free buffer
**   6                    insert buffer     free buffer
**   7     insert len     insert buffer     free buffer
**
**  The GETFUNC functions are called after a sequence #27a ... #28.
**  buffer contains the value of ... (which could contain already
**  processed sequences). The contents of the buffer depends on
**  what the corresponding PUTFUNC has written in the file.
**
**  The general load algorithm will just preprocess the information
**  to get simple loading functions. Adding new or different formats
**  is done by adding extra functions.
*/

static READSTACK *freel=NULL;
static READSTACK empty = { NULL,NULL,0,0,0};
#define DEFBUFSIZE 1024

static int def_buf_size;

static char *encode = "0123456789abcdef";
#define decode(C) ('0'<= (C) && (C)<='9' ? (C)-'0' : \
		   ('a'<=(C) && (C)<='f' ? (C)+10-'a' :\
		    ('A'<=(C) && (C)<='P' ? (C)+16-'A' : 32)))

static int add_buf(READSTACK *rst, Char n)
{
    if (rst->pos >= rst->max-1) {
	Char *t;
	if (!rst->max) {
	    if (aig(t = (Char *) malloc(def_buf_size*sizeof(Char)))) {
		rst->max = def_buf_size;
		rst->buf = t;
	    } else return 0;
	} else {
	    if (aig(t = (Char *) realloc(rst->buf, rst->max*2*sizeof(Char)))) {
		rst->max = rst->max*2;
		rst->buf = t;
	    } else return 0;
	}
    }
    rst->buf[rst->pos++] = n;
    return 1;
}

static int add_buf_string(READSTACK *rst, Char *c, int len)
{
    Char *t;
    if (rst->pos+len >= rst->max-1) {
	int new_size = (rst->max ? rst->max*2 :  DEFBUFSIZE);
	while (new_size<rst->pos+len) new_size = new_size*2;
	if (!rst->max)
	    t = (Char *) malloc(new_size*sizeof(Char));
	else
	    t = (Char *) realloc(rst->buf, new_size*sizeof(Char));
	if (t) {
	    rst->max = new_size;
	    rst->buf = t;
	} else return 0;
    }
    t = rst->buf+rst->pos;
    rst->pos = rst->pos+len;
    while (len) {
	*t++ = *c++;
	len--;
    }
    return 1;
}

static int push_rstack(READSTACK **rst)
{
    READSTACK *h;

    if (freel) {
	h=freel;
	freel = h->next;
    } else
	h=(READSTACK*) malloc(sizeof(READSTACK));
    if (!h) return 0;
    *h=empty;
    h->next = *rst;
    *rst=h;
    return 1;
}

static void pop_rstack(READSTACK **rst)
{
    READSTACK *h= (*rst)->next;
    int popcode=FREE_BUFFER;
    if ((*rst)->buf) (*rst)->buf[(*rst)->pos]=0;
    if (enabled[(*rst)->funcnr]) {
	popcode = getfunc[(*rst)->funcnr]((*rst)->buf, &((*rst)->pos),
					  (*rst)->max);
	if (popcode<0) popcode = FREE_BUFFER;
    }
    if (popcode & INSERT_LENGTH) {
	add_buf(h, (Char)(((*rst)->pos) >>16));
	add_buf(h, (Char)(((*rst)->pos) && 0xffff));
    }
    if (popcode & INSERT_BUFFER)
	add_buf_string(h, (*rst)->buf, (*rst)->pos);
    if (popcode & FREE_BUFFER)
	if ((*rst)->buf) free((*rst)->buf);
    (*rst)->next = freel;
    freel = *rst;
    *rst = h;
}

int read_file(FILE *f, int ftype)
{
    int i,readc;
    unsigned long number=0;
    READSTACK *rst=NULL;
    unsigned char c;
    char *s=file_type_use[ftype];

    loading_old=0;
    state=start_state[ftype];
    enabled[0] = 0;
    for (i=1; i<128; i++)
	if (aig(enabled[i] = (char)(*s == i)))
	    s++;
    if (getfunc[ftype]) enabled[ftype] = 1;
    def_buf_size = DEFBUFSIZE;
    if (!push_rstack(&rst)) return 0;
    rst->funcnr = ftype;
    i=fgetc(f);
    while (i != EOF) {
	c=(char)i;
	readc=1;
	switch (state) {
	case 0: /* binary */
	    if (c=='\n') add_buf(rst,(loading_old?OldNewline:Newline));
	    else add_buf(rst,(Char)c);
	    break;
	case 1: /* mathpad documents,stencils,... */
	    switch (c) {
	    case 13: /* added by mail/dos  */
		break;
	    case 10:
		add_buf(rst, (loading_old?OldNewline:Newline));
		break;
	    case 16: case 17: case 18: case 19:
		number=0;
		state = c-12;
		break;
	    case 20:
		state = 2;
		def_buf_size = DEFBUFSIZE;
		push_rstack(&rst);  /* should check errors */
		break;
	    case 21:
		if (rst->next)
		    pop_rstack(&rst);
		break;
	    case 27:
		state=8;
		break;
	    default:
		add_buf(rst, (Char)c);
		break;
	    }
	    break;
	case 2:  /* #20 */
	    if (c==18) {
		state = 3;
		number = 0;
	    } else {
		if (c<32 || c>127) c=0;
		rst->funcnr = c;
		state = 1;
	    }
	    break;
	case 3: /* #20#18[0-9a-f]*    */
	    i = decode(c);
	    number = (number<<4) | (i&0xf);
	    if (i&0x30) {
		def_buf_size = number;
		add_buf(rst, 0);
		rst->pos=0;
		state=2;
		def_buf_size = DEFBUFSIZE;
	    }
	    break;
	case 4: /* #16[0-9a-f]*  */
	    i = decode(c);
	    number = (number<<4) | (i&0xf);
	    if (i&0x30) {
		add_buf(rst, (Char) number);
		state=1;
	    }
	    break;
	case 5: /* #17[0-9a-f]*   */
	    i = decode(c);
	    number = (number<<4) | (i&0xf);
	    if (i&0x30) {
		add_buf(rst, (Char)number);
		state=1;
	    }
	    break;
	case 6: /* #18[0-9a-f]*   */
	    i = decode(c);
	    number = (number<<4) | (i&0xf);
	    if (i&0x30) {
		add_buf(rst, (Char)(number>>16));
		add_buf(rst, (Char)(number&0xffff));
		state=1;
	    }
	    break;
	case 7: /* #19[0-9a-f]*   */
	    i = decode(c);
	    number = (number<<4) | (i&0xf);
	    if (i&0x30) {
		add_buf(rst, (Char)(number^0xffff));
		state=1;
	    }
	    break;
	case 8: /* #27      */
	    switch (c) {
	    case '^':
		state=1;
		break;
	    case '\\':
		state=1;
		break;
	    case '[':
		state=9;
		break;
	    default:
		state=1;
		i=c;
		readc=0;
		break;
	    }
	    break;
	case 9: /* #27\[    */
	    switch (c) {
	    case '0':
		state=11;
		break;
	    case '1':
	        loading_old=1;
		state=10;
		break;
	    case '4':
	        loading_old=0;
		state=10;
		break;
	    default:
		add_buf(rst,'[');
		readc=0;
		i=c;
		state=1;
		break;
	    }
	    break;
	case 10: /* #27\[1   */
	    if (c=='m') {
		def_buf_size=DEFBUFSIZE>>2;
		push_rstack(&rst);
		state=2;
	    } else {
		add_buf(rst, '[');
		add_buf(rst,'0');
		readc=0;
		i=c;
		state=1;
	    }
	    break;
	case 11: /* #27\[0  */
	    if (c=='m') {
		if (rst->next)
		    pop_rstack(&rst);
		state=1;
	    } else {
		add_buf(rst, '[');
		add_buf(rst, '1');
		readc=0;
		i=c;
		state=1;
	    }
	    break;
	default:
	    /* Strange. This should not happen. */
	    break;
	}
	if (readc) i=fgetc(f);
    }
    while (rst) pop_rstack(&rst);
    return 1;
}

static FILE *opf=NULL;

#define OPFP(A)  fputc((A),opf)

void set_file(FILE *f)
{
    opf=f;
}

void unset_file(void)
{
    if (opf) {
	opf = NULL;
    }
}

static char mcodebuf[9];

static void mcode(unsigned long c)
{
    int i=-1;

    do {
	mcodebuf[++i] = (char)(c&0xf);
	c = c>>4;
    } while (c);
    while (i) OPFP(encode[mcodebuf[i--]]);
    OPFP('A'+mcodebuf[0]);
}

void put_char(Char c)
{
    if ((c>31 && c<127) || (c>159 && c<255))
	OPFP(c);
    else {
	OPFP(16);
	mcode(c);
    }
}

void put_Char(Char c)
{
    if (c==Newline)
	OPFP(10);
    else if ((c>31 && c<127) || (c>159 && c<255))
	OPFP(c);
    else if (((Char)(c^0xffff)) < c) {
	OPFP(19);
	mcode(c^0xffff);
    } else {
	OPFP(17);
	mcode(c);
    }
}

void put_integer(unsigned long l)
{
    OPFP(18);
    mcode(l);
}

void put_struct(char type, int size)
{
    OPFP(20);
    if (size>0) put_integer(size);
    OPFP(type);
}

void put_end_struct(void)
{
    OPFP(21);
}

static void put_bold_struct(char type, int size)
{
  /* old mpd documents used "[1m", new documents us "[4m" */
    OPFP(27);fputs("[4m",opf);
    if (size>0) put_integer(size);
    OPFP(type);
}

static void put_bold_end_struct(void)
{
    OPFP(27); fputs("[0m",opf);
}

void push_hidden(void)
{
}

void pop_hidden(void)
{
}

void put_filecode(int type)
{
    /* put the `unique' mathpad code first */
    put_bold_struct('M',0);
    OPFP(type);
    fputs("athpad ", opf);
    fputs(type_desc[type], opf);
    put_bold_end_struct();
}

void put_String(Char *s, int len)
{
    int i;
    put_struct('C', len+1);
    for (i=0; i<len; i++) put_Char(s[i]);
    put_end_struct();
}

void put_string(char *s, int len)
{
    int i;
    put_struct('c', len+1);
    for (i=0; i<len; i++) put_char(s[i]);
    put_end_struct();
}

typedef struct CSTRINGSTACK CSTRINGSTACK;
struct CSTRINGSTACK {
    CSTRINGSTACK *next;
    Char *str;
    int len;
    int max;
};

static CSTRINGSTACK *csfreel=NULL, *csstack=NULL;

int get_String(Char **s, int *len, int *max)
{
    if (csstack) {
	CSTRINGSTACK *h = csstack->next;
	*s = csstack->str;
	*len = csstack->len;
	*max = csstack->max;
	csstack->next = csfreel;
	csfreel = csstack;
	csstack=h;
	return MP_True;
    } else
	return MP_False;
}

static int get_Str(Char *s, int *len, int max)
{
    CSTRINGSTACK *h;
    if (csfreel) {
	h=csfreel;
	csfreel=h->next;
    } else
	h = (CSTRINGSTACK*) malloc(sizeof(CSTRINGSTACK));
    if (h) {
        int i;
	h->next = csstack;
	h->str  = s;
	h->len  = *len;
	h->max  = max;
	csstack = h;
	for (i=0; i<*len; i++) {
	  s[i] = oldtonew(s[i]);
	}
	return SUCCESS;
    } else
	return FAILURE;
}


typedef struct STRINGSTACK STRINGSTACK;
struct STRINGSTACK {
    STRINGSTACK *next;
    Char *str;
    int len;
    int max;
};

static STRINGSTACK *sfreel=NULL, *sstack=NULL;

int get_string(Char **s, int *len, int *max)
{
    if (sstack) {
	STRINGSTACK *h = sstack->next;
	*s = sstack->str;
	*len = sstack->len;
	*max = sstack->max;
	sstack->next = sfreel;
	sfreel = sstack;
	sstack=h;
	return MP_True;
    } else
        return get_String(s,len,max);
}

static int get_str(Char *s, int *len, int max __attribute__((unused)))
{
    STRINGSTACK *h;
    if (sfreel) {
	h=sfreel;
	sfreel=h->next;
    } else
	h = (STRINGSTACK*) malloc(sizeof(STRINGSTACK));
    if (h) {
	h->next = sstack;
	h->str = s;
	h->len  = *len;
	h->max  = max;
	sstack = h;
	return SUCCESS;
    } else
	return FAILURE;
}

static int get_dtc(Char *c,
		   int *len __attribute__((unused)),
		   int max __attribute__((unused)))
{
    int i;
    char *s;

    if ((*c)>=MAXFILE)
	*c=BINARYFILE;
    s=file_type_use[*c];
    for(i=0;i<128;i++)
	if (aig(enabled[i] = (char)(*s == i)))
	    s++;
    state=start_state[*c];
    return SUCCESS+FREE_BUFFER;
}

void cleanup_filestack(void)
{
    if (sstack || sfreel) {
	STRINGSTACK *g,*h;
	h = sstack;
	while (h) { g=h; h=h->next; if (g->str) free(g->str); free(g); }
	h = sfreel;
	while (h) { g=h; h=h->next; free(g); }
	sstack = sfreel = NULL;
    }
    if (csstack || csfreel) {
	CSTRINGSTACK *g,*h;
	h = csstack;
	while (h) { g=h; h=h->next; if (g->str) free(g->str); free(g); }
	h = csfreel;
	while (h) { g=h; h=h->next; free(g); }
	csstack = csfreel = NULL;
    }
}

/* these functions will be filled in later */

int get_defaults(Char* str __attribute__((unused)),
		 int* len __attribute__((unused)),
		 int max __attribute__((unused)))
{
    return SUCCESS+FREE_BUFFER;
}

int get_document(Char* str, int* len, int max)
{
    return get_node(str,len,max);
}

int get_fonts(Char* str __attribute__((unused)),
	      int* len __attribute__((unused)),
	      int max __attribute__((unused)))
{
    return SUCCESS+FREE_BUFFER;
}

int get_keyboard(Char* str __attribute__((unused)),
		 int* len __attribute__((unused)),
		 int max __attribute__((unused)))
{
    return SUCCESS+FREE_BUFFER;
}

int get_smacro(Char* str __attribute__((unused)),
	       int* len __attribute__((unused)),
	       int max __attribute__((unused)))
{
    return SUCCESS+FREE_BUFFER;
}

int get_symbol(Char* str __attribute__((unused)),
	       int* len __attribute__((unused)),
	       int max __attribute__((unused)))
{
    return SUCCESS+FREE_BUFFER;
}

Char oldtonew(Char c)
{
  if (loading_old) {
    AutoLoad(oldfileremap);
    return MapValue(oldfileremap, c);
  } else {
    return c;
  }
}
