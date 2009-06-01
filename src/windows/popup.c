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
#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "output.h"
#include "language.h"
#include "popup.h"
#include "memman.h"
#include "helpfile.h"

static XSetWindowAttributes popup_attr;
static unsigned long popup_mask;
static XTextProperty wname, iname;
static char *win_name="PopUp";

static MENU *replace_lock=NULL;

/* for menus build without the interpreter, the Sequence field
** is used to store a SystemFunc
*/
typedef struct {
  void (*sysfunc)(void*,int);
  void *dataarg;
  int intarg;
} SystemFunc;


#define TITLESEL -2

static void popup_bad_end(void *data)
{
    MENU *m= (MENU*) data;

    if (m->freesub) {
      /* memory leak */
    }
    if (m->builded) {
      PopupLine *pl = m->menu->firstline;
      while (pl) {
	if (pl->linetype==2) free(pl->line.func);
	pl=pl->next;
      }
      /*
      popup_destroy(m->menu);
      */
    }
    replace_lock=m;
    if (m->endfunc) (*(m->endfunc))(m->enddata);
    if (replace_lock==m) {
	if (m->submenu) popup_bad_end(m->submenu);
	destroy_window(m->ownwin);
    }
    replace_lock=0;
}

static XPoint triangle[4];
static int trsize;
static Bool invisible=MP_False;
static Uchar *disablecolor=NULL;

static void draw_line(PopupLine *ml, Window win_id, int ww, int reverse)
{
    int x,y,w,i;
    thinspace(5);
    y=where_y();
    w=0;
    if (reverse && ml->enabled && ml->linetype!=0) set_text_mode(Reverse);
    if (ml->string) {
      if (!ml->enabled) {
	int ip=0;
	out_char(ColorStart);
	if (!disablecolor) disablecolor=translate("grey");
	while (disablecolor[ip]) {
	  out_char(disablecolor[ip]);
	  ip++;
	}
	out_char(ColorSep);
      }
      for (i=0; ml->string[i]; i++)
	if (IsNewline(ml->string[i])) {
	  x=where_x();
	  if (x>w) w=x;
	  thinspace(ww-5-x);
	  out_char(ml->string[i]);
	  thinspace(5);
	} else
	  out_char(ml->string[i]);
      if (!ml->enabled)
	out_char(ColorEnd);
    }
    x=where_x();
    if (x>w) w=x;
    thinspace(ww-5-x);
    if (reverse && ml->enabled && ml->linetype!=0) set_text_mode(Normal);
    out_char(Newline);
    ml->height=where_y()-y;
    ml->width=w-5;
    if (ml->linetype==1 && win_id) {
	triangle[0].x=ww-10;
	triangle[0].y=y+line_height()/2;
	XDrawLines(display,win_id,get_GCXor(PrimSel),triangle,4,CoordModePrevious);
    }
}

static void draw_title(MENU *m, Bool inverse)
{
    if (!m->menu->title) return;
    set_x_y(0,5);
    thinspace(5);
    out_char(Font2Char(FontFont,1));
    out_char(StackB);
    out_char(TopGap);
    if (inverse) set_text_mode(Reverse);
    out_string(m->menu->title);
    if (inverse) set_text_mode(Normal);
    m->tw=where_x()-5;
    thinspace(m->menu->width-m->tw-10);
    out_char(GapBottom);
    out_char(GlueLine);
    out_char(StackClose);
    out_char(Font2Char(PopFont,1));
    out_char(Newline);
    m->th=where_y()-5;
}

static void popup_draw(void *data)
{
    MENU *m = (MENU*) data;
    PopupLine *pl;
    int i=0;
    push_fontgroup(POPUPFONT);
    set_output_window((void*) &m->ownwin);
    if (invisible) set_drawstyle(INVISIBLE);
    set_x_y(0,5);
    draw_title(m,m->selline==TITLESEL);
    pl = m->menu->firstline;
    while (pl) {
      draw_line(pl, m->ownwin, m->menu->width, m->selline==i);
      i++;
      pl=pl->next;
    }
    unset_output_window();
    pop_fontgroup();
}

