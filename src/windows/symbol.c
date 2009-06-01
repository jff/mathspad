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
** File: symbol.c
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
#include "message.h"
#include "symbol.h"
#include "keyboard.h"
#include "helpfile.h"
#include "output.h"
#include "popup.h"

#define SYMBOLNAME  "MathSpad Symbol"
#define ICONNAME    "Symbol"

#define SIZEX      16
#define SIZEY       8
#define DEFAULTDX   4
#define DEFAULTDY   2

#define SymbolKey 'S'
#define SymbolMode ModeAdd(ModeAdd(ModeAdd(ModeAdd(0,ModShift),ModMeta),ModAlt),ModHyper)


enum button { PAGEBUTTON, DONEBUTTON, NR_BUTTON };

static struct { int nr;
		int helpnr;
                char *name;
	    } symbolbutton[NR_BUTTON] =
{ {PAGEBUTTON, SYMBOLPREVHELP, "Page"},
  {DONEBUTTON, SYMBOLDONEHELP, "Done"} };

static MENU menusymbol[NR_BUTTON] = {
  {0,0,-1,-1,-1,-1,-1,0,0,0,0},
  {0,0,-1,-1,-1,-1,-1,0,0,0,0}
};
static char *menuname[NR_BUTTON] = { "PageMenu", "SymbolDoneMenu" };


typedef struct SYMBOLPAGE SYMBOLPAGE;
struct SYMBOLPAGE {
  Char *name;     /* name of the page, e.g. Arrows */
  int columns;    /* number of columns */ 
  int rows;       /* number of rows */
  Char *content;  /* array with columns * rows Chars */
  int lastused;   /* last used character in table */
  SYMBOLPAGE *next; /* next page */
};

typedef
struct {
         Window symbolwin;
         SYMBOLPAGE *sympage;
	 int dx, dy;
	 int xpos, ypos, width, height;
	 Bool iconized;
       } SYMBOLINFO;

static int nr_opened=0;
static int nr_iconized=0;
static int is_opened = MP_False;
static SYMBOLINFO *selected=0;
static SYMBOLINFO *current_symbol_window=0;
static Char select_sym = 0;

static int offset_x, offset_y, selx, sely;
static int last_xpos = 0, last_ypos = 0, last_width = 0, last_height = 0;
static Bool state_open = MP_False, as_icon = MP_False;
static XTextProperty symbol_name, icon_name;
static char *name = SYMBOLNAME, *iname = ICONNAME;
static XSetWindowAttributes symbol_attr;
static unsigned long symbol_mask;

static int pixel_to_pos(SYMBOLINFO *sinfo, int *x, int *y)
{
    *x = (sinfo->sympage->columns*(*x - offset_x)) /
         (sinfo->width - offset_x);
    *y = (sinfo->sympage->rows*(*y - offset_y)) /
	  (sinfo->height - offset_y);
    if (*x>=0 && *x<sinfo->sympage->columns &&
	*y>=0 && *y<sinfo->sympage->rows)
	return 1;
    else return 0;
}

static Char pos_to_char(SYMBOLINFO *sinfo, int i, int j)
{
    return (Char) (sinfo->sympage->content[i + j*sinfo->sympage->columns]);
}

static void pos_to_pixel(SYMBOLINFO *sinfo, int *i, int *j)
{
    *i = offset_x + (* i) * sinfo->dx +
         ((*i) * ((sinfo->width - offset_x)%sinfo->sympage->columns))/
           sinfo->sympage->columns;
    *j = offset_y + (* j) * sinfo->dy +
         ((*j) * ((sinfo->height - offset_y)%sinfo->sympage->rows))/
           sinfo->sympage->columns;
}

static void draw_char(SYMBOLINFO *sinfo, int i, int j, TextMode gcnr)
{
    int ischar;
    Char sym = pos_to_char(sinfo,i,j);

    pos_to_pixel(sinfo, &i,&j);
    /* XFillRectangle(display, sinfo->symbolwin,
       get_GC((TextMode)(1-gcnr),0,0),
       i, j, sinfo->dx, sinfo->dy); */
    set_output_window(&sinfo->symbolwin);
    set_drawstyle(INVISIBLE);
    set_x_y(0,j);
    ischar = char_width(sym);
    thinspace(i+(sinfo->dx-ischar)/2-2);
    set_drawstyle(VISIBLE);
    if (gcnr) switch_reverse();
    thinspace(2);
    if (ischar) out_char(sym);
    thinspace(2);
    unset_output_window();
}

