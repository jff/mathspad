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
**   File : find.c
**   Datum: 29-4-93
**   Doel : Het window voor het tonen van de te zoeken en vervangen boom.
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
#include "message.h"
#include "output.h"
#include "scrollbar.h"
/* #include "keymap.h" */
#include "find.h"
#include "fileread.h"
#include "fileselc.h"
#include "notatype.h"
#include "editor.h"
#include "popup.h"
#include "helpfile.h"

#define FINDNAME  "Find & Replace"
#define ICONNAME    "Find"

enum button { FINDBUTTON, STACKBUTTON, DONEBUTTON, NR_BUTTON,
	      FINDCOD, REPLCOD, REPFCOD, REPACOD, LOADCOD,
	      SAVECOD, PREVCOD, NEXTCOD, NEWSCOD, CLEACOD };
static
char *findbutton[NR_BUTTON] = { "Find", "Stack", "Done" };
static
int findhelp[NR_BUTTON] = { FINDFINDHELP, FINDSTACKHELP, FINDDONEHELP };

static void find_handle_button(void*,int n);
/*
static Char name1[] = { 'F','i','n','d',0};
static Char name2[] = { 'R','e','p','l','a','c','e',0};
static Char name3[] = { 'R','e','p','l','a','c','e','&','F','i','n','d',0};
static Char name4[] = { 'R','e','p','l','a','c','e',' ','A','l','l',0};
static Char name5[] = { 'L','o','a','d','.','.','.',0};
static Char name6[] = { 'S','a','v','e','.','.','.',0};
static Char name7[] = { 'P','r','e','v',0};
static Char name8[] = { 'N','e','x','t',0};
static Char name9[] = { 'N','e','w',0};
static Char name10[] = { 'C','l','e','a','r',0};

static MENULINE findmenu[] =
{ {name1 , 4, 0, 0, find_handle_button, NULL, FINDCOD, NULL },
  {name2 , 7, 0, 0, find_handle_button, NULL, REPLCOD, NULL },
  {name3 ,12, 0, 0, find_handle_button, NULL, REPFCOD, NULL },
  {name4 ,11, 0, 0, find_handle_button, NULL, REPACOD, NULL }};
static MENULINE stackmenu[] =
{ {name5 , 7, 0, 0, find_handle_button, NULL, LOADCOD, NULL },
  {name6 , 7, 0, 0, find_handle_button, NULL, SAVECOD, NULL },
  {name7 , 4, 0, 0, find_handle_button, NULL, PREVCOD, NULL },
  {name8 , 4, 0, 0, find_handle_button, NULL, NEXTCOD, NULL },
  {name9 , 3, 0, 0, find_handle_button, NULL, NEWSCOD, NULL },
  {name10, 5, 0, 0, find_handle_button, NULL, CLEACOD, NULL }
};
*/
static MENU menufind[NR_BUTTON] = {
  {0,0,-1,-1,-1,-1,-1,0,0,0,0},
  {0,0,-1,-1,-1,-1,-1,0,0,0,0}
};
static char *menuname[NR_BUTTON] = { "FindCommandMenu", "FindStackMenu" };

static void *scrollfind, *scrollrep, *find_info, *rep_info;
static int last_xpos = 0, last_ypos = 0, last_width=0, last_height = 0;
static Window findwin, finddrawwin, repdrawwin;
static char *findname = FINDNAME, *iconname = ICONNAME;
static XTextProperty find_name, icon_name;
static unsigned int win_width, win_height;

#define sub_width(A)   (A) - INTERSPACE*3 -SCROLLBARSIZE
#define sub_height(A)  ((A) - INTERSPACE*4 - button_height)/2
#define pos_x_with     INTERSPACE*2 +SCROLLBARSIZE
#define pos_x_without  INTERSPACE
#define pos_y_with(A)  INTERSPACE*3 + (int) button_height +(int)sub_height(A)
#define pos_y_without  INTERSPACE*2 + (int) button_height

static int find_margin(void *data __attribute__((unused)))
{
    return 3;
}

