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
**  File   : sources.c
**  Date   : 27-3-92
**  Purpose: initialisation and termination of windows environment
**           bookkeeping of windows and event distribution
**           making fonts, GC's and colors available
**           project file handling
**           some general functions
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "message.h"
#include "notatype.h"
#include "output.h"
#include "remark.h"
#include "intstack.h"
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <signal.h>
#include <pwd.h>
#include <unistd.h>
#include "config.h"
#include "keyboard.h"

FUNCTIONS nowindowfunc = { NULL } ;
FUNCTIONS *eventfunc[MAXWINDOWTYPE] =
    { &nowindowfunc, &buttonfuncs,       &scrollbarfuncs, &stringfuncs,
      &menufuncs,    &maineditfuncs,     &editfuncs,      &mainbufferfuncs,
      &bufferfuncs,  &mainnotationfuncs, &notationfuncs,  &notadeffuncs,
      &symbolfuncs,  &defaultfuncs,      &remarkfuncs,    &findfuncs,
      &fileselcfuncs,&popupfuncs,        &checkboxfuncs };

Display *display;
Visual *visual=NULL;
Colormap colormap;
static int usegray=0;
static int def_depth=1;
static int visualclass=-1;
static char *displayname = NULL;
Bool iconic=MP_False, output_mode = MPTEX;
unsigned long save_time = 0;
int save_minute = 20;
char *wmselection = NULL;
char *latexselection = NULL;
unsigned long message_time = 0, last_time = 0;
int screen_num;
Window root_window;
unsigned int display_height, display_width;
unsigned long white_pixel, black_pixel;
XClassHint class_hints;
Atom WM_SAVE_YOURSELF, WM_DELETE_WINDOW;
Atom protocol[2];
XWMHints wm_hints;
Pixmap icon_pixmap;
static Cursor wait_cursor=None;

Char *project_name = NULL;

#define BUFSIZE 2000
static Char buffer[BUFSIZE];

#include "progicon"

static int read_integer_short(char **txt)
{
    int n=0;
    Bool neg=MP_False;

    if (**txt=='-') { neg=MP_True; (*txt)++;}
    while (isdigit(**txt)) {
	n=n*10+ (**txt)-'0';
	(*txt)++;
    }
    return (neg? -n:n);
}

static int read_integer(char **txt)
{
    int n=read_integer_short(txt);

    while (isspace(**txt)) (*txt)++;
    return n;
}

/*
**
**  Functions for GC's, fonts and colors
**
*/

/* A color cache system.
** - allocate a color with 'get_color(name)'
** - the cache will consist of a limited set of colors
** - reference counting will be used whenever a color is
**   allocated or used.
** - after giving a certain amount of references, the given
**   reference counters decay by a certain percentage, such
**   that colors which are not used anymore will be freed
**   in the end.
** - due to freeing colors, some documents might be flashing
**   as colors change.  This can be solved by creating a
**   fixed colormap and turn reference counting off.
*/
#define COLCACHELIMIT 256
#define INITCREDIT 256
static int colcachemax=COLCACHELIMIT;
static struct {
  XColor def;
  int refcount;
  int taken;
  Char name[40];
} colorcell[COLCACHELIMIT];
static int refsum=0;
static int numtaken=1;

static void colorref(int pos)
{
  colorcell[pos].refcount++;
  refsum++;
  if (refsum>0x10000) {
    /* decrease the reference counter by dividing them by two */
    int i,n;
    for (i=1; i<colcachemax; i++) {
      n=colorcell[i].refcount;
      refsum=refsum-(n-n/2);
      colorcell[i].refcount=n/2;
      /* optional:  free color if not used anymore */
    }
  }
}

static int free_colorcells(void)
{
  int i,mincount,minpos;
  mincount=refsum;minpos=colcachemax;
  for (i=1; i<colcachemax; i++) {
    if (colorcell[i].refcount<mincount) {
      mincount=colorcell[i].refcount;
      minpos=i;
    }
  }
  for (i=minpos; i<colcachemax; i++) {
    if (colorcell[i].refcount==mincount) {
      XFreeColors(display, colormap, &colorcell[i].def.pixel, 1,0);
      colorcell[i].taken=0;
      refsum=refsum-mincount;
      numtaken--;
    }
  }
  return minpos;
}
      

static int get_colorcell(void)
{
  int i=1;
  if (numtaken==colcachemax) i=free_colorcells();
  while (colorcell[i].taken && i<colcachemax) i++;
  colorcell[i].taken=1;
  /* give it some initial credit */
  colorcell[i].refcount=INITCREDIT;
  refsum+=INITCREDIT;
  return i;
}

static int find_color(XColor *xcol)
{
  int i=1;
  while (i<colcachemax &&
	 (colorcell[i].def.red!=xcol->red ||
	  colorcell[i].def.blue!=xcol->blue ||
	  colorcell[i].def.green!=xcol->green)) {
    i++;
  }
  if (i<colcachemax) return i;
  return 0;
}

static int define_color(XColor *xcol)
{
  int i;
  i=find_color(xcol);
  if (i) {
    colorref(i);
  } else {
    if (XAllocColor(display, colormap, xcol)) {
      i=find_color(xcol);
      if (i) {
	XFreeColors(display, colormap, &xcol->pixel, 1, 0);
	colorref(i);
      } else {
	i=get_colorcell();
	colorcell[i].def= (*xcol);
      }
    }
  }
  return i;
}

int get_color(Char *name)
{
  char buffer[80];
  int i=0;
  XColor def;
  while (((buffer[i]=name[i])) && i<80) i++;
  buffer[i]=0;
  if (!XParseColor(display, colormap, buffer, &def)) {
    message2(MP_ERROR, translate("Unknown color: "),name);
    return 0;
  } else {
    return define_color(&def);
  }
}

/*
** To allow correct XOR operations in the previous version, the colors
** had to be located at specific locations, as the XOR operations works
** on the colormap positions, not on the colors. The only way to get this
** to work, is by allocating your own colors in the colormap, which can
** be regarded as antisocial, since other colorful applications degrade in
** appearance.  By removing the XOR operations, the use of colors is
** less restrictive, which would eventually allow the specification of
** colors in templates.  I'm not certain if all drawing operations will
** still work correct, as some operations heavily depended upon the XOR
** operation (for example cursor movements).
**
** For grayscale monitors (usually X terminals), the colors might
** not be displayed correctly, making the selections invisible. Therefore,
** the colors have to be adjustable for grayscale monitors. (They have
** to be adjustable anyhow.)
**
** In an environment with not enough colors, such as on black and white
** monitors (perhaps also grayscale monitors), a fallback mechanism has
** to be provided to make the multiple selections visible.  The most
** readable solution is using underlines, as patrons make the text unreable.
*/

#define MAX_CELLS 16

unsigned long colorlist[MAX_CELLS/2][2];
static int use_underline[MAX_CELLS/2];
static int maxcolors=MAX_CELLS;

/* this should be available through a file */
static char *colorname[MAX_CELLS] = {
/* foreground   background    selection */
    "Black",     "White",     /* normal */
    "Yellow",    "Blue",      /* source */
    "Green",     "Magenta",   /* argument */
    "Red",       "Cyan",      /* source+argument */
    "White",     "Black",     /* target */
    "Blue",      "Yellow",    /* target+source */
    "Magenta",   "Green",     /* target+argument */
    "Cyan",      "Red" };     /* target+source+argument */

