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
**  output.c
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "message.h"
#include "output.h"
#include "buffer.h"
#include "edit.h"
#include <ctype.h>
#include <memory.h>
#include "intstack.h"


#define MAXSIZE    5000

#define NR_CURSOR     3
#define NR_MARK       4
#define NR_CURBAR     12
#define MAX_SIGN_SIZE 13
#define CWH CURSORWIDTH/2
#define CHH CURSORHEIGHT/2
#define CURSORASCENT  192

static XPoint cursor_h[] = {{0, 0},    {CWH,   CHH},   {-CWH,   CHH}};
static XPoint cursor[]   = {{0, 0},    {CWH+1, CHH+1}, {-CWH-1, CHH+1}};
/*static XPoint mark_h[]   = {{0, -CHH}, {CWH, 0}, {0, CHH}, {-CWH, 0}}; */
static XPoint mark[]     = {{0, -CHH}, {CWH, 0}, {0, CHH}, {-CWH, 0}};
static XPoint curbar[] = {{ -CWH,   CHH/2},
			  { -CWH,   0},
			  { -CWH/2, 0},
			  { -CWH/2, CURSORASCENT + CHH/2},
			  { -CWH,   CURSORASCENT + CHH/2},
			  { -CWH,   CURSORASCENT },
			  {  CWH,   CURSORASCENT },
			  {  CWH,   CURSORASCENT + CHH/2},
			  {  CWH/2, CURSORASCENT + CHH/2},
			  {  CWH/2, 0},
			  {  CWH,   0},
			  {  CWH,   CHH/2 }};

static XPoint old_cursor_points[MAX_SIGN_SIZE];
static int old_cursor_kind;
static Bool active_target_cursor = MP_False;

static XPoint *sign[] = { cursor, curbar, cursor_h, mark };
static int sign_size[] = { NR_CURSOR, NR_CURBAR, NR_CURSOR, NR_MARK };
static Bool sign_filled[] = { MP_True, MP_True, MP_False, MP_False };
static Bool sign_convex[] = { Convex, Nonconvex, Convex, Convex};

#define SYMBOL_TAB(c)      (symbol_tab[Num2Tab(c)])
#define SYMBOL_PH(c)       (symbol_ph[Ph2Num(c)])
#define TEXTOPEN           (183)
#define TEXTCLOSE          (183)

#define LASTFONT(A)      (A)


static
char *symbol_tab[44] = { "",   "[set]", "[tab]", "[back]",
			       "[plus]", "[minus]", "[push]", "[pop]",
			       "[NwLn]", "[TEXT]", "[text]", "[math]",
			       "[disp]", "]", "[Hide:", "]",
			       "[Top]", "[top]", "[Bot]", "[bot]",
			       "[Gap]", "[gap]", "[Fill]", "[Line]", "[Dots]",
			       "[StackB:", "[StackC:", ":", ":", "]", "]",
			       "[Tabbing:", "]", "[Display:", "]", "[Bar]",
			       "[Space]", "[MATH]", "[BOTH]", "[Name:", "]",
		               "[Color:", ":", "]" };
static
Char symbol_ph[7] = {  'E', 'O', 'I', 'V', 'T', 'L', 'D'         };

#define FONTSYMBOL  "["
#define SIZESYMBOL  "[SizeA"
#define RELSIZESYMBOL "[SizeR"

static Window output_win = 0;
static Window message_win = 0;
static Window test_win = 0;
static Bool drindex = MP_False;
static Char messtxt[400];
static Char permanentmess[400];
static short messlen = 0;
static Bool messcurs = MP_False;
static int display_delta = 0;
static int def_thinspace=0;

#define PLACENAMESIZE 80
static Char placename[PLACENAMESIZE+1];
static int placepos=0;
static int nameset=0;
static int inplacename=0;

#define COLORNAMESIZE 80
static Char colorname[COLORNAMESIZE+1];
static int colorpos=0;
static int colorset=0;
static int incolorname=0;

static int base_x;
static int base_y;
static int move_x;
#define move_y  line_height()

/* for interpreting the old font and size markup codes */
static int sizeattrib=0;
static int defaultsizeval=0;
static struct {
  int fnr;
  int len;
  struct {
    char *grpname;
    int group;
    char *valname;
    int value;
  } attr[4];
} oldfontattr[] = {
  { 0, 3, { { "Series",0,"Medium",0},{"Shape",0,"Upright",0},{"Family",0,"Roman",0}}},
  { 1, 1, { { "Series",0,"Bold",0 }}},
  { 3, 1, { { "Shape", 0, "Italic", 0}}},
  { 5, 2, { { "Series",0,"Bold",0 }, { "Shape", 0, "Italic", 0}}},
  { 6, 1, { { "Shape",0,"Slanted",0}}},
  { 7, 1, { { "Family",0,"SansSerif",0}}},
  { 8, 1, { { "Family",0,"TypeWriter",0}}},
  { 0,0}
};
static char *oldfont_name[256] = {
  "Roman","Bold","Symbol", "Italic", "MathSymbol","Bold Italic", "Slanted",
  "Sans Serif", "Typewriter", 0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  "CMSY",0,0,0,0,0,0,0,0,0
};


static int maxlen;
static XChar2b *string;
static int maxitem;
static XTextItem16 *items;
static int initem;
static int limitchar;
static int lastfont=LASTFONT(-1);
static int instring;
static int left_margin;
static int smart_mode = INVISIBLE;
static int smart_height = 0;
static int search_x, search_y;
static void (*search_func)(void*) = NULL;
static void (*cursor_func)(void*,int,int) = NULL;
static Bool can_backtab;

static int base_color = 0;
static int base_style = 0;
static int identifier_font = 3;
static int hiding = 0;

#define TextBox  1
#define BackBox   2
#define DefaultBox 3
#define NodeBox    4
#define NewlineBox 5
#define StackCenterBox 6
#define StackBaseBox 7
#define BarBox 8
#define ColorBox 9

#define IsStackBox(A) ((A)==StackCenterBox || (A)==StackBaseBox)

#define GapBox 16
#define TopBox 17
#define BottomBox 18
#define IsFieldBox(A) (((A)&0x1F0)==0x10)

#define StippleBox 32
#define LineBox    33
#define SpaceBox   34
#define IsLineBox(A)  (((A)&0x1F0)==0x20)

#define CursorBox 64
#define IsCursorBox(A)  (((A)&0x1F0)==0x40)
#define CursorKind(A)   ((A)&0x03)

#define EmptyBox(A) (IsFieldBox(A)||IsLineBox(A)||IsCursorBox(A)|| \
		     (A)==NodeBox||(A)==NewlineBox)

typedef struct BOX BOX;

struct BOX {
    void *funcarg;
    short width, ascent, descent, x, y, itemnr,
          itemtot, lasc, ldesc, lright, maxwidth,
          special,fattrib,cattrib;
    char color, style;
    BOX *fbox, *lbox, *nbox;
};

static BOX *freebox = NULL;
static BOX *boxstack[1000];
static void *funcargs[1000];
static BOX *cursorstack = NULL;
static INTSTACK *savestack=NULL;
static int bsd = 0;

INTSTACK *attribstack = NULL;