static void symbol_update_name(SYMBOLINFO *sinfo)
{
  XTextProperty symbolname, iconname;
  Char fullname[1024];
  int len;
  char *strlist[2];
  if (!sinfo) return;
  Ustrncpy(fullname, translate("Symbol Page: "), 1023);
  fullname[1023]=0;
  len =Ustrlen(fullname);
  Ustrncpy(fullname+len,sinfo->sympage->name, 1023-len);
  strlist[0]=UstrtoLocale(fullname);
  strlist[1]=UstrtoLocale(sinfo->sympage->name);
  XStringListToTextProperty(strlist, 1, &symbolname);
  XStringListToTextProperty(strlist+1, 1, &iconname);
  XSetWMName(display, sinfo->symbolwin, &symbolname);
  XSetWMIconName(display, sinfo->symbolwin, &iconname);
  XFree(symbolname.value);
  XFree(iconname.value);
}

static void symbol_bad_end(void *data)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    if (sinfo->iconized) nr_iconized--;
    nr_opened--;
    if (sinfo == current_symbol_window) current_symbol_window=NULL;
    if (sinfo == selected) {
	selected = NULL;
	selx = -1;
	sely = -1;
    }
    if (!nr_opened) {
	symbol_is_open = MP_False;
    }
    if (nr_opened == nr_iconized) symbol_iconized=MP_True;
    popup_remove(sinfo->symbolwin);
    destroy_window(sinfo->symbolwin);
}

static void symbol_close(void *data)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    XDestroyWindow(display, sinfo->symbolwin);
    symbol_bad_end(data);
}

static void symbol_draw(void *data)
{
    int i,j;
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    push_fontgroup(SYMBOLFONT);
    for (i=0; i<sinfo->sympage->columns; i++)
	for (j=0; j<sinfo->sympage->rows; j++)
	    draw_char(sinfo, i,j,
		      (sinfo==selected && i==selx && j==sely ? Reverse
		       : Normal));
    pop_fontgroup();
}

static void symbol_layout_change(void *data)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;
    int x = BINTERSPACE/2;
    int y = BINTERSPACE/2;

    if (!data) return;
    push_fontgroup(SYMBOLFONT);
    sinfo->dx = font_width() + DEFAULTDX;
    sinfo->dy = font_height() + DEFAULTDY;
    sinfo->width = offset_x+ sinfo->dx * sinfo->sympage->columns + x;
    sinfo->height = offset_y + sinfo->dy * sinfo->sympage->rows + y;
    XResizeWindow(display, sinfo->symbolwin, sinfo->width, sinfo->height);
    if (!sinfo->iconized) {
	XClearWindow(display, sinfo->symbolwin);
	symbol_draw(data);
    }
    pop_fontgroup();
}

static void symbol_unselect(void)
{
    if (selected) {
	draw_char(selected, selx, sely, Normal);
	selx = sely = -1;
	selected = NULL;
    }
}

static SYMBOLPAGE *pagelist=0;
static SYMBOLPAGE *last_pagelist=0;

static void symbol_handle_button(void *data, int b_num)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;
    SYMBOLPAGE *sp;
    push_fontgroup(SYMBOLFONT);
    switch (b_num) {
    case PAGEBUTTON:
      current_symbol_window=sinfo;
      menusymbol[b_num].menu=popup_define(translate(menuname[b_num]));
      if (mouse_button == Button3) {
	menusymbol[b_num].parentwin=sinfo->symbolwin;
	popup_make(&menusymbol[b_num]);
      } else {
	popup_call_default(menusymbol+b_num);
      }
      break;
    case DONEBUTTON:
      if (can_close_symbol) symbol_close(data);
      break;
    }
    pop_fontgroup();
}

static void symbol_press(void *data, XButtonEvent *event)
{
    int x = event->x;
    int y = event->y;
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    if (event->button !=3) {
      push_fontgroup(SYMBOLFONT);
      if (pixel_to_pos(sinfo, &x,&y) && !(x==selx && y==sely)) {
	symbol_unselect();
	selected = sinfo;
	selx = x;
	sely = y;
	select_sym = pos_to_char(selected, selx, sely);
	draw_char(selected, selx, sely, Reverse);
      }
      get_motion_hints(sinfo->symbolwin, -1);
      pop_fontgroup();
    } else {
      current_symbol_window=sinfo;
      menusymbol[PAGEBUTTON].menu=popup_define(translate(menuname[PAGEBUTTON]));
      menusymbol[PAGEBUTTON].parentwin=sinfo->symbolwin;
      get_motion_hints(sinfo->symbolwin,-1);
      popup_make(&menusymbol[PAGEBUTTON]);
    }
}