static char *grayname[MAX_CELLS] = {
/* foreground   background    selection */
    "Black",     "White",     /* normal */
    "Gray25",    "Gray75",    /* source */
    "Gray12",    "Gray88",    /* argument */
    "Gray37",    "Gray63",    /* source+argument */
    "White",     "Black",     /* target */
    "Gray75",    "Gray25",    /* target+source */
    "Gray88",    "Gray12",    /* target+argument */
    "Gray63",    "Gray37" };  /* target+source+argument */



static void make_colors(void)
{
    Colormap def_cmap;
    int ncolors = maxcolors;
    int np=0,i,j;
    XColor exact_defs[MAX_CELLS];
    char **cname;

    def_cmap = colormap;
    cname=colorname;
    if (def_depth == 1) {
	ncolors=2;
	np=0;
    } else if (visualclass==GrayScale || visualclass==StaticGray || usegray)
        cname =grayname;
    for (i = 0; i < ncolors; i++) {
	if (!XParseColor (display, def_cmap, cname[i], &exact_defs[i])) {
	    fprintf(stderr,"%s: color name %s not in database",
		    UstrtoLocale(progname), colorname[i]);
	    exact_defs[i].red=exact_defs[i].green=exact_defs[i].blue=0;
	}
	exact_defs[i].flags = DoRed | DoGreen | DoBlue;
    }
    if (ncolors==2) {
	j=0;
	if (XAllocColor(display, def_cmap, &exact_defs[0])) {
	    j++;
	    colorlist[0][0]=exact_defs[0].pixel;
	}
	if (XAllocColor(display, def_cmap, &exact_defs[1])) {
	    j=j+2;
	    colorlist[0][1]=exact_defs[1].pixel;
	}
	if (j!=3 || colorlist[0][1]==colorlist[0][0]) {
	    if (j)
		XFreeColors(display, def_cmap,
			    colorlist[0]+(j==2), 1+(j==3), 0);
	    colorlist[0][0]=black_pixel;
	    colorlist[0][1]=white_pixel;
	}
	colorlist[4][0]=colorlist[0][1];
	colorlist[4][1]=colorlist[0][0];
	for (i=1; i<4; i++)
	    for (j=0; j<2; j++) {
		colorlist[i][j]=colorlist[0][j];
		colorlist[i+4][j]=colorlist[4][j];
	    }
    } else {
	int k;
	for (i=0; i<ncolors; i++) {
	  if (XAllocColor(display, def_cmap, &exact_defs[i])) {
	    colorlist[i/2][i%2]=exact_defs[i].pixel;
	  }
	}
	if (ncolors<MAX_CELLS) {
	    if (ncolors==4) {
		for (i=2; i<4; i++)
		    for (k=0; k<2; k++)
			colorlist[i][k]=colorlist[i-2][k];
	    }
	    for (i=4; i<8; i++)
		for (k=0; k<2; k++)
		    colorlist[i][k]=colorlist[i-4][1-k];
	}
    }
    for (i=0; i<MAX_CELLS/2; use_underline[i++]=0);
    for (j=1,i=3; j<MAX_CELLS/2; j*=2,i=i/2) {
	if (colorlist[0][0]==colorlist[j][0]) {
	    int k;
	    for (k=0; k<MAX_CELLS/2; k++)
		if (k&j) use_underline[k] ^= i;
	}
    }
    /* set back and foreground colors */
    black_pixel=colorlist[0][0];
    white_pixel=colorlist[0][1];
}

#define MAXGCS 3

static GC gca[MAXGCS];
static FontID gcfont[MAXGCS];
static int gccolor[MAXGCS];

static void make_GCs(void)
{
    XGCValues values;
    unsigned long mask;
    int i;

    values.foreground = colorlist[0][0];
    values.background = colorlist[0][1];
    values.fill_style = FillSolid;
    values.fill_rule  = WindingRule;
    values.function   = GXcopy;
    gccolor[Normal]=0;

    mask = (GCForeground | GCBackground | GCFillStyle | GCFunction |
	    GCFillRule);

    gca[Normal]   = XCreateGC(display, root_window, mask, &values);
    values.foreground = colorlist[0][1];
    values.background = colorlist[0][0];
    gccolor[Reverse]=0;
    gca[Reverse] = XCreateGC(display, root_window, mask, &values);

    values.background = 0;
    values.foreground = colorlist[0][0]^colorlist[4][0];
    gccolor[Xor]=4;
    values.function = GXxor;
    gca[Xor] = XCreateGC(display, root_window, mask, &values);
    for (i=0; i<MAXGCS; i++) gcfont[i]=TEXTFONT;
}

static void destroy_GCs(void)
{
    int i;

    for (i=0; i<MAXGCS; i++) XFreeGC(display,gca[i]);
}

GC get_GC(TextMode gcnr, int colortype,int colorpos)
{  /* add a check if XSet is needed. */
  if (colorpos && gcnr!=Reverse) {
    XSetForeground(display, gca[gcnr], colorcell[colorpos].def.pixel);
  } else {
    XSetForeground(display, gca[gcnr], colorlist[colortype][gcnr]);
  }
  XSetBackground(display, gca[gcnr], colorlist[colortype][1-gcnr]);
  return gca[gcnr];
}

GC get_GCXor(int colortype)
{   /* add a check if XSet is needed. */
    if (gccolor[Xor]!=colortype) {
	XSetForeground(display, gca[Xor],
		       colorlist[colortype][0]^colorlist[0][0]);
	gccolor[Xor]=colortype;
    }
    return gca[Xor];
}

int must_underline(int colortype)
{
    return use_underline[colortype];
}

GC get_GC_font(TextMode gcnr, int colortype, int colorpos,
	       FontID fontid)
{
    (void) get_GC(gcnr, colortype,colorpos);
    if (gcfont[gcnr]!=fontid) {
	XSetFont(display,gca[gcnr],fontid);
	gcfont[gcnr] = fontid;
    }
    return gca[gcnr];
}

void undefined_font(TextMode gcnr)
{
    gcfont[gcnr]= -1;
}

/*
**
**  Functies die gegevens van windows verwerken:
**
*/

static char *windowdesc[MAXWINDOWTYPE+2] =
    { "No Window", "Button", "Scrollbar", "String", "Menu", "Edit",
      "SubEdit", "Buffer", "SubBuffer", "Stencil", "StencilFile",
      "Define", "Symbol", "Default", "Remark", "Find", "FileSelc",
      "Popup", "CheckBox", "Notation", "NotationFile" };

static void *windowkeystack[MAXWINDOWTYPE];

typedef struct {
    int state,mapped;
    void *keyboardstack;
    Window win_id, pwin_id;
    WINDOWTYPE type;
    void *data;
    Char *helpfile;
} EXTENDEDINFO;

#define EXPAND_SIZE  10
static EXTENDEDINFO *winfo = NULL;
static int number_of_windows = 0;
static int max_usable = 0;

static int get_pos(Window win)
{
    int i,j,h;
    
    i= -1;
    j= number_of_windows;
    while (i<j-1) {
	h= (i+j)/2;
	if (winfo[h].win_id < win)      i=h;
	else if (winfo[h].win_id > win) j=h;
	else return h;
    }
    return i;
}