#define pushattrib(A) push_int(&attribstack,A)
#define popattrib()   pop_int(&attribstack)
#define headattrib()  head_int(attribstack)

static void init_item(int i)
{
    items[i].chars  = &string[instring];
    items[i].nchars = 0;
    items[i].delta  = 0;
    items[i].font   = None;
    limitchar = maxlen - i*2;
}

static char spaces[1000];

static void print_box(BOX *b, int d)
{
    BOX *h;

    spaces[d]='\0';
    fprintf(stdout,
	    "%s(X%i,Y%i)(W%i,H%i):(A%i,D%i)[A%i,D%i,R%i]:S%i:[FA%i,CA%i]\n",
	    spaces, b->x, b->y, b->width, b->ascent+b->descent,b->ascent,
	    b->descent, b->lasc, b->ldesc, b->lright, b->special,
	    b->fattrib,b->cattrib);
    spaces[d]=' ';
    h = b->fbox;
    while (h) {
	print_box(h, d+1);
	h = h->nbox;
    }
}

static BOX *new_box(void)
{
    BOX *t=freebox;
    if (t)
	freebox = freebox->nbox;
    else
	t = (BOX*) malloc(sizeof(BOX));
    memset(t,0,sizeof(BOX));
    return t;
}

static void free_box(BOX *b)
{
    BOX *h;

    if (!b) return;
    h=b;
    b->lbox = NULL;
    while (h) {
	if (h->fbox) {
	    h->fbox->lbox = h;
	    h = h->fbox;
	} else if (h->nbox) {
	    h->nbox->lbox = h->lbox;
	    h = h->nbox;
	} else if (h->lbox) {
	    h->nbox = freebox;
	    freebox = h->lbox->fbox;
	    h = h->lbox;
	    h->fbox = NULL;
	} else {
	    h->nbox = freebox;
	    freebox = b;
	    h = NULL;
	}
    }
}

static void close_box(void);
static int split_text=MP_False;
static int after_node=MP_False;

static void close_textbox(void)
{
    if (bsd && boxstack[bsd-1]->special==TextBox)
	close_box();
}

static BOX *open_box(int special)
{
    BOX *b;

    if (bsd>=1000) return NULL;
    if (bsd && special==TextBox && boxstack[bsd-1]->special==TextBox)
	return boxstack[bsd-1];
    close_textbox();
    b = new_box();

    boxstack[bsd] = b;
    b->color = (char)base_color;
    b->style = (char)base_style;
    if (bsd) {
	b->fattrib = boxstack[bsd-1]->fattrib;
	b->cattrib = boxstack[bsd-1]->cattrib;
	b->x = boxstack[bsd-1]->width;
	b->funcarg = boxstack[bsd-1]->funcarg;
	b->lasc = boxstack[bsd-1]->lasc;
	b->ldesc = boxstack[bsd-1]->ldesc;
	if (special==TextBox) {
	    b->lright = boxstack[bsd-1]->lright;
	    boxstack[bsd-1]->lright = 0;
	}
    }
    b->special = (short)special;
    switch (special) {
    case DefaultBox:
	b->ascent = (short)font_ascent();
	b->descent = (short)font_descent();
	b->lasc = (short)font_ascent();
	b->ldesc = (short)font_descent();
	break;
    default:
	b->ascent = -200;
	b->descent = -200;
	break;
    }
    bsd++;
    return b;
}

static int count_lines(BOX *b)
{
    int i=0;
    if (!b) return 0;
    b=b->fbox;
    while (b) {
	i += (IsLineBox(b->special));
	b=b->nbox;
    }
    return i;
}

static void expand_gap(BOX *gap, int width)
{
    if (!gap || gap->width>=width)
	return;
    else {
	BOX *h;
	int nr;
	int i,j,w;

	nr = count_lines(gap);
	w = width-gap->width;
	if (!nr)
	    gap->x = (short)(w/2);
	else {
	    j=0;
	    h=gap->fbox;
	    i=0;
	    while (h) {
		h->x=(short)j;
		if (IsLineBox(h->special)) {
		    j+= h->width = (short)((w+i)/nr);
		    i++;
		    h->y = (short)((gap->ascent-gap->descent)/2);
		    if (h->special!=SpaceBox) {
			switch (gap->ascent-gap->descent) {
			case 0: gap->ascent++;
			case 1: gap->descent++;
			case 2: gap->ascent++;
			default: break;
			}
		    }
		    h->ascent = h->descent=0;
		} else
		    j+= h->width;
		h = h->nbox;
	    }
	    gap->width = (short)width;
	}
    }
}

static void add_space_boxes(BOX *b, int width)
{
    int i = count_lines(b);
    BOX *h;

    if (!b || b->width>=width) return;
    if (!i) {
	/* add space right */
	h = new_box();
	h->special=SpaceBox;
	h->color = b->color;
	h->style = b->style;
	if (b->lbox) b->lbox->nbox=h; else b->fbox = h;
	b->lbox = h;
	/* add space left */
	h=new_box();
	h->special=SpaceBox;
	h->color = b->color;
	h->style = b->style;
	h->nbox = b->fbox;
	b->fbox = h;
    }
    expand_gap(b,width);
}