static void find_bad_end(void *data __attribute__((unused)))
{
    popup_remove(findwin);
    close_findwindow();
    close_replacewindow();
    find_iconized = MP_False;
    find_is_open = MP_False;
    destroy_window(findwin);
}

static void find_draw(void *data)
{
    Window win = *((Window *) data);
    if (win == finddrawwin)
	redraw_window(find_info);
    else if (win == repdrawwin)
	redraw_window(rep_info);
}

static void find_layout_change(void *data __attribute__((unused)))
{
    if (find_is_open) {
	if (!find_iconized) {
	    XClearArea(display, finddrawwin, 0, 0, 0, 0, MP_True);
	    XClearArea(display, repdrawwin, 0, 0, 0, 0, MP_True);
	}
	scrollbar_linesize(scrollfind, line_height());
	scrollbar_linesize(scrollrep, line_height());
    }
}

/*
static void find_load_stack(void *data __attribute__((unused)), Char *name)
{
    if (name) {
	FILE *f;
	if (!(f = fopen((char*)UstrtoFilename(name),"rb"))) {
	    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
	    free(name);
	    failure=MP_True;
	} else {
	    int i;
	    i = edit_fnr;
	    edit_fnr = 0;
	    read_file(f,FINDREPFILE);
	    unset_file();
	    fclose(f);
	    cleanup_nodestack();
	    cleanup_filestack();
	    cleanup_stencilstack();
	    free(name);
	}
    }
}

static void find_save_stack(void *data __attribute__((unused)), Char *name)
{
    if (name) {
	FILE *f;
	if (!(f=fopen((char*)UstrtoFilename(name), "wb"))) {
	    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
	    free(name);
	    failure=MP_True;
	} else {
	    set_file(f);
	    put_filecode(FINDREPFILE);
	    put_findrep();
	    unset_file();
	    cleanup_stencilstack();
	    fclose(f);
	    free(name);
	}
    }
}
*/
static void find_handle_button(void *data __attribute__((unused)), int b_num)
{
    Bool ph_match = MP_True;

    switch (b_num) {
    case FINDBUTTON:
    case STACKBUTTON:
	if (mouse_button == Button3) {
	    menufind[b_num].parentwin=findwin;
	    menufind[b_num].menu=popup_define(translate(menuname[b_num]));
	    popup_make(&menufind[b_num]);
	} else {
	    menufind[b_num].menu=popup_define(translate(menuname[b_num]));
	    popup_call_default(menufind+b_num);
	}
	break;
/*    case FINDCOD:
	if (aig(ph_match = check_find())) {
	    if (!find_tree())
		message(MP_ERROR, translate("Not found."));
	}
	break;
    case REPLCOD:
	if (aig(ph_match = check_find_replace()))
	    replace_tree();
	break;
    case REPFCOD:
	if (aig(ph_match = check_find_replace())) {
	    replace_tree();
	    if (!find_tree())
		message(MP_ERROR, translate("Not found."));
	}
	break;
    case REPACOD:
	if (aig(ph_match = check_find_replace()))
	    replace_all_tree();
	break;
    case LOADCOD:
	fileselc_open(find_load_stack, NULL, translate("Load Find&Replace Stack:"),
		      userdir, translate("*.mpr"), NULL, findwin);
	break;
    case SAVECOD:
	fileselc_open(find_save_stack, NULL, translate("Save Find&Replace Stack:"),
		      userdir, translate("*.mpr"), NULL, findwin);
	break;
    case PREVCOD:
	find_prev_on_stack();
	break;
    case NEXTCOD:
	find_next_on_stack();
	break;
    case NEWSCOD:
	find_new_on_stack();
	break;
    case CLEACOD:
	remove_find_stack();
	break;
*/    case DONEBUTTON:
	if (can_close_find) find_close();
	break;
    }
    if (!ph_match)
	message(MP_ERROR, translate("Place holders don't match."));
}

static void find_press(void *data __attribute__((unused)), XButtonEvent *event)
{
    if (event->window == finddrawwin) {
	mouse_down(find_info, event->x-3, event->y, mouse_button);
	get_motion_hints(finddrawwin, -1);
    } else if (event->window == repdrawwin) {
	mouse_down(rep_info, event->x-3, event->y, mouse_button);
	get_motion_hints(repdrawwin, -1);
    }
}