static void popup_layout_change(void *data)
{
    int w=0,h=10,s=0;
    MENU *m = (MENU*) data;
    push_fontgroup(POPUPFONT);
    trsize=line_height()/3; 
    pop_fontgroup();
    triangle[1].y=triangle[3].y=-trsize;
    trsize=trsize*2;
    triangle[1].x=-trsize;
    triangle[3].x=trsize;
    triangle[2].y=trsize;
    triangle[2].x=0;
    if (m) {
        XSizeHints size_hints;
        PopupLine *pl;
	invisible=MP_True;
	popup_draw(data);
	invisible=MP_False;
	w=m->tw;
	pl=m->menu->firstline;
	while (pl) {
	  if (pl->width>w) w=pl->width;
	  h+=pl->height;
	  if (pl->linetype==1) s++;
	  pl=pl->next;
	}
	if (m->menu->title) h+=m->th;
	m->menu->width=w+10+(s?8+trsize:0);
	m->menu->height=h;
	XResizeWindow(display, m->ownwin, m->menu->width, m->menu->height);
	size_hints.flags = PMinSize | PMaxSize;
	size_hints.min_height=size_hints.max_height=m->menu->height;
	size_hints.min_width=size_hints.max_width=m->menu->width;
	XSetWMNormalHints(display, m->ownwin, &size_hints);
	popup_draw(data);
    }
}

static void reselect(MENU *m, int newsel)
{
    int i,h;
    PopupLine *pl;
    if (newsel == m->selline) return;
    push_fontgroup(POPUPFONT);
    set_output_window((void*) &m->ownwin);
    h=m->th;
    pl=m->menu->firstline;
    i=0;
    while (i<newsel && pl) {
      h+=pl->height;
      i++;
      pl=pl->next;
    }
    set_x_y(0,h+5);
    if (i==newsel) draw_line(pl, m->ownwin, m->menu->width, MP_True);
    if (newsel==TITLESEL) draw_title(m,MP_True);
    h=m->th;
    i=0;
    pl=m->menu->firstline;
    while (i<m->selline && pl) {
      h+=pl->height;
      i++;
      pl=pl->next;
    }
    set_x_y(0,h+5);
    if (i==m->selline) {
      pl=m->menu->firstline;
      while (i && pl) {
	pl=pl->next;
	i--;
      }
      draw_line(pl, m->ownwin, m->menu->width, MP_False);
    }
    if (m->selline==TITLESEL) draw_title(m,MP_False);
    unset_output_window();
    pop_fontgroup();
    m->selline=newsel;
}

static int dragged;

static void popup_motion(void *data, int x, int y)
{
    MENU *m = (MENU*) data;
    if (m->submenu) {
	MENU *sm;
	sm =m->submenu;
	while (sm) {
	    XDestroyWindow(display, sm->ownwin);
	    sm = sm->submenu;
	}
	popup_bad_end((void*)m->submenu);
	m->submenu=NULL;
    }
    if (x>(int)m->menu->width || y<0 ||
	y>(int)m->menu->height || (x<0 && !m->mainmenu))
      reselect(m, -1);
    else if (x<0 && m->mainmenu) {
	when_motion_window=m->mainmenu->ownwin;
	XDestroyWindow(display, m->ownwin);
	m=m->mainmenu;
	popup_bad_end((void*) m->submenu);
	m->submenu=NULL;
    } else if (y<(int)m->th) {
	if (m->menu->pinable) reselect(m,TITLESEL);
	else reselect(m,-1);
    } else {
        PopupLine *pl;
	int i=0,h;
	pl=m->menu->firstline;
	h=5+m->th+(pl?pl->height:0);
	while (pl && h<y) {
	  pl=pl->next;
	  h+=(pl?pl->height:0);
	  i++;
	}
	if (i!=m->selline && pl) reselect(m,i);
	if (pl && pl->linetype==1 && pl->enabled &&
	    (x>(int)m->menu->width-trsize-8 ||
	     (!dragged && mouse_button==Button3))) {
	  MENU *sm, *tm;
	  tm = malloc(sizeof(MENU));
	  memset(tm, 0, sizeof(MENU));
	  tm->x = m->x+m->menu->width-trsize-8;
	  tm->y = m->y+h-pl->height;
	  tm->parentwin = m->parentwin;
	  tm->menu=pl->line.menu;
	  sm = popup_make(tm);
	  free(tm);
	  m->submenu=sm;
	  sm->mainmenu=m;
	  sm->freesub=0;
	  when_motion_window = sm->ownwin;
	}
    }
    dragged=MP_True;
}