static void close_box(void)
{
    BOX *b = (bsd ? boxstack[--bsd] : NULL);
    BOX *c = (bsd ? boxstack[bsd-1]: NULL);
    int la,ld, i;

    if (b) {
	boxstack[bsd]=NULL;
	if (IsStackBox(b->special)) {
	    if (!b->fbox)
		b->width = 0;
	    else {
		BOX *h = b->fbox;
		BOX *gap = NULL;
		BOX *bot = NULL;
		BOX *top = NULL;
		BOX *g=NULL;
		BOX *l=b;
		int tw = 0;
		int th = 0;
		int pos[4];

#define SET_BOX(A) if (!A && h->fbox) { A = h;if (tw<h->width) tw=h->width;} else g=h

		while (h) {
		    switch (h->special) {
		    case TopBox:
			SET_BOX(top);
			break;
		    case BottomBox:
			SET_BOX(bot);
			break;
		    case GapBox:
			SET_BOX(gap);
			break;
		    default:
			if (!IsCursorBox(h->special) &&
			    h->special!=NewlineBox)  g=h;
			break;
		    }
		    h=h->nbox;
		    if (g) {
			g->nbox = NULL;
			if (l==b) b->fbox = h; else l->nbox=h;
			free_box(g);
			g = NULL;
		    } else if (l==b) l = l->fbox; else l=l->nbox;
		}
		for (i=0; i<4; pos[i++]=0);
		pos[0]=line_space/2;
		if (top || gap || bot) {
		    b->width = (short)tw;
		    add_space_boxes(top, tw);
		    add_space_boxes(gap, tw);
		    add_space_boxes(bot, tw);
		    if (top) {
			pos[0]=top->ascent;
			if (!gap && bot) top->descent+=(short)line_space;
			pos[1]=top->descent;
		    }
		    pos[3]=pos[0]+pos[1];
		    if (gap) {
			if (top) gap->ascent+=(short)line_space;
			if (bot) gap->descent+=(short)line_space;
			pos[3]+=(gap->ascent-gap->descent)/2;
			pos[1]+=gap->ascent;
			pos[2]=gap->descent;
		    }
		    if (bot) {
			pos[2]+=bot->ascent;
			th=bot->descent;
		    }
		    pos[1]+=pos[0];
		    pos[2]+=pos[1];
		    th+=pos[2];
		    b->y = 0;
		    if (c && b->special==StackCenterBox) {
		      /* possible bug: incorrect font information */
			b->y =
			  (short)((char_ascent('x')-
			   char_descent('x'))/2);
		    }
		    b->ascent = (short)pos[1];
		    b->descent = (short)(th - b->ascent);
		    if (top) top->y = (short)(b->ascent-pos[0]);
		    if (gap) gap->y = (short)(b->ascent-pos[1]);
		    if (bot) bot->y = (short)(b->ascent-pos[2]);
		} else
		    b->width = 0;
	    }
	    move_x = 0;
	    i=0;
	    while (i<bsd) move_x+=boxstack[i++]->x;
	    move_x+=b->x+b->width;
	} else if (b->ascent < -b->descent && !IsCursorBox(b->special)) {
	    b->ascent = 0;
	    b->descent = 0;
	}
	if (b->special==BarBox) {
	    move_x+=3;
	    b->width=3;
	}
	if (IsFieldBox(b->special)) {
	    move_x = 0;
	    i=0;
	    while (i<bsd) move_x+=boxstack[i++]->x;
	    move_x+=b->x; /* +b->width; */
	}
	if (c) {
	    if (b->width || EmptyBox(b->special) || b->fbox) {
		la = b->y+b->ascent;
		if (la>c->ascent) c->ascent = (short)la;
		ld = -b->y+b->descent;
		if (ld>c->descent) c->descent = (short)ld;
		if (b->special==TextBox) {
		    c->ldesc = b->ldesc;
		    c->lasc = b->lasc;
		    if (!split_text) {
			b->width+=b->lright;
			move_x+=b->lright;
		    } else
			c->lright = b->lright;
		    c->fattrib = b->fattrib;
		    c->cattrib = b->cattrib;
		} else if (!IsCursorBox(b->special)) {
		    c->lasc = (short)la;
		    c->ldesc = (short)ld;
		}
		if (b->maxwidth > b->width) {
		    move_x = move_x + b->maxwidth-b->width;
		    b->width = b->maxwidth;
		}
		if (!c->lbox)
		    c->fbox = c->lbox = b;
		else {
		    c->lbox->nbox = b;
		    c->lbox = b;
		}
		if (!IsStackBox(c->special))
		    c->width+= b->width;
	    } else {
		if (b->special==TextBox) {
		    if (!split_text) {
			c->width+=b->lright;
			move_x+=b->lright;
		    } else
			c->lright=b->lright;
		    c->fattrib = b->fattrib;
		    c->cattrib = b->cattrib;
		}
		free_box(b);
	    }
	}
    }
}

static BOX *close_boxes(void)
{
    int i;
    BOX *c;

    free_int(savestack);
    savestack = NULL;
    for (i=bsd-1; i>=0; i--) {
	push_int(&savestack, boxstack[i]->fattrib);
	push_int(&savestack, boxstack[i]->cattrib);
	push_int(&savestack, boxstack[i]->special);
	funcargs[i] = boxstack[i]->funcarg;
    }
    c = boxstack[0];
    while (bsd>0) close_box();
    return c;
}

static void open_boxes(void)
{
    int i,j=0;
    BOX *b;

    while (aig(i=pop_int(&savestack))) {
	b = open_box(i);
	b->cattrib = (short)pop_int(&savestack);
	b->fattrib = (short)pop_int(&savestack);
	b->funcarg = funcargs[j++];
    }
}

static void put_sign(Window win, int x, int y, int kind, int ascent);

static void draw_cursors(void)
{
    BOX *h = cursorstack;
    while (h) {
	put_sign(output_win, h->x, h->y, CursorKind(h->special), h->ascent);
	cursorstack = h->fbox;
	h->fbox = NULL;
	h = cursorstack;
    }
}

static int draw_box(BOX *b, int x, int y, int ascent, int descent,int code)
{
    BOX *h=b->fbox;
    int ncode=0,i;
    int nl=0;

    x = x+b->x;
    y = y-b->y;
    ascent -=b->y;
    descent += b->y;
    switch (b->special) {
    case TopBox:
	if (code&0x3) descent = b->descent;
	break;
    case GapBox:
	if (code&0x1) descent = b->descent;
	if (code&0x4) ascent = b->ascent;
	break;
    case BottomBox:
	if (code&0x6) ascent = b->ascent;
	break;
    default:
	if (IsCursorBox(b->special)) {
	    b->x = (short)x;
	    b->y = (short)y;
	    b->ascent= ascent;
	    b->fbox = cursorstack;
	    cursorstack = b;
	} else if (IsStackBox(b->special)) {
	    while (h) {
		switch (h->special) {
		case GapBox: ncode |= 0x2; break;
		case TopBox: ncode |= 0x4; break;
		case BottomBox: ncode |= 0x1; break;
		default: break;
		}
		h = h->nbox;
	    }
	    h = b->fbox;
	}
	break;
    }
    switch (b->style) {
    case SMART:
	if (smart_mode != VISIBLE) break;
    case VISIBLE:
	if (!b->fbox && b->width)
	    XFillRectangle(display, output_win,
			   get_GC(Reverse, b->color, b->cattrib),
			   x, y-ascent, b->width, ascent+descent);
	switch (b->special) {
	case TextBox:
	    if (b->itemtot) {
		XDrawText16(display, output_win,
			    get_GC(Normal,b->color,b->cattrib), x,y,
			    items+b->itemnr, b->itemtot);
		undefined_font(Normal);
	    }
	    break;
	case LineBox:
	    if (b->width)
		XDrawLine(display, output_win,
			  get_GC(Normal, b->color, b->cattrib),
			  x, y-1, x+b->width, y-1);
	    break;
	case StippleBox:
	    if (b->width) {
		int j,yp = y-1;
		i=x;
		while (i<x+b->width) {
		    j = i+2;
		    if (j>x+b->width) j = x+b->width;
		    XDrawLine(display, output_win,
			      get_GC(Normal, b->color, b->cattrib),
			      i, yp, j, yp);
		    i+=5;
		}
	    }
	    break;
	case BarBox:
	    XDrawLine(display, output_win,
		      get_GC(Normal, b->color, b->cattrib),
		      x+1, y-ascent, x+1, y+descent);
	    break;
	default:
	    break;
	}
	if (!b->fbox && aig(i=must_underline(b->color)) && b->special!=GapBox
	    && b->width && !IsStackBox(b->special)) {
	    if (i!=2)
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-2, x+b->width-1, y+descent-2);
	    if (i>1) {
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-1, x+b->width-1, y+descent-1);
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-3, x+b->width-1, y+descent-3);
	    }
	}
	break;
    case SHADES:
	if (b->color && !b->fbox && b->width)
	    XFillRectangle(display, output_win, get_GCXor(b->color),
			   x, y-ascent, b->width, ascent+descent);
	if (!b->fbox && aig(i=must_underline(b->color)) &&
	    b->special!=GapBox && b->width && !IsStackBox(b->special)) {
	    if (i!=2)
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-2, x+b->width-1, y+descent-2);
	    if (i>1) {
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-1, x+b->width-1, y+descent-1);
		XDrawLine(display, output_win, get_GCXor(PrimSel),
			  x, y+descent-3, x+b->width-1, y+descent-3);
	    }
	}
	break;
    default:
	break;
    }
    while (h) {
	nl+=draw_box(h, x, y, ascent, descent,ncode);
	h = h->nbox;
    }
    if (b->special==NewlineBox) nl++;
    if (search_func && b->funcarg && search_x>=x && search_x<x+b->width &&
	search_y>=y-ascent && search_y < y+descent) {
	(*search_func)(b->funcarg);
	search_func = NULL;
    }
    return nl;
}