static void double_click_func(void *data)
{
    if (*((Window*) data)== finddrawwin) {
	dbl_click();
	get_motion_hints(finddrawwin,0);
    } else if (*((Window*) data) == repdrawwin) {
	dbl_click();
	get_motion_hints(repdrawwin,0);
    }
}

static void find_release(void *data __attribute__((unused)), XButtonEvent *event)
{
    if (event->window == finddrawwin || event->window == repdrawwin) {
	mouse_up(event->x-3, event->y);
	stop_motion_hints();
    }
}

static void find_motion(void *data __attribute__((unused)), int x, int y)
{
    mouse_move(x-3, y);
}

static void find_resize(void *data __attribute__((unused)), XConfigureEvent *event)
{
    int x,y;
    unsigned int new_width, new_height;

    if (event->window == findwin) {
	win_width  = last_width = event->width;
	new_width  = sub_width( event->width );
	win_height = last_height = event->height;
	new_height = sub_height(event->height );
	window_manager_added(findwin, &x, &y);
	last_xpos = event->x-x;
	last_ypos = event->y-y;
	XResizeWindow(display, finddrawwin, new_width-2, new_height-2);
	XMoveWindow(display, repdrawwin, pos_x_with, pos_y_with(win_height));
	XResizeWindow(display, repdrawwin, new_width-2, new_height-2);
	resize_window(find_info, new_width-2, new_height-2);
	resize_window(rep_info, new_width-2, new_height-2);
	scrollbar_resize(scrollfind, new_height);
	scrollbar_move(scrollrep, pos_x_without, pos_y_with(win_height));
	scrollbar_resize(scrollrep, new_height);
    }
}

static void find_scrollto(void *data, int kind __attribute__((unused)))
{
    Window win = *((Window*) data);
    if (win == finddrawwin) {
	int line_nr = scrollbar_line(scrollfind, 0);
	editwindow_line(find_info, line_nr);
    } else {
	int line_nr = scrollbar_line(scrollrep, 0);
	editwindow_line(rep_info, line_nr);
    }
}

static void find_iconize(void *data __attribute__((unused)))
{
    find_iconized = MP_True;
    popup_unmap(findwin);
    /*
    ** sluit invoer op find af
    */
}

static void find_deiconize(void *data __attribute__((unused)))
{
    find_iconized = MP_False;
    popup_map(findwin);
    /*
    **  open invoer op find
    */
}

void find_set_number_of_lines(void *window, int numlin)
{
    if (*((Window*)window)==finddrawwin)
	scrollbar_set(scrollfind, line_number(find_info), numlin);
    else if (*((Window*)window)==repdrawwin)
	scrollbar_set(scrollrep, line_number(rep_info), numlin);

}

static int find_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return MP_False;
}

static void find_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

FUNCTIONS findfuncs = {
    find_bad_end, find_draw, find_resize, find_press, find_release,
    find_motion, find_iconize, find_deiconize, NULL, NULL, find_layout_change,
    NULL, NULL, NULL, find_margin, find_set_number_of_lines, find_last_pos,
    find_set_last_pos, double_click_func };

void find_init(void)
{
    win_width = display_width/2;
    win_height= display_height/3;
    if (!XStringListToTextProperty(&findname, 1, &find_name))
	message(MP_EXIT-1,translate("Can't set the name for the find."));
    if (!XStringListToTextProperty(&iconname, 1, &icon_name))
	message(MP_EXIT-1, translate("Can't set iconname for the find."));
}