static int expand_windows(int nr)
{
    if (number_of_windows+nr>max_usable) {
	int newsize = (((number_of_windows+nr)+EXPAND_SIZE-1) / EXPAND_SIZE) *
	    EXPAND_SIZE;
	EXTENDEDINFO *temp =
	    (EXTENDEDINFO *) malloc(newsize * sizeof(EXTENDEDINFO));
	int i;
	if (!temp) return MP_False;
	for (i=0; i<number_of_windows; i++) {
	    temp[i] = winfo[i];
	}
	free(winfo);
	winfo = temp;
	max_usable = newsize;
	return MP_True;
    }
    return MP_True;
}

static void shift_left(int i)   /* schuif alles na i 1 plaats naar links */
{
    while (i<number_of_windows-1) {
	winfo[i]=winfo[i+1];
	i++;
    }
    number_of_windows--;
}

static void shift_right(int i)  /* schuif alles na i 1 plaats naar rechts */
{
    int j=number_of_windows-1;
    
    while (j>i) {
	winfo[j+1] = winfo[j];
	j--;
    }
    number_of_windows++;
}

WINDOWTYPE get_window_type(Window win, Window *pwin, void **wdata)
{
    int i;
    
    i = get_pos(win);
    if (i==-1 || winfo[i].win_id != win || !exist_window(winfo[i].pwin_id)) {
	*pwin = 0;
	*wdata = NULL;
	return NOWINDOW;
    }
    *wdata = winfo[i].data;
    *pwin = winfo[i].pwin_id;
    return winfo[i].type;
}

Bool exist_window(Window win)
{
    int i;

    i = get_pos(win);
    return (i!=-1 && winfo[i].win_id == win);
}

int add_window(Window win, WINDOWTYPE wtype, Window pwin, void *wdata,
	       Char *helpfile)
{
    int i;

    if (!expand_windows(1)) {
	message(MP_ERROR, translate("Not enough windows available."));
	return 0;
    }
    i = get_pos(win);
    if (i!=-1 && winfo[i].win_id == win) return 0;
    shift_right(i);
    i++;
    winfo[i].state = VisibilityFullyObscured;
    winfo[i].mapped = MP_False;
    winfo[i].win_id = win;
    winfo[i].pwin_id = pwin;
    winfo[i].type = wtype;
    winfo[i].data = wdata;
    winfo[i].keyboardstack = copy_keyboard_stack(windowkeystack[wtype]);
    winfo[i].helpfile = helpfile;
    return 1;
}

void window_def_keyboard(Char *windowtype, Char *list)
{
  int i;
  for (i=0; i<MAXWINDOWTYPE; i++) {
    if (!Ustrcmp(translate(windowdesc[i]),windowtype)) {
      windowkeystack[i]=get_keyboard_stack(list);
      return;
    }
  }
  fprintf(stderr, "%s: %s\n", UstrtoLocale(translate("Unknown window type")),
	  UstrtoLocale(windowtype));
}

void* window_keyboard(Window win)
{
  int i;
  i = get_pos(win);
  if (i!=-1 && winfo[i].win_id == win) return winfo[i].keyboardstack;
  else return NULL;
}

#define MAX_WIN_DEPTH 40
Char* window_find_help(void)
{
  Char *c;
  Window rW=None, cW=root_window, dW;
  Window subwindow[MAX_WIN_DEPTH];
  int rx, ry, x1, y1,tl=0;
  unsigned int mask;
  do {
    subwindow[tl]=cW;
    dW=cW;
    if (!XQueryPointer(display, dW, &rW, &cW, &rx, &ry,
		       &x1, &y1, &mask)) {
      break;
    }
    tl++;
  } while (cW && tl<MAX_WIN_DEPTH);
  c=NULL;
  while (tl>=0 && !c) {
    c = window_help(subwindow[tl]);
    tl--;
  }
  if (!c)  c = translate("Tutorial.mpd");
  return c;
}

Char* window_help(Window win)
{
    int i;
    i = get_pos(win);
    if (i!=-1 && winfo[i].win_id == win) return winfo[i].helpfile;
    else return NULL;
}

void change_visibility(Window win, int state)
{
    int i;
    i = get_pos(win);
    if (i==-1 || winfo[i].win_id != win) return;
    winfo[i].state = state;
}

void change_mapped(Window win, int mapped)
{
    int i;
    i = get_pos(win);
    if (i==-1 || winfo[i].win_id != win) return;
    winfo[i].mapped = mapped;
}

void refresh_all(void)
{
    int i,k;
    for (i=0; i<number_of_windows; i++) {
	k=i;
	if (winfo[i].state != VisibilityFullyObscured &&
	    winfo[i].type !=BUTTONWINDOW &&
	    winfo[i].mapped && eventfunc[winfo[i].type]->draw) {
	    if (!winfo[i].data) {
		int j;
		j = get_pos(winfo[i].pwin_id);
		if (j!=-1 && winfo[j].win_id == winfo[i].pwin_id &&
		    winfo[j].data) k=j;
	    }
	    if (winfo[k].data)
		(*(eventfunc[winfo[i].type]->draw))(winfo[k].data);
	    else
		(*(eventfunc[winfo[i].type]->draw))((void*) &winfo[i].win_id);
	}
    }
}

void *next_data_with_type(WINDOWTYPE wt, int *i)
{
    if (*i<0) *i=0;
    while (*i<number_of_windows && winfo[*i].type != wt) (*i)++;
    if (*i==number_of_windows)
	return NULL;
    return winfo[*i].data;
}

void *remove_window(Window win)
{
    int i;
    void *temp;
    
    i = get_pos(win);
    if (i==-1 || winfo[i].win_id != win) return NULL;
    destroy_keyboard_stack(winfo[i].keyboardstack);
    temp=winfo[i].data;
    shift_left(i);
    while (i<number_of_windows) {
	if (winfo[i].pwin_id==win) winfo[i].mapped=MP_False;
	i++;
    }
    return temp;
}

void destroy_window(Window win)
{
    void *temp;
    
    if (aig(temp = remove_window(win))) free(temp);
}

static Atom targets[5], target_string=0, data_prop=0, clipboard=0,
            target_targets=0, target_length=0;
static Window select_window = 0;

static void make_atoms(void)
{
    if (!target_string) {
	target_string = XInternAtom(display, "STRING", MP_True);
	targets[0] = target_string;
    }
    if (!target_targets) {
	target_targets = XInternAtom(display, "TARGETS", MP_True);
	targets[1] = target_targets;
    }
    if (!target_length) {
	target_length = XInternAtom(display, "LENGTH", MP_True);
	targets[2] = target_length;
    }
    if (!clipboard) clipboard = XInternAtom(display, "CLIPBOARD", MP_True);
}

void set_selection_window(Window win)
{
    select_window = win;
}

void get_wm_selection(void)
{
    if (!select_window) return;
    if (!data_prop) data_prop = XInternAtom(display, "STRING_ASK", MP_False);
    make_atoms();
    if (data_prop && target_string)
	XConvertSelection(display, XA_PRIMARY, target_string, data_prop,
			  select_window, last_time);
}

Bool set_wm_selection(void)
{
    if (!select_window) return MP_False;
    make_atoms();
    if (clipboard) {
	XSetSelectionOwner(display, clipboard, select_window, last_time);
	(void) XGetSelectionOwner(display, clipboard);
    }
    XSetSelectionOwner(display, XA_PRIMARY, select_window, last_time);
    return (XGetSelectionOwner(display, XA_PRIMARY)==select_window);
}