static void box_add_char(Char c)
{
    BOX *b;

    b = open_box(TextBox);
    if (!b->width) {
	b->itemnr = (short)initem;
	init_item(initem++);
	b->itemtot = 1;
    }
    /* No font encoding, thus not needed
    if (Char2Font(c)==TEXTFONT)
	c = (Char)Font2Char(b->fontnr, Char2ASCII(c));
    */
    /* should get character information directly, to increase performance */
    if (char_width(c)) {
        int tf = LASTFONT(b->fattrib);
	Font fid;
	int i = b->itemnr+b->itemtot-1;
	int j=0;
	CharInfo *ci;
	ci = character_info(c);
	fid = font_ID(ci);
	if ((tf!=lastfont || fid!=items[i].font) && items[i].font!=None) {
	    b->itemtot++;
	    init_item(initem);
	    initem++;
	    i++;
	}
	if (items[i].font==None) {
	    j = (!split_text||tf!=lastfont
		 ? b->lright+(CharLeft(ci->sysinfo)<0
			      ?-CharLeft(ci->sysinfo)
			      :0)
		 : 0 );
	    items[i].font = font_ID(ci);
	    items[i].delta += j;
	}
	split_text = MP_False;
	lastfont = tf;
	string[instring].byte1 = (ci->pos&0xff00)>>8;
	string[instring].byte2 = (ci->pos&0xff);
	instring++;
	items[i].nchars++;
	b->lasc = (short)CharAscent(ci->sysinfo);
	b->ldesc = (short)CharDescent(ci->sysinfo);
	b->lright = (short)(CharRight(ci->sysinfo)>CharWidth(ci->sysinfo)
			    ?CharRight(ci->sysinfo)-CharWidth(ci->sysinfo)
			    :0);
	j+= CharWidth(ci->sysinfo);
	b->width += (short)j;
	move_x += j;
	if (b->ascent<b->lasc) b->ascent= b->lasc;
	if (b->descent<b->ldesc) b->descent = b->ldesc; 
    }
}

static void box_neg_space(int space)
{
    BOX *b = open_box(BackBox);
    BOX *c = (bsd>1? boxstack[bsd-2]:NULL);

    b->width = (short)space;
    b->x-= (short)space;
    if (c) {
	if (c->maxwidth<c->width)
	    c->maxwidth = c->width;
	c->width-=(short)space;
    }
    close_box();
    if (c) {
      c->width-=(short)space;
      move_x-=space;
    }
}

static void box_space(int space)
{
    BOX *b = open_box(TextBox);

    if (!b->itemtot) {
	b->itemnr = (short)initem;
	init_item(initem++);
	b->itemtot=1;
    }
    if (items[b->itemnr+b->itemtot-1].nchars) {
	init_item(initem++);
	b->itemtot++;
    }
    if (b->lright) space+= b->lright;
    b->lright = 0;
    items[b->itemnr+b->itemtot-1].delta+=space;
    b->width+=(short)space;
    move_x+=space;
}

static void switch_size_fontnr(int fattrib)
{
    BOX *b = open_box(TextBox);

    if (fattrib!=b->fattrib) {
	if (!b->itemtot) b->itemnr = (short)initem;
	init_item(initem++);
	b->itemtot +=1;
        items[b->itemnr+b->itemtot-1].delta+=b->lright;
	b->width+=b->lright;
	move_x+=b->lright;
	b->lright = 0;
	b->fattrib = (short)fattrib;
    }
}

static void restore_fattrib(int attrib)
{
  BOX *b = open_box(TextBox);
  font_set_attributes(attrib);
  if (attrib!=b->fattrib) {
    switch_size_fontnr(attrib);
  }
}

static void switch_fattrib(int attrgroup, int attrval)
{
  int cav;
  (void) open_box(TextBox);
  cav = font_get_attribute(attrgroup);
  if (cav!=attrval) {
    font_set_attribute(attrgroup, attrval);
    switch_size_fontnr(font_get_attributes());
  }
}

static void switch_sizenr(int sizenr)
{
    int csa;
    (void) open_box(TextBox);
    csa = font_get_attribute(sizeattrib)-defaultsizeval;
    if (sizenr!=csa) {
      font_set_attribute(sizeattrib, defaultsizeval+sizenr);
      switch_size_fontnr(font_get_attributes());
    }
}

static void switch_fontnr(int fontnr)
{
    int cfa= -1;
    int i,j;
    (void) open_box(TextBox);
    for (j=0; oldfontattr[j].len && oldfontattr[j].fnr!=fontnr; j++);
    i=0;
    while (i<oldfontattr[j].len) {
      if (font_get_attribute(oldfontattr[j].attr[i].group) !=
	  oldfontattr[j].attr[i].value) {
	font_set_attribute(oldfontattr[j].attr[i].group,
			   oldfontattr[j].attr[i].value);
	cfa = font_get_attributes();
      }
      i++;
    }
    if (cfa!= -1) {
      switch_size_fontnr(cfa);
    }
}

static void box_color(int cattrib)
{
  BOX *b = open_box(ColorBox);
  b->cattrib=cattrib;
}

static void pushtabs(void);
static void poptabs(void);
static void endline(void);
static void settab(void);
static void rtab(void);
static void ltab(void);
static void tabplus(void);
static void tabminus(void);

static int bufcount = 0;
int saved_chars = 0;

int more_keys(void)
{
    XEvent report;

    saved_chars = bufcount;
    if (bufcount>10 || !XEventsQueued(display, QueuedAfterFlush))
	return (bufcount = 0);
    XPeekEvent(display, &report);
    while (report.type == KeyRelease) {
	XNextEvent(display, &report);
	if (XEventsQueued(display,QueuedAfterFlush))
	    XPeekEvent(display, &report);
	else
	    return (bufcount = 0);
    }
    if (report.type == KeyPress)
	return ++bufcount;
    else
	return (bufcount=0) ;
}

void set_message_window(void *w)
{
   message_win = *((Window *) w);
}

void draw_message(void)
{
    if (message_win) {
	XClearWindow(display, message_win);
	push_fontgroup(POPUPFONT);
	/* direct draw needed (without box structures) */
	if (messlen) {
	    XDrawString(display, message_win,
			get_GC_font(Normal,0,0,font_ID(character_info(' '))),
			0, font_ascent() + line_space/2, UstrtoLocale(messtxt), messlen);
	}
	if (messcurs) {
	    put_sign(message_win, string_width(messtxt, messlen),
		     font_ascent()+line_space/2, 0,font_ascent());
	}
	XFlush(display);
	pop_fontgroup();
    }
}