Char symbol_last(void)
{
    return select_sym;
}

static void symbol_release(void *data, XButtonEvent *event)
{
    int x = event->x;
    int y = event->y;
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    stop_motion_hints();
    push_fontgroup(SYMBOLFONT);
    if (!pixel_to_pos(sinfo, &x,&y))
	symbol_unselect();
    if (selected) {
	if (!char_width(select_sym = pos_to_char(selected, selx, sely)))
	    select_sym = 0;
    }
    pop_fontgroup();
    handle_key(SymbolKey, SymbolMode);
}

static void symbol_resize(void *data, XConfigureEvent *event)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    push_fontgroup(SYMBOLFONT);
    sinfo->xpos = last_xpos = event->x;
    sinfo->ypos = last_ypos = event->y;
    if (sinfo->width!=event->width || sinfo->height!=event->height) {
	sinfo->width = last_width = event->width;
	sinfo->height = last_height = event->height;
	/* if (sinfo==selected) symbol_unselect(); */
	sinfo->dx = (sinfo->width-offset_x)/sinfo->sympage->columns;
	sinfo->dy = (sinfo->height-offset_y)/sinfo->sympage->rows;
	XClearWindow(display, sinfo->symbolwin);
	symbol_draw(data);
    }
    pop_fontgroup();
}

static void symbol_motion(void *data, int x, int y)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    push_fontgroup(SYMBOLFONT);
    if (!pixel_to_pos(sinfo, &x,&y)) {
	symbol_unselect();
    } else if ((x!=selx || y!=sely)) {
	symbol_unselect();
	selected = sinfo;
	selx = x;
	sely = y;
	select_sym = pos_to_char(selected, selx, sely);
	draw_char(selected, selx, sely, Reverse);
    }
    pop_fontgroup();
}

static void symbol_iconize(void *data)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    if (!sinfo->iconized) {
	sinfo->iconized = MP_True;
	nr_iconized++;
	if (nr_iconized == nr_opened) {
	    symbol_iconized = MP_True;
	}
	if (selected == sinfo) {
	    selected = NULL;
	    selx = sely = -1;
	}
    }
}

static void symbol_deiconize(void *data)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;

    if (sinfo->iconized) {
	sinfo->iconized = MP_False;
	nr_iconized--;
	symbol_iconized = MP_False;
	symbol_is_open = MP_True;
    }
}

static void symbol_state(void *data, int *x, int*y, int *w, int *h,
		  int *i, int *s, Char **str)
{
    SYMBOLINFO *sinfo = (SYMBOLINFO *) data;
    int xm,ym;

    window_manager_added(sinfo->symbolwin, &xm,&ym);
    *x = sinfo->xpos-xm;
    *y = sinfo->ypos-ym;
    *w = sinfo->width;
    *h = sinfo->height;
    *i = sinfo->iconized;
    *s = 0;
    *str = sinfo->sympage->name;
}

static void symbol_use_state(int x, int y, int w, int h,
		      int i, int s, Char *str)
{
    as_icon = i;
    state_open = MP_True;
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
    last_pagelist=pagelist;
    symbol_open();
    state_open = MP_False;
    as_icon = MP_False;
    free(str);
}

static int symbol_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return is_opened;
}

static void symbol_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

FUNCTIONS symbolfuncs = {
    symbol_bad_end, symbol_draw, symbol_resize, symbol_press, symbol_release,
    symbol_motion, symbol_iconize, symbol_deiconize, NULL, NULL,
    symbol_layout_change, NULL, symbol_use_state, symbol_state, NULL, NULL,
    symbol_last_pos, symbol_set_last_pos  };


void symbol_new_page(Char *str, int columns, int rows)
{
  SYMBOLPAGE *nsp;
  nsp = (SYMBOLPAGE*) malloc(sizeof(SYMBOLPAGE));
  nsp->name = str;
  nsp->columns = columns;
  nsp->rows = rows;
  nsp->content = (Char*) malloc(sizeof(Char)*columns * rows);
  memset(nsp->content, 0, sizeof(Char)*columns*rows);
  nsp->lastused = 0;
  nsp->next = pagelist;
  pagelist=nsp;
}

void symbol_range_page(Char *str, int start, int end)
{
  SYMBOLPAGE *nsp;

  nsp=pagelist;
  while (nsp && Ustrcmp(str, nsp->name)) nsp=nsp->next;
  if (!nsp) {
    message2(MP_ERROR, translate("Unknown symbol page:"), str);
    return;
  }
  while (nsp->lastused < nsp->rows*nsp->columns && start<=end) {
    nsp->content[nsp->lastused++] = start++;
  }
}