void set_clipboard(void)
{
/*    if (latexselection && target_string && clipboard)
	XChangeProperty(display, select_window, clipboard,
			target_string, 8, PropModeReplace,
			latexselection, strlen(latexselection)); */
}

void send_selection(XSelectionRequestEvent *event)
{
    XSelectionEvent sev;
    make_atoms();
    if (event->target==target_string) {
	if (latexselection)
	    XChangeProperty(event->display, event->requestor, event->property,
			    event->target, 8, PropModeReplace,
			    (unsigned char*) latexselection,
			    strlen(latexselection));
	else
	    XChangeProperty(event->display, event->requestor, event->property,
			    event->target, 8, PropModeReplace,
			    (unsigned char*)"", 0);
	sev.property = event->property;
    } else if (event->target==target_targets) {
	XChangeProperty(event->display, event->requestor, event->property,
			event->target, 32, PropModeReplace,
			(unsigned char*) targets, 3);
	sev.property = event->property;
    } else if (event->target==target_length) {
	int l;
	if (latexselection) l=strlen(latexselection); else l=0;
	XChangeProperty(event->display, event->requestor, event->property,
			event->target, 32, PropModeReplace,
			(unsigned char*) &l, 1);
	sev.property = event->property;
    } else
	sev.property = None;
    sev.type = SelectionNotify;
    sev.serial = event->serial;
    sev.send_event = MP_True;
    sev.display = event->display;
    sev.target = event->target;
    sev.requestor = event->requestor;
    sev.selection = event->selection;
    sev.time = event->time;
    XSendEvent(event->display, event->requestor, MP_True, 0, (XEvent*) &sev);
}

/*
**
**  Functies die de gegevens initialiseren of verwijderen:
**
*/

static void make_fontpath(void)
{
    char *cp;
    char *c;
    char *fp, **cdirs, **adirs, **ndirs;
    int i, j, ol, nl, ncdirs, nadirs, nndirs;

    if (!(cp = getenv("MATHPADFONTPATH"))) cp = MATHPADHOME "/fonts";

    if (aig(i = strlen(cp)) == 0) return;

    if (!(cdirs = XGetFontPath(display, &ncdirs))) {
        message(MP_ERROR, translate("Cannot get old font path."));
        return;
    }

    if (!(fp = (char*) malloc(i+1))) message(MP_EXIT-1, translate("Out of memory."));
    strcpy(fp, cp);
    nadirs = i = 1;
    for (c = fp; (c = strchr(c,',')) != NULL; c++) nadirs++;
    if (!(adirs = (char**) malloc(nadirs*sizeof(char*))))
	message(MP_EXIT-1, translate("Out of memory."));
    adirs[0] = fp;
    for (cp = fp; (cp=strchr(cp,',')) != NULL; *cp++ = '\0') adirs[i++] = cp+1;

    if (!(ndirs = (char**) malloc((ncdirs+nadirs)*sizeof(char*))))
	message(MP_EXIT-1, translate("Out of memory."));
    for (i = 0; i != ncdirs; i++) ndirs[i] = cdirs[i];
    nndirs = ncdirs;
    for (i = 0; i != nadirs;  i++) {
	nl = strlen(adirs[i]);
        for (j=0; j!=nndirs; j++) {
	    ol = strlen(ndirs[j]);
	    if (ol-nl<2 && nl-ol<2) {
		int dl;
		dl = (ol<nl ? ol : nl);
		if ((dl==nl || adirs[i][nl-1]=='/') &&
		    (dl==ol || ndirs[j][ol-1]=='/') &&
		    !strncmp(adirs[i],ndirs[j],dl-1))  break;
	    }
	}
        if (j == nndirs) ndirs[nndirs++] = adirs[i];
    }

    if (nndirs != ncdirs) XSetFontPath(display, ndirs, nndirs);

    free((char*)ndirs);
    free((char*)adirs);
    free(fp);
    XFreeFontPath(cdirs);
}

static Bool in_dump = MP_False;
static Bool in_auto_save = MP_False;
static Bool alarm_works = MP_True;
static Bool alarm_went_off = MP_False;
Bool can_auto_save = MP_True;

static void do_auto_save(void)
{
    int i;
    int j=0;

    if (!in_dump) {
	message(MP_MESSAGE,translate("AutoSave in progress ... "));
	XFlush(display);
    }
    in_auto_save = MP_True;

    for (i=0; i<number_of_windows; i++) {
	if (eventfunc[winfo[i].type]->auto_save) {
	    if (in_dump) j++;
	    (*(eventfunc[winfo[i].type]->auto_save))(winfo[i].data, j);
	}
    }
    if (!in_dump) {
	message(MP_MESSAGE,translate("AutoSave done"));
	XFlush(display);
    }
    in_auto_save = MP_False;
    alarm_went_off = MP_False;
}

static Bool alarm_set = MP_False;

typedef struct {
    int signum;
    char *sigcomment;
    void (*oldhandle)(int);
    int sigtype; /* 1=dump backups,reset old: 2=alarm: 3=backups,quit */
} SIGTYPE;

static SIGTYPE catchsig[] = {
#ifdef SIGHUP
                              { SIGHUP, "Hanging up.", NULL, 3 },
#endif
#ifdef SIGINT
                              { SIGINT, "Interrupt.", NULL, 3 },
#endif
#ifdef SIGQUIT
                              { SIGQUIT, "Quit.", NULL, 1 },
#endif
#ifdef SIGILL
                              { SIGILL, "Illegal instruction.", NULL, 1 } ,
#endif
#ifdef SIGBUS
                              { SIGBUS, "Bus error.", NULL, 1 } ,
#endif
#ifdef SIGSEGV
                              { SIGSEGV, "Segmentation fault.", NULL, 1 },
#endif
#ifdef SIGALRM
                              { SIGALRM, NULL, NULL, 2 },
#endif
#ifdef SIGTERM
                              { SIGTERM, "Terminated.", NULL, 3 },
#endif
#ifdef SIGLOST
                              { SIGLOST, "Resource lost.", NULL, 3 },
#endif
                              { 0, NULL, NULL, 0 } };

static void handle(int sig)
{
    int i=0;

    while (catchsig[i].sigtype && catchsig[i].signum!=sig) i++;
    switch (catchsig[i].sigtype) {
    case 1:
	signal(sig, handle);
	if (!in_dump) {
	    in_dump = MP_True;
	    do_auto_save();
	    fprintf(stderr, "%s: %s Dumps made.\n",
		    UstrtoLocale(progname), catchsig[i].sigcomment);
	    in_dump = MP_False;
	} else
	    fprintf(stderr, "%s: %s\n", UstrtoLocale(progname), catchsig[i].sigcomment);
	signal(sig, catchsig[i].oldhandle);
	break;
    case 3:
	signal(sig, handle);
	if (!in_dump) {
	    in_dump = MP_True;
	    do_auto_save();
	    in_dump = MP_False;
	    fprintf(stderr, "%s: %s Dumps made.\n",
		    UstrtoLocale(progname), catchsig[i].sigcomment);
	}
	exit(1);
	break;
    case 2:
	signal(sig,handle);
	if (in_dump) break;
	alarm_works = MP_True;
	alarm_went_off = !in_auto_save;
	if (can_auto_save && alarm_went_off) {
	    do_auto_save();
	    alarm_went_off = MP_False;
	}
	alarm(save_minute*60);
	alarm_set = MP_True;
	break;
    default:
	break;
    }
}