static void make_sticky(MENU *m)
{
    XSizeHints size_hints;
    XClassHint classhints;
    XWMHints wmhints;
    Atom rsatom[2],olatom,atatom;
    XSetWindowAttributes override;

    wmhints=wm_hints;
    wmhints.flags = StateHint | InputHint | WindowGroupHint;
    wmhints.input = MP_False;
    size_hints.flags = USPosition | USSize | PMinSize | PMaxSize;
    size_hints.min_height=size_hints.max_height=m->menu->height;
    size_hints.min_width=size_hints.max_width=m->menu->width;
    classhints.res_name = "popup";
    classhints.res_class = "MathEdit";
    override.override_redirect = MP_False;
    XUnmapWindow(display, m->ownwin);
    /*
    popup_mask = popup_mask ^ CWOverrideRedirect;
    oldwin=m->ownwin;
    m->ownwin = XCreateWindow(display,root_window,m->x,m->y,m->menu->width,
			      m->menu->height,2,CopyFromParent,InputOutput,
			      visual,
			      popup_mask, &popup_attr);
    */
    XChangeWindowAttributes(display, m->ownwin,CWOverrideRedirect, &override);
    rsatom[0] = XInternAtom(display, "_OL_DECOR_RESIZE", MP_True);
    rsatom[1] = XInternAtom(display, "_OL_DECOR_HEADER", MP_True);
    olatom = XInternAtom(display, "_OL_DECOR_DEL", MP_True);
    atatom = XInternAtom(display, "ATOM", MP_True);
    if (rsatom[0] && rsatom[1] && olatom) {
	XChangeProperty(display, m->ownwin, olatom, atatom, 32,
			PropModeReplace, (void*) rsatom, 2);
    }
    XSetWMProperties(display, m->ownwin, &wname, NULL, NULL, 0,
		     &size_hints, &wmhints, &classhints);
    set_protocols(m->ownwin);
    XSetTransientForHint(display, m->ownwin, m->parentwin);
    XMapWindow(display, m->ownwin);
    /*
    XDestroyWindow(display, oldwin);
    (void) remove_window(oldwin);
    add_window(m->ownwin, POPUPWINDOW, m->parentwin, (void*) m,
	       m->menu->help);
    popup_mask = popup_mask ^ CWOverrideRedirect;
    */
    m->sticky=MP_True;
}

static void popup_press(void *data, XButtonEvent *event)
{
    MENU *m = (MENU*) data;

    popup_motion(data, event->x, event->y);
    get_motion_hints(m->ownwin,-1);
    dragged=MP_False;
}

static void remove_submenus(MENU *menu)
{
    MENU *sm, *mh;
    sm=menu;
    while (sm->submenu) sm=sm->submenu;
    while (sm) {
	mh=sm->mainmenu;
	sm->submenu=NULL;
	if (sm->sticky) {
	    sm->mainmenu=NULL;
	    sm->selline=-1;
	    popup_draw((void*)sm);
	} else {
	    if (sm->ownwin) XDestroyWindow(display, sm->ownwin);
	    popup_bad_end((void*)sm);
	}
	sm = mh;
    }
}    