void out_message(Char *txt)
{
    if (message_win) {
	Ustrncpy(messtxt, txt, 400);
	messlen = Ustrlen(messtxt);
	messcurs = MP_False;
	draw_message();
    }
}

void out_message_curs(Char *txt)
{
    if (message_win) {
        Ustrncpy(messtxt, txt, 400);
	messlen = Ustrlen(messtxt);
	messcurs = MP_True;
	draw_message();
    }
}

void clear_message(Bool allways)
{
    if (allways || !messcurs) {
	Ustrncpy(messtxt, permanentmess, 400);
	messlen = Ustrlen(messtxt);
	messcurs = MP_False;
	if (message_win) draw_message();
    }
}

void out_permanent_message(Char *txt)
{
    Ustrncpy(permanentmess, txt, 399);
    clear_message(MP_False);
}

void *test_window(void)
{
    return &test_win;
}

void set_output_window(void *w)
{
    if (output_win || bsd) {
	BOX *c;
	fprintf(stderr, "Unset Window forgotten...\n");
	c = close_boxes();
	free_box(c);
	free_int(savestack);
	savestack = NULL;
	search_func = NULL;
    }
    output_win = *((Window *) w);
    /* SetCanvas(output_win); */
    lastfont = LASTFONT(-1);
    def_thinspace=0;
    base_x = base_y = move_x = 0;
    left_margin = 0;
    can_backtab = MP_True;
    drindex=MP_False;
    bsd = 0;
    initem = 0;
    instring = 0;
    defaultsizeval=font_get_attribute(sizeattrib);
    placepos=nameset=inplacename=0;
    colorpos=colorset=incolorname=0;
    free_int(attribstack);
    attribstack = NULL;
    pushattrib(font_get_attributes());
    base_style = VISIBLE;
    base_color = 0;
    split_text = 0;
    hiding = 0;
    open_box(DefaultBox);
}

void adjust_lineheight(void)
{
  int i;
  for (i=0; i<bsd && boxstack[i]->special!=DefaultBox; i++);
  if (i<bsd) {
    boxstack[i]->ascent=(short)font_ascent();
    boxstack[i]->descent = (short)font_descent();
    boxstack[i]->lasc = (short)font_ascent();
    boxstack[i]->ldesc = (short)font_descent();
  }
}

static void resettabs(void);

void unset_output_window(void)
{
    BOX *c;
    flush();
    c = close_boxes();
    free_box(c);
    free_int(savestack);
    savestack = NULL;
    search_func = NULL;
    output_win = 0;
    resettabs();
}

void set_drawstyle(int style)
{
    split_text = !after_node;
    close_textbox();
    base_style = style;
}

void set_underline(Bool b)
{
    split_text = !after_node;
    close_textbox();
    if (b) {
	out_char(StackB);
	out_char(TopGap);
    } else {
	out_char(GapBottom);
	out_char(GlueLine);
	out_char(StackClose);
    }
}

void set_italic(Bool b)
{
    if (b) {
	if (b>MP_True) {
	    switch_fontnr(b-1);
	} else {
	    switch_fontnr(identifier_font);
	}
	pushattrib(font_get_attributes());
    } else {
	popattrib();
	restore_fattrib(headattrib());
    }
}

int next_id_font(int n)
{
    int i;
    i=n+1;
    while (i!=n) {
	if (!i) i++;
	if (font_openmathtex(i) || font_opentexttex(i)) return i;
	if (i==255) i=0; else i++;
    }
    return -1;
}

void set_default_thinspace(int n)
{
    def_thinspace=n&0xff;
}

int  get_default_thinspace(void)
{
  return def_thinspace;
}

void set_index(Bool b)
{
    drindex = b;
}

void set_text_mode(int mode)
{
    split_text = !after_node;
    close_textbox();
    if ((mode && !(base_color&PrimSel)) || (!mode && base_color&PrimSel))
	base_color ^= PrimSel;
}

void switch_thick(void)
{
    split_text = !after_node;
    close_textbox();
    base_color ^= SecondSel;
}

void switch_thin(void)
{
    split_text = !after_node;
    close_textbox();
    base_color ^= ThirdSel;
}

void switch_reverse(void)
{
    split_text = !after_node;
    close_textbox();
    base_color ^= PrimSel;
}

void switch_visible(void)
{
    split_text = !after_node;
    close_textbox();
    if (base_style==VISIBLE) {
      base_style=INVISIBLE;
    } else if (base_style==INVISIBLE) {
      base_style=VISIBLE;
    }
}

void out_clear(void)
{
    if (output_win)
	XClearWindow(display, output_win);
    base_x = 0;
    base_y = 0;
}

void set_margin(int newmargin)
{
    left_margin = newmargin;
    can_backtab = MP_True;
}

void set_smart_height(int height)
{
    smart_height = height;
}

void set_search_func(void (*func)(void*), int x, int y)
{
    search_func = func;
    search_x = x+left_margin;
    search_y = y;
}

void set_cursor_func(void (*func)(void*,int,int))
{
    cursor_func = func;
}

void detect_margin(void)
{
    WINDOWTYPE wtype,ptype;
    Window wp,wpp;
    void *data;
    int i;

    wtype = get_window_type(output_win, &wp, &data);
    if (!data)
	ptype = get_window_type(wp, &wpp, &data);
    if (!data) data = (void*) &output_win;
    if (eventfunc[wtype]->margin)
	i = (*(eventfunc[wtype]->margin))(data);
    else
	i = 0;
    left_margin = i;
    can_backtab = MP_True;
}

int where_x(void)
{
  return base_x+move_x;
/*    int i=0,x=0;
    while (i<bsd) {
	x+= boxstack[i]->x;
	i++;
    }
    if (i)
	x+=boxstack[i-1]->width;
    return base_x+x; */
}

int where_y(void)
{
    return base_y;
}

void set_x_y(int new_x, int new_y)
{
    flush();
    base_x = new_x;
    base_y = new_y;
}

void set_x(int new_x)
{
    base_x = new_x;
}

void set_y(int new_y)
{
    base_y = new_y;
}

int line_height(void)
{
    return font_height()+line_space;
}

static void clear_to_end_of_line(int x, int height)
{
    if (base_style==VISIBLE || base_style==SMART)
	XClearArea(display, output_win, x, base_y, 0, height, MP_False);
}

void clear_to_end_of_page(void)
{
    if (base_style==VISIBLE) {
	endline();
	XClearArea(display, output_win, 0, base_y, 0, 0, MP_False);
    }
}

static void clear_to_begin_of_line(int height)
{
  if (left_margin+base_x>0)
    XClearArea(display, output_win, 0, base_y, base_x+left_margin,
	       height, MP_False);
}

void move_content_up(int size, int height)
{
  if (output_win) {
    XCopyArea(display, output_win, output_win,  get_GC(Normal, 0, 0), 0,size,
	      display_width,height-size,
	      0,0);
  }
}
void move_content_down(int size, int height)
{
  if (output_win) {
    XCopyArea(display, output_win, output_win,  get_GC(Normal, 0, 0), 0,0,
	      display_width, height-size,
	      0,size);
  }
}