void find_open(void)
{
    int x_pos, y_pos;
    int x = INTERSPACE;
    int y = INTERSPACE;
    unsigned int w=4,h=4;
    int i;
    XSetWindowAttributes find_attr;
    XSizeHints size_hints;

    if (find_is_open) {
      if (find_iconized) XMapWindow(display, findwin);
      find_iconized = MP_False;
      XRaiseWindow(display, findwin);
      return;
    }
    if (last_width) {
	x_pos = last_xpos;
	y_pos = last_ypos;
	win_width = last_width;
	win_height = last_height;
    } else {
	x_pos = (display_width - win_width)/2;
	y_pos = (display_height - win_height)/2;
    }
    find_attr.background_pixel = white_pixel;
    find_attr.border_pixel = black_pixel;
    find_attr.colormap = colormap;
    find_attr.bit_gravity = NorthWestGravity;
    find_attr.event_mask = (  ExposureMask | ButtonPressMask
			    | ButtonReleaseMask | ButtonMotionMask
			    | PointerMotionHintMask | KeyPressMask
			    | FocusChangeMask
			    | StructureNotifyMask | VisibilityChangeMask);

    findwin = XCreateWindow(display, root_window,
			   x_pos, y_pos, win_width, win_height,
			   BORDERWIDTH, CopyFromParent, InputOutput,
			   visual,
			   (CWBackPixel | CWBorderPixel | CWColormap |
			    CWBitGravity | CWEventMask), &find_attr);
    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width =
	size_hints.min_height = pos_y_without + SCROLLBARSIZE*3;
    i=0;
    XSetWMProperties(display, findwin, &find_name, &icon_name,
		     NULL, 0,
		     &size_hints, &wm_hints, &class_hints);
    set_protocols(findwin);
    if (add_window(findwin, FINDWINDOW, root_window,
		   NULL, translate(helpname[FINDHELP]))) {
	while (i<NR_BUTTON &&
	       button_make(i,findwin,translate(findbutton[i]), &x, y, 1, NULL,
			   helpname[findhelp[i]], NULL,
			   NULL, find_handle_button, find_handle_button,
			   find_handle_button, NULL))
	    i++,x+=BINTERSPACE;
	w = sub_width(win_width);
	h = sub_height(win_height);
	if (i==NR_BUTTON) {
	    finddrawwin = XCreateWindow(display, findwin,
					pos_x_with, pos_y_without,
					w-2, h-2, 1,
					CopyFromParent, InputOutput,
					visual,
					(CWBackPixel | CWBorderPixel |
					 CWBitGravity | CWEventMask),
					&find_attr);
	    if (add_window(finddrawwin, FINDWINDOW, findwin,
			   NULL, translate(helpname[FINDUPPERHELP])))
		i++;
	}
	if (i==NR_BUTTON+1) {
	    repdrawwin = XCreateWindow(display, findwin,
				       pos_x_with, pos_y_with(win_height),
				       w-2, h-2, 1,
				       CopyFromParent, InputOutput,
				       visual,
				       (CWBackPixel | CWBorderPixel |
					CWBitGravity | CWEventMask),
				       &find_attr);
	    if (add_window(repdrawwin, FINDWINDOW, findwin,
			   NULL, translate(helpname[FINDLOWERHELP])))
		i++;
	}
	if (i==NR_BUTTON +2 &&
	    (scrollrep = scrollbar_make(VERTICAL, findwin,
					pos_x_without, pos_y_with(win_height),
					h, line_height(),
					find_scrollto, (void*)(&repdrawwin))))
	    i++;
	if (i==NR_BUTTON+3 &&
	    (scrollfind = scrollbar_make(VERTICAL, findwin,
					 pos_x_without, pos_y_without,
					 h, line_height(), find_scrollto,
					 (void*)(&finddrawwin))))
	    i++;
    }
    if (i<NR_BUTTON+4) {
	XDestroyWindow(display, findwin);
	destroy_window(findwin);
    } else {
	scrollbar_set(scrollfind, 0, 1);
	scrollbar_set(scrollrep, 0, 80);
	rep_info = open_replacewindow((void *) &repdrawwin, w-2, h-2);
	find_info = open_findwindow((void *) &finddrawwin, w-2, h-2);
	/* if (clear) */
	clear_window(rep_info);
	clear_window(find_info);
	find_iconized = MP_False;
	find_is_open = MP_True;
	XMapSubwindows(display, findwin);
	XMapWindow(display, findwin);
    }
}

void find_close(void)
{
    XDestroyWindow(display, findwin);
    find_bad_end(NULL);
}