static Bool first_error = MP_True;

static int error_handler(Display *d, XErrorEvent *err)
{
    char msg[80];
    int i;

    if (err->request_code == 51) {
	fprintf(stderr, "%s: Unable to add fontpath.", UstrtoLocale(progname));
	return 0;
    }
    fprintf(stderr, "%s: Error from server:\n", UstrtoLocale(progname));
    if (d!=display)
	fprintf(stderr, "  ? ");
    XGetErrorText(d, err->error_code, msg, 80);
    i = 0;
    while (!isspace(msg[i])) i++;
    msg[i] = '\0';
    fprintf(stderr, "   (%s) %i.%i 0x%lx ",
	    msg, err->request_code, err->minor_code, err->resourceid);
    if (err->error_code == BadWindow || err->error_code == BadDrawable) {
	WINDOWTYPE wt;
	void *data;
        Window par;
	wt = get_window_type((Window) err->resourceid, &par, &data);
	if (wt)
	    fprintf(stderr, "%i 0x%lx %p",wt,par,data);
    }
    fprintf(stderr, "\n");
    if (first_error) {
	in_dump = MP_True;
	do_auto_save();
	in_dump = MP_False;
	fprintf(stderr, "%s Dumps made.\n", UstrtoLocale(progname));
	first_error = MP_False;
    }
    return 0;
}

#ifdef IOERROR_HANDLE
static int ioerror_handler(Display *d, XErrorEvent *err)
#else
static int ioerror_handler(Display *d)
#endif
{
    if (d!=display)
	fprintf(stderr, "%s Error from strange display.\n", UstrtoLocale(progname));
    fprintf(stderr, "%s: XIO-error. Program has to terminate.\n", UstrtoLocale(progname));
    in_dump = MP_True;
    do_auto_save();
    fprintf(stderr, "%s: Dumps made.\n", UstrtoLocale(progname));
    exit(2); return 0;
}

void server_init(void)
{
    int i;
    if (!aig(display=XOpenDisplay(displayname))) {
	fprintf(stderr, "%s: Cannot connect to server %s\n",
		UstrtoLocale(progname), XDisplayName(displayname));
	exit(-1);
    }
    screen_num = DefaultScreen(display);
    root_window = RootWindow(display, screen_num);
    display_width = DisplayWidth(display,screen_num);
    display_height = DisplayHeight(display,screen_num);
    if (visualclass>=StaticGray && visualclass<=DirectColor) {
	XVisualInfo vinfo, *vinforet;
	int vinfonr;
	vinfo.class=visualclass;
	vinfo.screen=screen_num;
	vinforet=XGetVisualInfo(display,VisualScreenMask|VisualClassMask,
				&vinfo, &vinfonr);
	if (vinfonr && vinforet) visual=vinforet[0].visual;
	def_depth=vinfo.depth;
	colormap = XCreateColormap(display, root_window, visual, AllocNone);
	if (vinforet) XFree(vinforet);
    }
    if (!visual) {
	XVisualInfo vinfo, *vinforet;
	VisualID vid;
	int vinfonr;
	visual=DefaultVisual(display,screen_num);
	vid=XVisualIDFromVisual(visual);
	vinfo.visualid=vid;
	vinforet=XGetVisualInfo(display, VisualIDMask, &vinfo, &vinfonr);
	if (vinfonr && vinforet) visualclass = vinforet[0].class;
	if (vinforet) XFree(vinforet);
	def_depth=DefaultDepth(display, screen_num);
	colormap=DefaultColormap(display,screen_num);
    }
    black_pixel = BlackPixel(display,screen_num);
    white_pixel = WhitePixel(display,screen_num);
    icon_pixmap = XCreateBitmapFromData(display, root_window,
					(char*) progicon_bits, progicon_width,
					progicon_height);
    WM_SAVE_YOURSELF = XInternAtom(display, "WM_SAVE_YOURSELF", MP_True);
    WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", MP_True);
    XSetErrorHandler(error_handler);
    XSetIOErrorHandler(ioerror_handler); /* Add option -DIOERROR_HANDLE */
    if (WM_SAVE_YOURSELF==None || WM_DELETE_WINDOW == None) 
	message(MP_MESSAGE, translate("No save_yourself or delete_window available."));
    protocol[1]=WM_SAVE_YOURSELF;
    protocol[0]=WM_DELETE_WINDOW;
    make_fontpath();
    /* make_fonts(); */
    font_set_system_data(display);
    make_colors();
    make_GCs();
    wait_cursor = XCreateFontCursor(display, XC_watch);
    (void) add_window(root_window, NOWINDOW, root_window, NULL, NULL);
    user_id = getuid();
    group_id = getgid();

    i=0;
    while (catchsig[i].sigtype) {
	catchsig[i].oldhandle = signal(catchsig[i].signum, handle);
	i++;
    }
}

void server_close(void)
{
  /* destroy_fonts(); */
    destroy_GCs();
    XCloseDisplay(display);
    exit(0);
}

Window motion_window = 0, when_motion_window=0;
Bool   motion_set = MP_False, motion_wait = MP_False, is_click = MP_True;
unsigned long press_time = 0, drag_time = 0, release_time = 0;
Bool   double_click = MP_False;
unsigned int mouse_button = 0;
unsigned int press_state = 0;

#define ANYBUTTON (Button1Mask | Button2Mask | Button3Mask | \
		   Button4Mask | Button5Mask)

Bool mouse_press(unsigned int status, unsigned int bnr)
{
    if (!(status & ANYBUTTON)) {
	press_state = status;
	if (status&ControlMask) bnr++;
	if (status&Mod1Mask) bnr+=2;
	mouse_button=bnr;
	is_click = MP_True;
	return MP_True;
    } else
	return MP_False;
}

Bool mouse_release(unsigned int status, unsigned int bnr)
{
    return ((status & ANYBUTTON) ^ (Button1Mask<<(bnr-1)))==0;
}

void set_save_period(int minute)
{
    char s[50];
    if (minute>0) {
	alarm(60*minute);
	alarm_set = MP_True;
	save_minute = minute;
	sprintf(s, "AutoSave every %i minutes.", minute);
	message(MP_MESSAGE, translate(s));
    } else {
	do_auto_save();
    }
}

Bool is_drag(unsigned long  motion_time)
{
    if (motion_set && motion_wait) {
	if (motion_time > drag_time) {
	    motion_wait = MP_False;
	    return MP_True;
	} else
	    return MP_False;
    }
    return motion_set;
}

Bool motion_get_pos(int *x, int *y)
{
    Window root, child;
    int rx, ry;
    unsigned int buttons;

    if (motion_set && motion_window)
	return XQueryPointer(display, motion_window, &root, &child,
			     &rx, &ry, x, y, &buttons);
    else
	return MP_False;
}

void get_motion_hints(Window win, int ms)
{
    motion_window = win;
    motion_set = MP_True;
    if (ms == -1) ms = wait_time;
    motion_wait = (ms>0);
    drag_time = press_time+ms;
}