static void popup_release(void *data, XButtonEvent *event __attribute__((unused)))
{
    MENU *m = (MENU*) data;
    Bool remove_it=MP_True;
    /*
      select && submenu && !dragged && button==3 -> open_submenu()
      select && submenu && dragged  || button!=3 -> submenu_default_func()
      select && !submenu                         -> line_func()
      remove_none_sticky()
    */
    stop_motion_hints();
    replace_lock=m;
    if (m->selline>=0) {
	PopupLine *pl=m->menu->firstline;
	int i;
	for (i=0; i<m->selline && pl; i++) { pl=pl->next; }
	if (pl->linetype==1) {
	    if (dragged || mouse_button!=3) {
		PopupLine *pldef=pl->line.menu->firstline;
		while (pldef && (!pldef->defaultline || pldef->linetype!=2)) {
		  if (!pldef->defaultline) pldef=pldef->next;
		  else if (pldef->linetype==1) {
		    pldef=pldef->line.menu->firstline;
		  } else {
		    pldef=0;
		  }
		}
		if (pldef) pl=pldef;
	    } else {
	      remove_it = MP_False;
	    }
	}
	if (pl->linetype==2 && pl->enabled) {
	  if (m->builded) {
	    SystemFunc *sf;
	    sf = (SystemFunc*) pl->line.func;
	    (*(sf->sysfunc))(sf->dataarg, sf->intarg);
	  } else {
	    eval_sequence(pl->line.func);
	  }
	}
    } else if (m->selline==TITLESEL) {
        if (aig(m->sticky= !m->sticky))
	    make_sticky(m);
    }
    if (remove_it && replace_lock==m) {
      /* remove popups (except the sticky one) */
      remove_submenus(m);
    }
    replace_lock=0;
}

static void popup_resize(void *data, XConfigureEvent *event)
{
    MENU *m = (MENU*) data;
    int change=((int)m->menu->width!=event->width || (int)m->menu->height!=event->height);
    m->x=event->x;
    m->y=event->y;
    if (change) {
	m->menu->width=event->width;
	m->menu->height=event->height;
	popup_draw(data);
    }
}

FUNCTIONS popupfuncs = {
    popup_bad_end, popup_draw, popup_resize, popup_press, popup_release,
    popup_motion, NULL, NULL, NULL, NULL, popup_layout_change };

void popup_init(void)
{
    push_fontgroup(POPUPFONT);
    trsize=line_height()/3;
    pop_fontgroup();
    triangle[1].y=triangle[3].y=-trsize;
    trsize=trsize*2;
    triangle[1].x=-trsize;
    triangle[3].x=trsize;
    triangle[2].y=trsize;
    triangle[2].x=0;
    popup_mask = (CWBackPixel | CWBorderPixel | CWEventMask | CWColormap |
		  CWSaveUnder | CWOverrideRedirect);
    popup_attr.background_pixel = white_pixel;
    popup_attr.colormap = colormap;
    popup_attr.border_pixel = black_pixel;
    popup_attr.event_mask = (ExposureMask | ButtonPressMask |
			     ButtonReleaseMask | KeyPressMask |
			     ButtonMotionMask | PointerMotionHintMask |
			     OwnerGrabButtonMask | StructureNotifyMask |
			     VisibilityChangeMask );
    popup_attr.save_under = MP_True;
    popup_attr.override_redirect = MP_True;
    XStringListToTextProperty(&win_name, 1, &wname);
    XStringListToTextProperty(&win_name, 1, &iname);
}

void popup_remove(Window win)
{
    MENU *m;
    int i;
    i=0;
    while (aig(m = (MENU*) next_data_with_type(POPUPWINDOW, &i))) {
	if (m->parentwin==win) {
	    if (m->mainmenu) {
		m->mainmenu->submenu=NULL;
		m->mainmenu=NULL;
	    }
	    if (m->submenu) {
		m->submenu->mainmenu=NULL;
		m->submenu=NULL;
	    }
	    XDestroyWindow(display, m->ownwin);
	    popup_bad_end(m);
	} else
	    i++;
    }
}

void popup_unmap(Window win)
{
    MENU *m;
    int i;
    i=0;
    while (aig(m = (MENU*) next_data_with_type(POPUPWINDOW, &i))) {
	if (m->parentwin==win)
	    XUnmapWindow(display, m->ownwin);
	i++;
    }
}

void popup_map(Window win)
{
    MENU *m;
    int i;
    i=0;
    while (aig(m = (MENU*) next_data_with_type(POPUPWINDOW, &i))) {
	if (m->parentwin==win)
	    XMapWindow(display, m->ownwin);
	i++;
    }
}