void symbol_selected_symbol(Char *str)
{
  SYMBOLPAGE *nsp;

  if (!select_sym) return;
  nsp=pagelist;
  while (nsp && Ustrcmp(str, nsp->name)) nsp=nsp->next;
  if (!nsp) {
    message2(MP_ERROR, translate("Unknown symbol page:"), str);
    return;
  }
  if (nsp->lastused < nsp->rows*nsp->columns) {
    nsp->content[nsp->lastused++] = select_sym;
  }
}

void symbol_empty_page(Char *str, int length)
{
  SYMBOLPAGE *nsp;

  nsp=pagelist;
  while (nsp && Ustrcmp(str, nsp->name)) nsp=nsp->next;
  if (!nsp) {
    message2(MP_ERROR, translate("Unknown symbol page: "), str);
    return;
  }
  while (nsp->lastused < nsp->rows*nsp->columns && length > 0) {
    nsp->content[nsp->lastused++] = 0;
    length--;
  }
}

static SYMBOLINFO *lastopened=0;

void symbol_goto_page(Char *str)
{
  /* go to the page named STR and display that page in the
  ** symbol window with the symbol selection
  ** if no symbol window is selected, open a new symbol window
  ** and set the selection to that.
  */
  SYMBOLPAGE *nsp;
  nsp=pagelist;
  while (nsp && Ustrcmp(str, nsp->name)) nsp=nsp->next;
  if (!nsp) {
    message2(MP_ERROR, translate("Unknown symbol page: "), str);
    return;
  }
  if (!selected && !current_symbol_window) {
    last_pagelist=nsp;
    symbol_open();
    selected=lastopened;
    selx=0;
    sely=0;
    select_sym = pos_to_char(selected, selx, sely);
  } else {
    if (current_symbol_window) lastopened=current_symbol_window;
    else lastopened=selected;
    symbol_unselect();
    selected=lastopened;
    selx=0;
    sely=0;
    selected->sympage=nsp;
    select_sym = pos_to_char(selected, selx, sely);
    symbol_update_name(selected);
    selected->dx = (selected->width-offset_x)/selected->sympage->columns;
    selected->dy = (selected->height-offset_y)/selected->sympage->rows;
    XClearWindow(display, selected->symbolwin);
    symbol_draw(selected);
  }
}

void symbol_select_up(int num)
{
  if (selected) {
    push_fontgroup(SYMBOLFONT);
    draw_char(selected, selx, sely, Normal);
    num=num % selected->sympage->rows;
    sely=(sely+selected->sympage->rows -num)%selected->sympage->rows;
    select_sym = pos_to_char(selected, selx, sely);
    draw_char(selected, selx, sely, Reverse);
    pop_fontgroup();
  }
}

void symbol_select_down(int num)
{
  if (selected) {
    push_fontgroup(SYMBOLFONT);
    draw_char(selected, selx, sely, Normal);
    num=num % selected->sympage->rows;
    sely=(sely+selected->sympage->rows +num)%selected->sympage->rows;
    select_sym = pos_to_char(selected, selx, sely);
    draw_char(selected, selx, sely, Reverse);
    pop_fontgroup();
  }
}

void symbol_select_left(int num)
{
  if (selected) {
    push_fontgroup(SYMBOLFONT);
    draw_char(selected, selx, sely, Normal);
    num=num % selected->sympage->columns;
    selx=(selx+selected->sympage->columns -num)%selected->sympage->columns;
    select_sym = pos_to_char(selected, selx, sely);
    draw_char(selected, selx, sely, Reverse);
    pop_fontgroup();
  }
}

void symbol_select_right(int num)
{
  if (selected) {
    push_fontgroup(SYMBOLFONT);
    draw_char(selected, selx, sely, Normal);
    num=num % selected->sympage->columns;
    selx=(selx+selected->sympage->columns +num)%selected->sympage->columns;
    select_sym = pos_to_char(selected, selx, sely);
    draw_char(selected, selx, sely, Reverse);
    pop_fontgroup();
  }
}