void flush(void)
{
    BOX *b = close_boxes();

    if (b && output_win) {
	b->ascent+=(short)(line_space/2);
	b->descent+=(short)(line_space/2+line_space%2);
	if (b->style==VISIBLE || b->style==SMART || b->style==SHADES)
	  clear_to_begin_of_line(b->ascent+b->descent);
	smart_mode = (b->ascent+b->descent!=smart_height ? VISIBLE: INVISIBLE);
	if (draw_box(b,base_x+left_margin, base_y+b->ascent,
		     b->ascent, b->descent,0)) {
	    clear_to_end_of_line(base_x+left_margin+b->width,
				 b->ascent+b->descent);
	    draw_cursors();
	    base_y+=b->ascent+b->descent;
	} else
	    draw_cursors();
    }
    initem = 0;
    instring = 0;
    init_item(0);
    smart_mode = INVISIBLE;
    smart_height = 0;
    boxstack[0] = NULL;
    if (b) free_box(b);
    open_boxes();
}

static void out_symbol(Char data)
{
    box_add_char(data);
}

void out_text_delim(int on)
{
    if (textdots && !hiding) {
	box_add_char(on ? (Char)TEXTOPEN : (Char)TEXTCLOSE);
	after_node=MP_False;
    }
}

void out_index(int c)
{
    char str[10];
    int i;

    if (!c) return;
    if (c<-500) c = -500;
    if (c>500) c = 500;
    sprintf(str, "%i",c);
    i = font_get_attribute(sizeattrib)-defaultsizeval-2;
    switch_sizenr(i);
    pushattrib(font_get_attributes());
    i=0;
    while (str[i]) {
	out_symbol((Char)str[i]);
	i++;
    }
    popattrib();
    restore_fattrib(headattrib());
    after_node=MP_False;
}

void out_cursor(int kind)
{
  if (base_style != INVISIBLE) {
    split_text = !after_node;
    open_box(CursorBox+kind);
    close_box();
  }
}

void out_cursor_select(int kind, void *selection)
{
  BOX *b;
  split_text = !after_node;
  b = open_box(CursorBox+kind);
  b->funcarg=selection;
  close_box();
}

void open_node(void* data)
{
    BOX *b;

    close_textbox();
    b = open_box(NodeBox);
    b->funcarg = data;
    after_node=MP_True;
    if (placepos) {
	nameset=1;
	placename[placepos]='\0';
	placepos=0;
    } else nameset=0;
}

void close_node(void)
{
    after_node=MP_False;
    while (bsd && boxstack[bsd-1]->special!=NodeBox) close_box();
    close_box();
    /* fonts and sizes should be changed */
    placepos=nameset=0;
}

void out_bold(Char *str)
{
    switch_sizenr(-1);
    pushattrib(font_get_attributes());
    switch_fontnr(1);
    pushattrib(font_get_attributes());
    out_string(str);
    popattrib();
    restore_fattrib(headattrib());
    popattrib();
    restore_fattrib(headattrib());
}

void out_char_bold(char *str)
{
    switch_sizenr(-1);
    pushattrib(font_get_attributes());
    switch_fontnr(1);
    pushattrib(font_get_attributes());
    out_char_string(str);
    popattrib();
    restore_fattrib(headattrib());
    popattrib();
    restore_fattrib(headattrib());
}

void out_char(Char data)
{
    if (hiding && !IsNewline(data) && data!=StartHide && data!=EndHide) return;
    if (inplacename) {
	if (placepos<PLACENAMESIZE && !(data&0xff00))
	    placename[placepos++]=(char)(data&0xff);
	else if (data==PlNameEnd) inplacename=0;
	return;
    }
    if (incolorname) {
	if (colorpos<COLORNAMESIZE && !(data&0xff00))
	    colorname[colorpos++]=(char)(data&0xff);
	else if (data==ColorSep) {
	  int cattrib;
	  incolorname=0;
	  colorname[colorpos]=0;
	  /* parse color */
	  cattrib=get_color(colorname);
	  /* open color box */
	  box_color(cattrib);
	  colorpos=0;
	}
	return;
    }
    switch (data) {
    case Newline:
    case 0x2029:
	endline();
	can_backtab = MP_True;
	break;
    case SoftNewline:
    case '\n':
    case 0xd:
    case 0xc:
    case 0x2028:
	endline();
	break;
    case Ltab:
	ltab();
	break;
    case Rtab:
	rtab();
	can_backtab = MP_False;
	break;
    case Settab:
	settab();
	can_backtab = MP_False;
	break;
    case Tabplus:
	tabplus();
	can_backtab = MP_False;
	break;
    case Tabminus:
	tabminus();
	can_backtab = MP_False;
	break;
    case Pushtabs:
	pushtabs();
	can_backtab = MP_False;
	break;
    case Poptabs:
	poptabs();
	can_backtab = MP_False;
	break;
    case AskText:
    case AskMath:
    case AskBoth:
    case InText:
    case InMath:
    case InDisp:
	break;
    case StartHide:
	hiding++;
	break;
    case EndHide:
	if (hiding) hiding--;
	break;
    case CloseStack:
    case CloseTop:
    case CloseGap:
    case CloseBottom:
	close_textbox();
	if (bsd>1) close_box();
	break;
    case StackClose:
	close_textbox();
	if (bsd>1) close_box();
	if (bsd>1) close_box();
	break;
    case TopGap:
	close_textbox();
	if (bsd>1) close_box();
	open_box(GapBox);
	break;
    case GapBottom:
	close_textbox();
	if (bsd>1) close_box();
	open_box(BottomBox);
	break;
    case StackC:
	open_box(StackCenterBox);
	open_box(TopBox);
	break;
    case StackB:
	open_box(StackBaseBox);
	open_box(TopBox);
	break;
    case VerLine:
	close_textbox();
	open_box(BarBox);
	close_box();
	break;
    case ThinSpace:
	thinspace(def_thinspace*screen_space);
	break;
    case PopSize:
	popattrib();
	restore_fattrib(headattrib());
	break;
    case OpenTop:
	open_box(TopBox);
	break;
    case OpenBottom:
	open_box(BottomBox);
	break;
    case OpenGap:
	open_box(GapBox);
	break;
    case GlueLine:
	open_box(LineBox);
	close_box();
	break;
    case GlueSpace:
	open_box(SpaceBox);
	close_box();
	break;
    case GlueStipple:
	open_box(StippleBox);
	close_box();
	break;
    case TabOpen:
	open_tabbing();
	break;
    case TabClose:
	close_tabbing();
	break;
    case DisplayOpen:
	open_display();
	break;
    case DisplayClose:
	close_display();
	break;
    case PlName:
	inplacename=1;
	placepos=0;
	break;
    case PlNameEnd:
	inplacename=0;
	break;
    case ColorStart:
	incolorname=1;
	break;
    case ColorSep:
	incolorname=0;
	colorname[colorpos]=0;
	/* ask color */
	{ int cattrib;
	  cattrib=get_color(colorname);
	  /* open color box */
	  box_color(cattrib);
	}
	colorpos=0;
	break;
    case ColorEnd:
	/* close color box */
	close_textbox();
	if (bsd>1) close_box();
	break;
    default:
	if (IsPh(data)) {
	    if (placepos || nameset) {
		if (!nameset) placename[placepos]='\0';
		out_bold(placename);
		placepos=nameset=0;
	    } else {
	      placename[0]=SYMBOL_PH(data);
	      placename[1]=0;
	      out_bold(placename);
	    }
	    if (drindex) out_index(Num(data));
	} else if (Char2Font(data)==SpaceFont) {
	    thinspace(Char2ASCII(data)*screen_space);
	} else if (Char2Font(data)==StackFont) {
	    open_box(Char2ASCII(data)==63?StackBaseBox:StackCenterBox);
	} else if (Char2Font(data)==FontFont) {
	    switch_fontnr(Char2ASCII(data));
	    pushattrib(font_get_attributes());
	} else if (Char2Font(data)==PopFont) {
	    popattrib();
	    restore_fattrib(headattrib());
	} else if (Char2Font(data)==SizeFont) {
	    int c = Char2ASCII(data);
	    if (c<128) {
		int i = font_get_attribute(sizeattrib)-defaultsizeval +c-64;
		switch_sizenr(i);
		pushattrib(font_get_attributes());
	    } else {
		switch_sizenr(c-192);
		pushattrib(font_get_attributes());
	    }
	} else if (Char2Font(data)==AttribFont) {
	  switch_fattrib(AttribGroup(data),AttribValue(data));
	  pushattrib(font_get_attributes());
	} else if (Char2Font(data)==AttrPopFont) {
	  popattrib();
	  restore_fattrib(headattrib());
	} else {
	    out_symbol(data);
	    can_backtab = MP_False;
	}
	break;
    }
    after_node=MP_False;
}