void stop_motion_hints(void)
{
    motion_set = MP_False;
    motion_window = 0;
    motion_wait = MP_False;
}

static Window wc_window=0;

void set_wait_cursor(Window win)
{
    XSetWindowAttributes swat;

    swat.cursor=wait_cursor;
    wc_window=win;
    XChangeWindowAttributes(display, win, CWCursor, &swat);
    XFlush(display);
}

void remove_wait_cursor(void)
{
    XSetWindowAttributes swat;
    swat.cursor=None;
    XChangeWindowAttributes(display, wc_window, CWCursor, &swat);
    XFlush(display);
    wc_window=0;
}


static void (*create_func)(int,Bool) = 0;
static int create_arg = 0;

static void handle_directory_creation(void *data, int bnr)
{
    Bool made = MP_False;
    char *c;
    void (*f)(int,Bool) = create_func;
    int a = create_arg;

    c = (char*)data + strlen((char*)data)-1;
    if (*c=='/') *c = '\0';
    else c = NULL;
    if (!bnr) {
	made = !mkdir((char*)data,511);
    }
    create_func = NULL;
    create_arg = 0;
    if (f) (*f)(a, made);
    if (c) *c = '/';
}

static Bool create_directory(Char *data, void (*func)(int,Bool), int arg)
{
    DIR *d;
    FILE *f;
    Char *c;

    if (aig(d = opendir(UstrtoFilename(data)))) {
	closedir(d);
	return MP_True;
    }
    if (aig(f = fopen(UstrtoFilename(data), "r"))) {
	fclose(f);
	return MP_False;
    }
    c = concat(data,
	       translate("\nis a directory that MathSpad needs.\nMust it be created ?"));
    if (c) {
        Char *buttons[3];
	buttons[0] = translate(" Create ");
	buttons[1] = translate(" Cancel ");
	buttons[2] = 0;
	create_func = func;
	create_arg = arg;
	remark_make(root_window, (void*) data, handle_directory_creation,
		    REMARK_CENTRE, c, buttons,
		    NULL,0, NULL);
	free(c);
    }
    return MP_False;
}

static Char **needed_dir[3] = { &userdir, &notationdir, &latexdir }; 

void create_needed_directories(int number, Bool made)
{
    if (made) {
	while (number<3) {
	    if (create_directory( *needed_dir[number],
				  create_needed_directories, number+1))
		number++;
	    else
		return;
	}
    }
}

/*
**  Defaults loading
*/

static Char *defaultfilename = NULL;

#define DEFAULTFILENAME  ".mpdefaults"
#define NR_FILEDESC 19
#define NR_FILEDESC_OLD 21
static char *filedesc[NR_FILEDESC_OLD] =
    { "ScreenLine", "ScreenTab",    "ScreenSpace",  "LatexLine",
      "LatexTab",   "LatexSpace",   "DirUser",      "DirStencils",
      "DirLatex",   "Fonts",        "StencilFont",  "SymbolFont",
      "PopupFont",  "KeyPath",      "AutoSave",     "LeftMargin",
      "TextDots",   "ClickTime",    "Position",     "DirNotations",
      "NotationFont", };

static Char **filechar[NR_FILEDESC_OLD] =
    { NULL, NULL, NULL, &latex_line_unit, &latex_tab_unit,
      &latex_space_unit, &userdir, &notationdir, &latexdir,
      &fontfile[0], &fontfile[1], &fontfile[2], &fontfile[3],
      &keypath, NULL, NULL, NULL, NULL, NULL, &notationdir,
      &fontfile[1] };
static int *fileint[NR_FILEDESC_OLD] =
    { &line_space, &screen_tab, &screen_space, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, &save_minute, &latex_side, &textdots, &wait_time,
      NULL, NULL, NULL };

static Char *no_return_copy(char *str)
{
    Char *tmp;
    int i;

    tmp = concat(LocaletoUstr((unsigned char*)str),NULL);
    i = Ustrlen(tmp);
    if (tmp[i-1]=='\n') tmp[i-1]=0;
    return tmp;
}

extern void default_update(void);

static void load_defaults(FILE *file)
{
    int i,x,y,w,h,k;
    char *id;
    char buf[BUFSIZE];

    while (fgets(buf, BUFSIZE, file) && strcmp(buf, "STOP\n")) {
	if (buf[0]!='#') {
	    id = NULL;
	    for (i=0;
		 i<NR_FILEDESC_OLD && !(id=begins_with(filedesc[i],buf));
		 i++);
	    if (id) {
		if (filechar[i]) {
		    free(*filechar[i]);
		    *filechar[i] = no_return_copy(id);
		} else if (fileint[i])
		    sscanf(id, "%i", fileint[i]);
		else {
		    sscanf(id, "%i %i %i %i %i", &k, &x, &y,&w,&h);
		    if (eventfunc[k]->set_last_pos)
			(*(eventfunc[k]->set_last_pos))(x,y,w,h);
		}
	    } else {
		i=MAXWINDOWTYPE;
		while (i && !(id=begins_with(windowdesc[--i],buf)));
		if (id) {
		    sscanf(id, "%i\t%i\t%i\t%i", &x, &y, &w, &h);
		    if (eventfunc[i]->set_last_pos)
			(*(eventfunc[i]->set_last_pos))(x,y,w,h);
		}
	    }
	}
    }
    userdir = standard_dir(userdir);
    notationdir = standard_dir(notationdir);
    latexdir = standard_dir(latexdir);
    default_update();
}

static void write_defaults(FILE *f)
{
    int i;

    fprintf(f,"#\n#  Default values for MathSpad.\n#\n");
    for (i=0; i<NR_FILEDESC; i++) {
	if (filechar[i])
	    fprintf(f, "%s\t%s\n", filedesc[i], UstrtoFilename(*filechar[i]));
	else if (fileint[i])
	    fprintf(f, "%s\t%i\n", filedesc[i], *fileint[i]);
	else {
	    int x,y,w,h,j;
	    int dx,dy;
	    Bool add;

	    fprintf(f, "#\n#  Default window positions.\n#\n"
		       "#Type\t\tx\ty\twidth\theight\n");
	    window_manager_added(select_window, &dx, &dy);
	    for (j=NOWINDOW; j<MAXWINDOWTYPE; j++) {
		if (eventfunc[j]->last_pos) {
		    add = (*(eventfunc[j]->last_pos))(&x,&y,&w,&h);
		    /* virtual window managers move windows */
		    while (x<0) x+=display_width;
		    while (y<0) y+=display_height;
		    while (x>(int)display_width) x-=display_width;
		    while (y>(int)display_height) y-=display_height;
		    if (add) { x -= dx; y -= dy; }
		    fprintf(f, "%s", windowdesc[j]);
		    add = strlen(windowdesc[j]);
		    while (add<16) {
			fprintf(f, "\t");
			add = add + (8-(add&7));
		    }
		    fprintf(f, "%i\t%i\t%i\t%i\n", x, y, w, h);
		}
	    }
	}
    }
    fprintf(f, "#  Don't remove the next 'STOP'\nSTOP\n");
}

void save_defaults(void)
{
    FILE *file;

    if (aig(file = fopen(UstrtoFilename(defaultfilename), "w"))) {
	write_defaults(file);
	fclose(file);
    }
}