void symbol_init(void)
{
    int x = BINTERSPACE/2;
    int y = BINTERSPACE/2;
    int dx, dy;

    symbol_new_page(translate("Latin1"), 16, 16);
    symbol_range_page(translate("Latin1"), 0,255);
    push_fontgroup(SYMBOLFONT);
    offset_x = x;
    offset_y = button_height+BINTERSPACE;
    selx = sely = -1;
    dx = font_width() + DEFAULTDX;
    dy = font_height() + DEFAULTDY;
    pop_fontgroup();
    last_pagelist=pagelist;
    last_width = offset_x+ dx * SIZEX + x;
    last_height = offset_y + dy * SIZEY + y;
    if (!last_xpos && !last_ypos) {
	last_xpos = (display_width - last_width) /2;
	last_ypos = (display_height - last_height) /2;
    }
    if (!XStringListToTextProperty(&name, 1, &symbol_name)) {
	message(MP_EXIT-1, translate("No location for symbolname."));
	return;
    }
    if (!XStringListToTextProperty(&iname, 1, &icon_name)) {
	message(MP_EXIT-1, translate("No location for symbol iconname."));
	return;
    }
    symbol_mask = (CWBackPixel | CWBorderPixel | CWBitGravity |
		   CWColormap | CWEventMask);
    symbol_attr.background_pixel = white_pixel;
    symbol_attr.border_pixel = black_pixel;
    symbol_attr.colormap = colormap;
    symbol_attr.bit_gravity = NorthWestGravity;
    symbol_attr.event_mask = (ExposureMask | ButtonPressMask |
			      ButtonMotionMask | PointerMotionHintMask |
			      ButtonReleaseMask | KeyPressMask |
			      StructureNotifyMask | VisibilityChangeMask);
}

void symbol_open(void)
{
    int x = BINTERSPACE/2, y = BINTERSPACE/2;
    int i;
    XSizeHints size_hints;
    SYMBOLINFO *sinfo;

    if (!(sinfo= (SYMBOLINFO *) malloc( sizeof(SYMBOLINFO)))) {
	message(MP_ERROR, translate("Not enough memory to open window."));
	return;
    }
    push_fontgroup(SYMBOLFONT);
    sinfo->dx = font_width()+DEFAULTDX;
    sinfo->dy = font_height()+DEFAULTDY;
    pop_fontgroup();
    sinfo->sympage = last_pagelist;
    sinfo->width = offset_x + sinfo->dx*sinfo->sympage->columns + x;
    sinfo->height = offset_y + sinfo->dy*sinfo->sympage->rows + y;
    if (!state_open) {
	sinfo->xpos = last_xpos;
	sinfo->ypos = last_ypos;
    } else {
	sinfo->xpos = last_xpos;
	sinfo->ypos = last_ypos;
	sinfo->width = last_width;
	sinfo->height = last_height;
    }
    sinfo->iconized = MP_True;
    sinfo->symbolwin =
	XCreateWindow(display, root_window,
		      sinfo->xpos, sinfo->ypos, sinfo->width, sinfo->height,
		      BORDERWIDTH, CopyFromParent, InputOutput,
		      visual,
		      symbol_mask, &symbol_attr);
    if (!state_open)
	size_hints.flags = PPosition | PSize | PMinSize;
    else {
	size_hints.flags = USPosition | USSize | PMinSize;
	sinfo->width = sinfo->height = 0;
    }
    size_hints.min_height =
    size_hints.min_width  = 3*button_height;
    wm_hints.initial_state = (iconic || as_icon ? IconicState : NormalState);
    i=0;
    XSetWMProperties(display, sinfo->symbolwin, &symbol_name, &icon_name,
		     NULL, 0, &size_hints, &wm_hints, &class_hints);
    set_protocols(sinfo->symbolwin);
    if (add_window(sinfo->symbolwin, SYMBOLWINDOW, root_window,
		   (void *) sinfo, translate(helpname[SYMBOLHELP])))
	while (i<NR_BUTTON && button_make(symbolbutton[i].nr, sinfo->symbolwin,
					  translate(symbolbutton[i].name), &x, y,1,
					  (void*) sinfo,
					  helpname[symbolbutton[i].helpnr],
					  NULL, NULL, symbol_handle_button,
					  symbol_handle_button, NULL, NULL))
	    i++, x+=BINTERSPACE;
    if (i<NR_BUTTON) {
	XDestroyWindow(display, sinfo->symbolwin);
	free(sinfo);
	lastopened = 0;
    } else {
        lastopened = sinfo;
	is_opened = MP_True;
	symbol_is_open = MP_True;
	nr_opened++;
	nr_iconized++;
	symbol_update_name(sinfo);
	XMapSubwindows(display, sinfo->symbolwin);
	XMapWindow(display, sinfo->symbolwin);
    }
}