Bool display_tab(Char data)
{
    if (IsTab(data)) {
	out_bold(translate(SYMBOL_TAB(data)));
	return MP_True;
    } else {
	Char *c;
	int i, colon=MP_True,ind,fr=1;
	switch (Char2Font(data)) {
	case StackFont:
	    c = (Char2ASCII(data)==63?translate(SYMBOL_TAB(StackB)):
		       translate(SYMBOL_TAB(StackC)));
	    fr=0;
	    colon = MP_False;
	    break;
	case FontFont:
	    if (oldfont_name[Char2ASCII(data)])
		c = concat(translate("["), translate(oldfont_name[Char2ASCII(data)]));
	    else {
		c = translate("[Font??");
		fr=0;
	    }
	    break;
	case SizeFont:
	    if ((int)Char2ASCII(data)>128) {
		c = concat(translate(SIZESYMBOL),translate("xxxx"));
		i = Ustrlen(translate(SIZESYMBOL));
		ind = Char2ASCII(data)-192;
	    } else {
		c = concat(translate(RELSIZESYMBOL),translate("xxxx"));
		i = Ustrlen(translate(RELSIZESYMBOL));
		ind = Char2ASCII(data)-64;
	    }
	    {
	      Char lbuf[40];
	      Char *s;
	      lbuf[39]=0;
	      s=Ultostr(ind,lbuf+39);
	      Ustrcpy(c+i,s);
	    }
	    break;
	case PopFont:
	    c = translate("]");
	    fr = 0;
	    colon = MP_False;
	    break;
	case AttribFont:
	  {
	    static char buffer[1000];
	    sprintf(buffer,"[%s=%s", font_get_name(AttribGroup(data),-1),
		    font_get_name(AttribGroup(data),AttribValue(data)));
	    c=LocaletoUstr(buffer);
	    fr = 0;
	  }
	  break;
	case AttrPopFont:
	  c = translate("]");
	  fr=0;
	  colon=MP_False;
	  break;
	default:
	    return MP_False;
	}
	out_bold(c);
	if (colon) out_bold(translate(":"));
	if (fr) free(c);
	return MP_True;
    }
}


void out_char_string(char *str)
{
  int i;
  for (i=0; str[i]; i++) {
    box_add_char((unsigned)(str[i]));
  }
  can_backtab=MP_False;
}

void out_string(Char *str)
{
    int i, l = Ustrlen(str);

    for (i=0; i<l; i++)
	box_add_char(str[i]);
    can_backtab=MP_False;
}

void thinspace(int spacing)
{
    if (spacing)
	if (spacing >=0)
	    box_space(spacing);
	else
	    box_neg_space(-spacing);
    else
	close_textbox();
}

static void put_sign(Window win, int x, int y, int kind, int ascent)
{
    GC   tempgc;
    XPoint temp1[MAX_SIGN_SIZE];
    XPoint *temp2 = sign[kind];
    int nr = sign_size[kind];
    int i;

    for (i=0; i<nr; i++) {
	temp1[i].x = (short)(temp2[i].x+x);
	if (temp2[i].y>CURSORASCENT-15) {
	  temp1[i].y = (short)(temp2[i].y+y-CURSORASCENT-ascent);
	} else {
	  temp1[i].y = (short)(temp2[i].y+y);
	}
	old_cursor_points[i] = temp1[i];
	
    }
    old_cursor_points[i] = temp1[i] = temp1[0];
    old_cursor_kind = kind;
    tempgc = get_GCXor(PrimSel);
    if (sign_filled[kind])
	XFillPolygon(display, win, tempgc, temp1,
		     nr, sign_convex[kind], CoordModeOrigin);
    else
	XDrawLines(display, win, tempgc, temp1, nr, CoordModeOrigin);
    active_target_cursor = MP_True;
}

void remove_cursor(void)
{
    GC   tempgc;
    int nr = sign_size[old_cursor_kind];

    if (!output_win) return;
    tempgc = get_GCXor(PrimSel);
    if (sign_filled[old_cursor_kind])
      XFillPolygon(display, output_win, tempgc, old_cursor_points,
                   nr, sign_convex[old_cursor_kind], CoordModeOrigin);
    else
      XDrawLines(display, output_win, tempgc, old_cursor_points,
                 nr, CoordModeOrigin);
}

void put_mark(int x, int y)
{
    put_sign(output_win, x+left_margin, y, 1, font_ascent());
}

/*
**  tabbing functies
*/

#define MAXTAB 20

typedef struct TABBING TABBING;
struct TABBING {
                short stops[MAXTAB];
                unsigned char curtabmar;
                unsigned char hightab;
                unsigned char curtab;
                TABBING *prev;
               };

static TABBING *stacktop = 0, *freelist = 0;
static int freel=0;

static int tabsize = 24;

static int simpletabsize; /* = 8 * "width of an n" */

static int in_tab_env = 0;

/*
static void stopfunc(void)
{
    int i=5;
    while (i) i--;
}

static void check(void)
{
    int i=0;
    TABBING *h=freelist;

    if (!freelist && !freel) return;
    if (!freel || !freelist) { stopfunc(); return; }
    while (h) {
	i++;
	h=h->prev;
    }
    if (i!=freel) stopfunc();
}
*/
static void check(void) { }

#define GET_FREE(A) { if (freel) { A=freelist;freelist=A->prev;freel--; }\
                      else  A = (TABBING*) malloc(sizeof(TABBING)); check(); }