static FILE* open_project_file(Char *name)
{
    FILE *f;
    int i;
    Char *h;

    concat_in(buffer, homedir, translate("mathspad/"));
    i = Ustrlen(buffer);
    Ustrcat(buffer, strip_name(name));
    h = Ustrrchr(buffer,'.');
    if (!h || Ustrcmp(h, translate(".mpj"))) Ustrcat(buffer, translate(".mpj"));
    if (aig(f = fopen(UstrtoFilename(buffer), "r"))) return f;  /* $HOME/mathspad/name.mpj */
    if (aig(f = open_dirfile(program_dir, buffer+i, "r")))
	return f;                            /* $MATHPAD/name.mpj */
    return NULL;
}

static char *visname[6] = { "StaticGray",  "GrayScale", "StaticColor",
			    "PseudoColor", "TrueColor", "DirectColor" };
static int viscode[6] = { StaticGray,  GrayScale, StaticColor,
			  PseudoColor, TrueColor, DirectColor };
#define MAXVISUAL 6


static int parse_colorstring(char *c)
{
    int n=0;
    if (!c) return 0;
    while (*c) {
	switch (*c) {
	case 'T': case 't': case '1': n=n|4; break;
	case 'S': case 's': case '2': n=n|1; break;
	case 'A': case 'a': case '3': n=n|2; break;
	default: break;
	}
	c++;
    }
    return n;
}

typedef struct FileList FileList;
struct FileList {
  Char *filename;
  FileList *next;
};

static FileList *filelist=NULL;

void make_defaults(int argc, char *argv[])
{
    FILE *f=NULL;
    int i;
    Bool isopt;
    Bool noopt=MP_False;

    get_currentwd();
    arguments = argv;
    number_of_arguments = argc;
    progname = strip_name(translate(argv[0]));
    for (i=1; i<argc; i++) {
	if (argv[i][0]=='-') {
	    isopt=MP_True;
	    switch (argv[i][1]) {
	    case 'd':
		if (!strcmp(argv[i]+2,"isplay") && i<argc-1)
		    displayname = argv[++i];
		else
		    isopt=MP_False;
		break;
	    case 'v':
		if (!strcmp(argv[i]+2, "isual") && i<argc-1) {
		    int n;
		    i++;
		    for (n=0;n<MAXVISUAL && strcmp(argv[i],visname[n]);n++);
		    if (n==MAXVISUAL) {
			fprintf(stderr,
				"%s: Unknown visual class \"%s\"."
				"Using default.\n",
				UstrtoLocale(progname), argv[i]);
		    } else
			visualclass=viscode[n];
		} else
		    isopt=MP_False;
		break;
	    case 'i':
		if (!strcmp(argv[i]+2,"conic"))
		    iconic = MP_True;
		else
		    isopt=MP_False;
		break;
	    case 'a':
		if (!strcmp(argv[i]+2,"scii"))
		    output_mode = ASCII;
		else
		    isopt=MP_False;
		break;
	    case 'g':
		if (!strcmp(argv[i]+2, "ray")) {
		    usegray=1;
		} else isopt=MP_False;
		break;
	    case 'b':
		if (argv[i][2]=='g' && i<argc-1) {
		    int nr;
		    nr = parse_colorstring(argv[i]+3);
		    colorname[nr*2+1]=argv[++i];
		} else isopt=MP_False;
		break;
	    case 'f':
		if (argv[i][2]=='g' && i<argc-1) {
		    int nr;
		    nr = parse_colorstring(argv[i]+3);
		    colorname[nr*2]=argv[++i];
		} else if (!strcmp(argv[i]+2, "onts") && i<argc-1) {
		    if (generalinfofile) free(generalinfofile);
		    generalinfofile = concat(translate(argv[++i]),NULL);
		} else
		    isopt=MP_False;
		break;
	    case 'c':
		if (!strcmp(argv[i]+2, "olors") && i<argc-1) {
		    i++;
		    sscanf(argv[i], "%i", &maxcolors);
		    if (maxcolors==0) maxcolors=MAX_CELLS;
		} else
		    isopt=MP_False;
		break;
	    case 'p':
		if (!strcmp(argv[i]+2, "roject") && i<argc-1) {
		    if (project_name) free(project_name);
		    project_name = concat(translate(argv[++i]), NULL);
		} else if (!strcmp(argv[i]+2, "lain"))
		    output_mode = PLAINTEX;
		else
		    isopt=MP_False;
		break;
	    default:
		isopt=MP_False;
		break;
	    }
	    if (!isopt) {
		if (noopt) {
		  Char *c;
		  Char sep[2];
		  sep[0]=':';
		  sep[1]=0;
		  c=program_keypath;
		  program_keypath = concat(c,sep);
		  free(c);
		  c=program_keypath;
		  program_keypath = concat(c, translate(argv[i]+1));
		  free(c);
		} else {
		  free(program_keypath);
		  program_keypath = concat(translate(argv[i]+1), NULL);
		  noopt=MP_True;
		}
	    }
	} else {
	    if (!project_name) {
	      project_name = concat(translate(argv[i]),NULL);
	    } else {
	      FileList *l, *k;
	      l=malloc(sizeof(FileList));
	      l->filename=standard_dir(concat(translate(argv[i]),NULL));
	      l->next=0;
	      if (!filelist) {
		filelist=l;
	      } else {
		k=filelist;
		while (k->next) k=k->next;
		k->next=l;
	      }
	    }
	}
    }
    defaultfilename = concat( homedir, translate(DEFAULTFILENAME));
    if (project_name)
	f = open_project_file(project_name);
    if (!f && !(f = fopen(UstrtoFilename(defaultfilename), "r")))
	f = open_dirfile(program_dir, translate(DEFAULTFILENAME), "r");
    if (f) {
	load_defaults(f);
	fclose(f);
    }
}

/*
** Functions and variables for saving and loading a save-state-file (.mpj)
*/

static FILE *ssf = NULL;

static void set_save_state_file(Char *name)
{
    concat_in(buffer, homedir, translate("mathspad/"));
    if (!is_directory(buffer)) mkdir(UstrtoFilename(buffer),511);
    Ustrcat(buffer, strip_name((Char*)name));
    Ustrcat(buffer, translate(".mpj"));
    ssf = fopen(UstrtoFilename(buffer), "w");
    if (!ssf) {
	message(MP_CLICKREMARK, translate("Unable to create project file."));
	failure=MP_True;
    } else {
	Char *c = project_name;
	project_name = concat(strip_name(name), NULL);
	free(c);
	fprintf(ssf, "#\n#  MathSpad project file.\n");
	write_defaults(ssf);
	fprintf(ssf, "#\n#  Opened windows and loaded stencils.\n#\n"
		"#Type\t\tx\ty\twidth\theight\ticon?\tpos.\tData\n");
    }
}