MENU *popup_make(MENU *menu)
{
    int xp,yp,i,h;
    MENU *m;
    PopupLine *pl;
 
    if (!menu || !menu->menu || !menu->menu->firstline) {
      return 0;
    }
    m = (MENU*) malloc(sizeof(MENU));
    *m = *menu;
    m->th=m->tw=0;
    m->mainmenu=NULL;
    m->submenu=NULL;
    m->selline= -1;
    xp=m->x;
    yp=m->y;
    if (xp==-1 && yp==-1) {
	Window root,child;
	int rx,ry;
	unsigned int buttons;
	(void) XQueryPointer(display, root_window, &root, &child,
			     &rx, &ry, &xp, &yp, &buttons);
    }
    m->ownwin = *((Window*)test_window());
    popup_layout_change((void*) m);
    i=0;
    h=m->th;
    pl=m->menu->firstline;
    while (pl && !pl->defaultline) {
      h+=pl->height;
      pl=pl->next;
    }
    if (!pl) h=m->th;
    xp-=10;
    yp-=h;
    if (xp<0) xp=0;
    if (yp<0) yp=0;
    if (xp+m->menu->width > display_width) xp=display_width - m->menu->width;
    if (yp+m->menu->height> display_height) yp=display_height - m->menu->height;
    m->ownwin = XCreateWindow(display, root_window, xp, yp, m->menu->width,
			      m->menu->height, 2, CopyFromParent, InputOutput,
			      visual,
			      popup_mask, &popup_attr);
    m->x=xp;m->y=yp;
    if (!m->menu->help) m->menu->help=translate(helpname[PINUPHELP]);
    add_window(m->ownwin, POPUPWINDOW, m->parentwin, (void*) m, m->menu->help);
    XMapWindow(display, m->ownwin);
    when_motion_window = m->ownwin;
    return m;
}

void popup_call_default(MENU *m)
{
  PopupLine *pldef;
  pldef = m->menu->firstline;
  while (pldef && (!pldef->defaultline || pldef->linetype!=2)) {
    if (!pldef->defaultline) pldef=pldef->next;
    else if (pldef->linetype==1 && pldef->enabled) {
      pldef=pldef->line.menu->firstline;
    } else {
      pldef=0;
    }
  }
  if (pldef && pldef->linetype==2 && pldef->enabled) {
    if (m->builded) {
      SystemFunc *sf;
      sf = (SystemFunc*) pldef->line.func;
      (*(sf->sysfunc))(sf->dataarg, sf->intarg);
    } else {
      eval_sequence(pldef->line.func);
    }
  }
}

void popup_set_termfunc(MENU *menu, void (*func)(void*), void *data)
{
  if (menu) {
    menu->endfunc=func;
    menu->enddata=data;
  }
}

/* create a menu object to be filled with add_item calls */
MENU *build_menu(Char *title)
{
    MENU *m=malloc(sizeof(MENU));
    memset(m,0,sizeof(MENU));
    m->menu = popup_define(translate("InternalBuildMenu"));
    popup_set_title(m->menu, title);
    popup_pinable(m->menu);
    m->selline= -1;
    m->parentwin=wm_hints.window_group;
    m->x=-1;
    m->y=-1;
    m->freesub=1;
    m->endfunc=0;
    m->enddata=0;
    m->builded=1;
    return m;
}

/* add an item to the menu.
** STRING  : the string to be displayed
** FUNC    : the function to call when this item is selected
** DATAARG : pointer to pass as first argument to FUNC
** INTARG  : integer to pass as second argument to FUNC
**
** Example:  add_item(newpopup, "Apply Term",  send_wrapper,
**                    "ACTION<Apply>", 0);
** For a separator, the FUNC argument should be NULL:
**
**           add_item(newpopup, "", NULL, NULL,0);
*/
void add_item(MENU *menu, Char *string, void (*func)(void*,int),
	      void *dataarg, int intarg)
{
  if (func) {
    SystemFunc *sf;
    sf = malloc(sizeof(SystemFunc));
    sf->sysfunc=func;
    sf->dataarg=dataarg;
    sf->intarg=intarg;
    popup_add_line(menu->menu, string, (Sequence*)sf);
  } else {
    popup_add_separator(menu->menu);
  }
}

/* check if MENU is still connected to a popup window */
static int valid_popup(MENU *menu)
{
    MENU *m;
    int i;
    i=0;
    while (aig(m= (MENU*) next_data_with_type(POPUPWINDOW, &i))) {
	if (m==menu) return 1;
	i++;
    }
    return 0;
}