#define SET_FREE(A) { TABBING *h=A->prev; A->prev = freelist; freelist=A;\
		      freel++; A=h; check(); }

static void pushtabs(void)
{
    TABBING *h;

    if (!in_tab_env) return;
    GET_FREE(h);
    if (h) {
	if (stacktop)
	    *h = *stacktop;
	else {
	    /* open tabbing. */
	    h->stops[0] = 0;
	    h->hightab = h->curtab = h->curtabmar = 0;
	}
	h->prev = stacktop;
	stacktop = h;
    }
}

static void poptabs(void)
{
    if (!in_tab_env) return;
    if (stacktop && stacktop->prev) SET_FREE(stacktop);
}

void tab_unlock(void *ts)
{
    int i=1;
    TABBING *h = (TABBING*) ts;
    if (!ts) return;
    while (h->prev) { h=h->prev; i++; }
    h->prev=freelist;
    freelist=(TABBING*) ts;
    freel+=i;
    check();
    while (freel>100) {
	h=freelist; freelist=h->prev;
	free(h);
	freel--;
    }
    check();
}

void *tab_lock(void)
{
    TABBING *a,*b,*c;
    if (!stacktop) return NULL;
    GET_FREE(a);
    *a=*stacktop;
    b=a->prev;
    c=a;
    while (b) {
	GET_FREE(c->prev);
	c=c->prev;
	*c=*b;
	b=b->prev;
    }
    return (void*) a;
}

static void resettabs(void)
{
    while (stacktop) SET_FREE(stacktop);
    in_tab_env = 0;
}


void set_tab_stack(void *ts, int nropen)
{
    resettabs();
    stacktop = (TABBING*) ts;
    stacktop = (TABBING*) tab_lock();
    in_tab_env = nropen;
    if (in_tab_env)
	base_x = stacktop->stops[stacktop->curtab=stacktop->curtabmar];
    else
	base_x = 0;
    move_x = 0;
}

Bool tab_equal(void *ts)
{
    TABBING *a,*b;

    if (!ts && !stacktop) return MP_True;
    if (!ts || !stacktop) return MP_False;
    a= (TABBING*) ts;
    b= stacktop;
    while (a && b) {
	int i;
	if (a->curtabmar!=b->curtabmar || a->hightab!=b->hightab ||
	    a->curtab!=b->curtab || (!a->prev)^(!b->prev)) return MP_False;
	i=a->hightab+1;
	while (i) {
	    i--;
	    if (a->stops[i]!=b->stops[i]) return MP_False;
	}
	a = a->prev;
	b = b->prev;
    }
    return MP_True;
}

void open_tabbing(void)
{
    in_tab_env++;
    pushtabs();
}

void set_display_delta(int d)
{
    display_delta = d;
}

int get_display_delta(void)
{
    return display_delta;
}

void open_display(void)
{
    int i, j;

    open_tabbing();
    if (in_tab_env==1) {
	j = (display_delta+latex_side<0 ? 0 : display_delta+latex_side);
	for (i=0; i<j; i++) {
	    rtab();
	    tabplus();
	}
    }
    display_delta = 0;
}

void close_tabbing(void)
{
    if (in_tab_env) {
	in_tab_env--;
	if (in_tab_env)
	    poptabs();
	else
	    resettabs();
    }
}

void close_display(void)
{
    close_tabbing();
}

void settabsize(int newsize)
{
    tabsize = newsize;
}

int getsimpletabsize(void)
{
    return simpletabsize;
}

void setsimpletabsize(int newsize)
{
    simpletabsize = newsize;
}

void opspace(int size)
{
    thinspace(size * screen_space);
}

static void endline(void)
{
    close_textbox();
    open_box(NewlineBox);
    close_box();
    lastfont=LASTFONT(-1);
    flush();
    if (in_tab_env)
	base_x = stacktop->stops[stacktop->curtab=stacktop->curtabmar];
    else
	base_x = 0;
    move_x = 0;
}

static void settab(void)
{
    if (in_tab_env && (int)stacktop->curtab < MAXTAB-1) {
	stacktop->curtab++;
	stacktop->stops[stacktop->curtab] = (short) where_x();
	if (stacktop->hightab < stacktop->curtab) 
	    stacktop->hightab = stacktop->curtab;
    }
}

static void rtab(void)
{
    if (in_tab_env) {
	if ((int)stacktop->curtab < MAXTAB-1) {
	    stacktop->curtab++;
	    if (stacktop->curtab > stacktop->hightab) {
		thinspace(tabsize);
		stacktop->stops[stacktop->curtab] = (short)where_x();
		stacktop->hightab++;
	    } else
		thinspace(stacktop->stops[stacktop->curtab]-where_x());
	}
    } else
	thinspace(simpletabsize - where_x()%simpletabsize);
}

static void ltab(void)
{
    if (in_tab_env)
	if (stacktop->curtab)
	    if (can_backtab) {
		thinspace(stacktop->stops[stacktop->curtab-1]-where_x());
		stacktop->curtab--;
	    }
}

static void tabplus(void)
{
    if (in_tab_env) {
	if (stacktop->curtabmar < stacktop->hightab)
	    stacktop->curtabmar++;
    }
}

static void tabminus(void)
{
    if (in_tab_env) {
	if (stacktop->curtabmar)
	    stacktop->curtabmar--;
    }
}

int output_important(void)
{
  return (incolorname || inplacename);
}

void output_init(void)
{
    long i;

    permanentmess[0]='\0';
    i = (XMaxRequestSize(display) -4) *4;
    if (i>MAXSIZE) i=MAXSIZE;
    maxlen = i -2;
    if (!aig(string = (XChar2b *) malloc(maxlen*sizeof(XChar2b))))
	message(MP_EXIT -1, translate("Out of memory in output."));
    maxitem = i /3;
    if (!aig(items = (XTextItem16 *) malloc(maxitem * sizeof (XTextItem16))))
	message(MP_EXIT -1, translate("Out of memory in output."));
    initem = 0;
    limitchar = maxlen;
    instring = 0;
    init_item(initem);
    lastfont = LASTFONT(-1);
    boxstack[0] = NULL;
    simpletabsize = 8 * char_width('n');
    tabsize = screen_tab;
    test_win = XCreateSimpleWindow(display, root_window, 0,0,1,1,0,0,0);
    {
      int j,k;
      char *sizename;
      for (j=0; oldfontattr[j].len; j++) {
	for (k=0; k<oldfontattr[j].len; k++) {
	  int atp;
	  char *attname;
	  atp=0;
	  while (aig(attname=font_get_name(atp,-1)) &&
		 strcmp(attname,oldfontattr[j].attr[k].grpname)) atp++;
	  if (attname) {
	    int vap;
	    char *vaname;
	    oldfontattr[j].attr[k].group=atp;
	    vap=0;
	    while (aig(vaname=font_get_name(atp,vap)) &&
		   strcmp(vaname,oldfontattr[j].attr[k].valname)) vap++;
	    if (vaname) oldfontattr[j].attr[k].value=vap;
	  }
	}
      }
      j=0;
      while (aig(sizename=font_get_name(j,-1)) &&
	     strcmp(sizename,"Size")) j++;
      if (sizename) sizeattrib=j; else sizeattrib=2;
    }	      
}