static void set_save_entry(WINDOWTYPE type, int xpos, int ypos, int width, int height,
		    Bool as_icon, int sbpos, Char *string)
{
    int i;
    if (ssf) {
	fprintf(ssf, "%s", windowdesc[type]);
	i = strlen(windowdesc[type]);
	while (i<16) { fprintf(ssf, "\t"); i = i+ 8-(i&7); }
	/* to work with virtual window managers on different resolutions */
	i=0;
	while (xpos<0) { xpos+=display_width; i--; }
	while (xpos>(int)display_width) { xpos-=display_width; i++; }
	if (i) fprintf(ssf, "%i&%i\t", xpos, i);
	else fprintf(ssf, "%i\t", xpos);
	i=0;
	while (ypos<0) { ypos+=display_height; i--; }
	while (ypos> (int)display_height) { ypos-=display_height; i++; }
	if (i) fprintf(ssf, "%i&%i\t", ypos, i);
	else fprintf(ssf, "%i\t", ypos);
	fprintf(ssf, "%i\t%i\t%i\t%i\t%s\n", width, height,
		as_icon, sbpos, (string ? (char*)UstrtoLocale(string) : ""));
    }
}

void close_save_state_file(void)
{
    if (ssf) fclose(ssf);
    ssf = NULL;
}

Bool get_save_state_file(Char *name)
{
    ssf = open_project_file(name);
    if (!ssf) {
	return MP_False;
    } else {
	Char *h = project_name;
	load_defaults(ssf);
	project_name = concat(strip_name(name), NULL);
	free(h);
	return MP_True;
    }
}

Bool get_save_entry(WINDOWTYPE *type, int *xpos, int *ypos, int *width,
		    int *height, Bool *as_icon, int *sbpos, Char **string)
{
    char *idx=NULL;
    int i=0;
    char buf[BUFSIZE];

    if (!ssf) return MP_False;
    while (!idx && fgets(buf, BUFSIZE, ssf)) {
	if (buf[0]!='#') {
	    if (isdigit(buf[0])) {
		idx = buf;
		i = read_integer(&idx);
	    } else {
		i=MAXWINDOWTYPE+2;
		while (i && !(idx= (char*)begins_with(windowdesc[--i], buf)));
	    }
	}
    }
    if (!idx) return MP_False;
    if (i==MAXWINDOWTYPE) i=MAINNOTATIONWINDOW;
    else if (i==MAXWINDOWTYPE+1) i=NOTATIONWINDOW;
    *type = (WINDOWTYPE) i;
    *xpos = read_integer(&idx);
    if (*idx=='&') {
	idx++;
	i = read_integer(&idx);
	*xpos += i*display_width;
    }
    *ypos = read_integer(&idx);
    if (*idx=='&') {
	idx++;
	i = read_integer(&idx);
	*ypos += i*display_height;
    }
    *width = read_integer(&idx);
    *height = read_integer(&idx);
    *as_icon = read_integer(&idx);
    *sbpos = read_integer(&idx);
    if (idx[0]) idx[strlen(idx)-1] = '\0';
    if (idx[0])
	*string = standard_dir(concat(translate(strdup(idx)),NULL));
    else
	*string = concat(NULL, NULL);
    return MP_True;
}

void load_project(Char *name)
{
    if (get_save_state_file(name)) {
	int x,y,w,h,i,s;
	WINDOWTYPE wt;
	Char *str;
	Bool menu_opened = MP_False;
	while (get_save_entry(&wt, &x, &y, &w, &h, &i, &s, &str)) {
	    menu_opened |= (wt==MENUWINDOW);
	    if (eventfunc[wt]->use_state)
		(*(eventfunc[wt]->use_state))(x,y,w,h,i,s,str);
	}
	close_save_state_file();
	clear_file_ref();
	if (!menu_opened) {
	    message(MP_ERROR, translate("Strange project file."));
	    (*(eventfunc[MENUWINDOW]->use_state))(0,0,0,0,0,0,NULL);
	}
    } else {
      /* project file is a normal document or stencil file */
      /* interpret name as edit file */
      FileList *l;
      project_name=NULL;
      l=malloc(sizeof(FileList));
      l->filename=standard_dir(concat(name,NULL));
      l->next=filelist;
      filelist=l;
      (*(eventfunc[MENUWINDOW]->use_state))(0,0,0,0,0,0,NULL);
    }
    if (filelist) {
      FileList *l;
      l=filelist;
      while (l) {
	int i,el;
	Char *extend;
	FileList *k;
	extend=translate(".mps");
	el=Ustrlen(extend);
	i=Ustrlen(l->filename)-el;
	if (i>0) {
	  if (!Ustrcmp(l->filename+i, extend)) {
	    (*(eventfunc[MAINNOTATIONWINDOW]->use_state))(0,0,0,0,0,0,
							  l->filename);
	  } else {
	    (*(eventfunc[MAINEDITWINDOW]->use_state))(0,0,0,0,0,0,l->filename);
	  }
	}
	k=l;
	l=l->next;
	free(k);
      }
    }
}

void save_project(Char *name)
{
    int x,y,w,h,i,s,j;
    Char *str;
    Char dir[1000];

    set_save_state_file(name);
    if (failure) return;
    (*(eventfunc[MENUWINDOW]->state))(NULL, &x, &y, &w, &h, &i, &s,&str);
    set_save_entry(MENUWINDOW, x, y, w, h, i, s, str);
    j=0;
    while ((j=get_next_filename(j, &str, &i))>=0) {
	if (i) { /* only open notation files are listed. */
	    concat_in(dir, get_notation_dirname(j), str);
	    Ustrcat(dir, translate(".mps"));
	    set_save_entry(NOTATIONWINDOW, 0,0,0,0,0,0,dir);
	}
    }
    j=0;
    while (j<number_of_windows) {
	if (eventfunc[winfo[j].type]->state && winfo[j].type != MENUWINDOW) {
	    (*(eventfunc[winfo[j].type]->state))(winfo[j].data, &x, &y, &w,
						 &h, &i, &s, &str);
	    set_save_entry(winfo[j].type,x,y,w,h,i,s,str);
	}
        j++;
    }
    close_save_state_file();
    message2(MP_MESSAGE, translate("Project saved: "), name);
}

void call_layout_change(void)
{
    int i;

    Bool used[MAXWINDOWTYPE];

    for (i=0; i<MAXWINDOWTYPE; used[i++]=0);
    settabsize(screen_tab);
    setsimpletabsize(8*char_width('n'));

    for (i=0; i<number_of_windows; i++) {
        if (eventfunc[winfo[i].type]->layout_change) {
	    (*(eventfunc[winfo[i].type]->layout_change))(winfo[i].data);
	    used[winfo[i].type] = MP_True;
	    XFlush(display);
	}
    }
    for (i=0; i<MAXWINDOWTYPE; i++) {
	if (!used[i] && eventfunc[i]->layout_change)
	    (*(eventfunc[i]->layout_change))(NULL);
    }
}

void call_auto_save(unsigned long curtime)
{
    last_time = curtime;
    if (message_time && message_time+5000<curtime) {
	clear_message(MP_False);
	message_time = 0;
    }
    if (!alarm_works)
        if (!save_time || curtime < save_time)
	    save_time = curtime;
        else if (save_time+save_minute*60000 < curtime) {
	    alarm_went_off = MP_True;
	    save_time = curtime;
        }
    if (!alarm_set) {
        alarm(save_minute*60);
        alarm_set = MP_True;
    }
    if (alarm_went_off) {
        can_auto_save = MP_False;
        do_auto_save();
        can_auto_save = MP_True;
        alarm_went_off = MP_False;
    }
}    

void window_manager_added(Window win, int *x, int *y)
{
    unsigned int w,h,b,d;
    Window r;

    XGetGeometry(display, win, &r, x, y, &w, &h, &b, &d);
}