/* Replace MENUOLD with MENUNEW.  If MENUOLD is visible on screen,
** MENUNEW will use the same window and replace the available options.
** If MENUOLD is not visible on screen or not defined, MENUNEW is openend.
** If MENUNEW is not defined, MENUOLD will be removed if visible.
**
** It returns the MENU structure which could be used in a next call to
** popup_replace.
*/
MENU *popup_replace(MENU *menuold, MENU *menunew)
{
    MENU *retmenu=0;
    if (menuold && valid_popup(menuold)) {
	MENU *sm;
	int unset_lock=0;

	if (replace_lock==menuold) unset_lock=1;
	retmenu=menuold;
	menuold=malloc(sizeof(MENU));
	*menuold=*retmenu;
	/* free memory used by menuold */
	menuold->ownwin=0;
	sm=menuold->submenu;
	while (sm) {
	    if (replace_lock==sm) unset_lock=1;
	    sm=sm->submenu;
	}
	remove_submenus(menuold);
	if (unset_lock) replace_lock=NULL;
    } else {
	retmenu=menunew;
    }
    if (menunew) {
	Window window,transwin;
	int sticky, xp,yp;
	xp=retmenu->x;
	yp=retmenu->y;
	sticky=retmenu->sticky;
	window=retmenu->ownwin;
	transwin=retmenu->parentwin;
	*retmenu=*menunew;
	retmenu->mainmenu=retmenu->submenu=NULL;
	retmenu->th=retmenu->tw=0;
	/* determine size of popup by drawing it on a dummy window */
	retmenu->ownwin = *((Window*)test_window());
	popup_layout_change((void*)retmenu);

	/* replace original fields */
	if (!transwin) transwin=wm_hints.window_group;
	retmenu->parentwin=transwin;
	retmenu->ownwin=window;
	retmenu->sticky=sticky;
	if (!retmenu->menu->help) retmenu->menu->help=translate(helpname[PINUPHELP]);
	if (xp==-1 && yp==-1) {
	    /* if position is not known, determine it */
	    Window root,child;
	    int rx,ry;
	    unsigned int buttons;
	    (void) XQueryPointer(display, root_window, &root, &child,
				 &rx, &ry, &xp, &yp, &buttons);
	    if (!window) {
		int h;
		PopupLine *pldef;
		h=retmenu->th;
		pldef=retmenu->menu->firstline;
		while (pldef && !pldef->defaultline) {
		  h=h+pldef->height;
		  pldef=pldef->next;
		}
		xp-=10;
		yp-=h;
	    }
	}
	/* adjust position if size requests it */
	retmenu->x=xp;
	retmenu->y=yp;
	if (xp<0) xp=0;
	if (yp<0) yp=0;
	if (xp+retmenu->menu->width > display_width)
            xp=display_width - retmenu->menu->width;
	if (yp+retmenu->menu->height> display_height)
            yp=display_height - retmenu->menu->height;
	/* create a new window or adjust the current one */
	if (!window) {
	    window = XCreateWindow(display, root_window, xp, yp,
				   retmenu->menu->width, retmenu->menu->height,
				   2, CopyFromParent, InputOutput, visual,
				   popup_mask, &popup_attr);
	    add_window(window, POPUPWINDOW, transwin, (void*) retmenu,
		       retmenu->menu->help);
	    retmenu->ownwin=window;
	} else {
	    if (xp!=retmenu->x || yp!=retmenu->y) {
		XMoveWindow(display, window, xp, yp);
	    }
	    XResizeWindow(display, window, retmenu->menu->width, retmenu->menu->height);
	}
	retmenu->x=xp;
	retmenu->y=yp;
	XMapWindow(display, window);
	XClearArea(display, window, 0,0,0,0,MP_True);
	when_motion_window = window;
	if (retmenu!=menunew) free(menunew);
    } else {
	if (retmenu) {
	    if (retmenu->ownwin) XDestroyWindow(display, retmenu->ownwin);
	    remove_window(retmenu->ownwin);
	    free(retmenu);
	    retmenu=0;
	}
    }
    return retmenu;
}

void update_popups(void)
{
  int i=0;
  void *data;
  data = next_data_with_type(POPUPWINDOW, &i);
  while (data) {
    popup_layout_change(data);
    i++;
    data=next_data_with_type(POPUPWINDOW, &i);
  }
}
