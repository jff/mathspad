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
**   File : buffer.c
**   Datum: 11-4-92
**   Doel : Het tonen, selecteren en editeren van verwijderde tekst.
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
/* #include "keymap.h" */
#include "message.h"
#include "scrollbar.h"
#include "output.h"
#include "buffer.h"
#include "editor.h"
#include "helpfile.h"

#define BUFFERNAME  "MathSpad Buffer"
#define ICONNAME    "Buffer"

enum button { KILLBUTTON, DONEBUTTON, NR_BUTTON };

static
char *bufferbutton[NR_BUTTON] = { "Kill", "Done"  };
static
int bufferhelp[NR_BUTTON] = { BUFFERKILLHELP, BUFFERDONEHELP };

static void *scrollver, *scrollhor, *buffer_info;
static Window bufwin, bufdrawwin;
static char *buffername = BUFFERNAME, *iconname = ICONNAME;
static XTextProperty buffer_name, icon_name;
static int last_xpos = 0, last_ypos = 0;
static unsigned int last_width = 0, last_height = 0;
static Bool as_icon = MP_False;
static Bool state_open = MP_False;

static unsigned int win_width, win_height;

#define sub_width(A)   (A) - INTERSPACE*3 -SCROLLBARSIZE
#define sub_height(A)  (A) - INTERSPACE*4 -SCROLLBARSIZE - button_height
#define pos_x_with     INTERSPACE*2 +SCROLLBARSIZE
#define pos_x_without  INTERSPACE
#define pos_y_with     INTERSPACE*3 +SCROLLBARSIZE +(int)button_height
#define pos_y_without  INTERSPACE*2 +(int)button_height

static void buffer_bad_end(void *data __attribute__((unused)))
{
    close_scratchwindow();
    buffer_iconized = MP_False;
    buffer_is_open = MP_False;
    destroy_window(bufwin);
}

static int buffer_margin(void *data __attribute__((unused)))
{
    return -scrollbar_line(scrollhor, 0)*font_width()+3;
}

static void buffer_draw(void *data __attribute__((unused)))
{
    redraw_window(buffer_info);
}

static void buffer_layout_change(void *data __attribute__((unused)))
{
    if (buffer_is_open) {
	if (!buffer_iconized)
	    XClearArea(display, bufdrawwin, 0, 0, 0, 0, MP_True);
	scrollbar_linesize(scrollver, line_height());
    }
}

static void buffer_handle_button(void *data __attribute__((unused)), int b_num)
{
    switch (b_num) {
    case KILLBUTTON:
	XClearWindow(display, bufdrawwin);
	clear_window(buffer_info);
	redraw_window(buffer_info);
	scrollbar_set(scrollver,0, 1);
	break;
    case DONEBUTTON:
	if (can_close_buffer) buffer_close();
	break;
    }
}

static void buffer_press(void *data __attribute__((unused)), XButtonEvent *event)
{
    if (event->window == bufdrawwin) {
	mouse_down(buffer_info,event->x-buffer_margin(NULL),event->y,
		   mouse_button);
	get_motion_hints(bufdrawwin, -1);
    }
}

static void double_click_func(void *data __attribute__((unused)))
{
    if (bufdrawwin) {
	dbl_click();
	get_motion_hints(bufdrawwin,0);
    }
}

static void buffer_release(void *data __attribute__((unused)), XButtonEvent *event)
{
    if (event->window == bufdrawwin) {
	mouse_up(event->x - buffer_margin(NULL), event->y);
	stop_motion_hints();
    }
}

static void buffer_motion(void *data __attribute__((unused)), int x, int y)
{
    mouse_move(x-buffer_margin(NULL), y);
}

static void buffer_resize(void *data __attribute__((unused)), XConfigureEvent *event)
{
    unsigned int new_width, new_height;
    int x, y;

    win_width  = last_width = event->width;
    new_width  = sub_width( event->width );
    win_height = last_height = event->height;
    new_height = sub_height(event->height );
    window_manager_added(bufwin, &x,&y);
    last_xpos = event->x-x;
    last_ypos = event->y-y;
    XResizeWindow(display, bufdrawwin, new_width-2, new_height-2);
    resize_window(buffer_info, new_width-2, new_height-2);
    scrollbar_resize(scrollver, new_height);
    scrollbar_resize(scrollhor, new_width);
}

static void buffer_scrollto(void *data __attribute__((unused)), int kind)
{
    if (!kind) {
	XClearWindow(display, bufdrawwin);
	redraw_window(buffer_info);
    } else {
	int line_nr = scrollbar_line(scrollver, 0);
	editwindow_line(buffer_info, line_nr);
    }
}

static void buffer_iconize(void *data __attribute__((unused)))
{
    buffer_iconized = MP_True;
    /*
    ** sluit invoer op buffer af
    */
}

static void buffer_deiconize(void *data __attribute__((unused)))
{
    buffer_iconized = MP_False;
    /*
    **  open invoer op buffer
    */
}

void buffer_set_number_of_lines(void *data __attribute__((unused)), int numlin)
{
    if (!data || *((Window*)data)!=bufdrawwin) return;
    scrollbar_set(scrollver,
                  line_number(buffer_info),
                  numlin);
}

static void buffer_state(void *data __attribute__((unused)), int *x, int *y, int *w, int *h,
			 int *i, int *s, Char **str)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    *i = buffer_iconized;
    *s = 0;
    *str = NULL;
}

static void buffer_use_state( int x, int y, int w, int h,
			      int i, int s __attribute__((unused)), Char *str)
{
    as_icon = i;
    state_open = MP_True;
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
    buffer_open();
    state_open = MP_False;
    as_icon = MP_False;
    free(str);
}

static int buffer_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return MP_False;
}

static void buffer_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

FUNCTIONS mainbufferfuncs = {
    buffer_bad_end, NULL, buffer_resize, NULL, NULL, NULL, buffer_iconize,
    buffer_deiconize, NULL, NULL, buffer_layout_change, NULL,
    buffer_use_state, buffer_state, NULL, NULL, buffer_last_pos,
    buffer_set_last_pos  };

FUNCTIONS bufferfuncs = {
    NULL, buffer_draw, NULL, buffer_press, buffer_release, buffer_motion,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, buffer_margin,
    buffer_set_number_of_lines, NULL, NULL, double_click_func };

void buffer_init(void)
{
    win_width = display_width/2;
    win_height= display_height/3;
    if (!XStringListToTextProperty(&buffername, 1, &buffer_name))
	message(MP_EXIT-1,translate("Can't set the name for the buffer."));
    if (!XStringListToTextProperty(&iconname, 1, &icon_name))
	message(MP_EXIT-1, translate("Can't set iconname for the buffer."));
}

void buffer_open(void)
{
    int x_pos, y_pos;
    int x = INTERSPACE;
    int y = INTERSPACE;
    unsigned int sw=4, sh=4;
    int i;
    XSetWindowAttributes buffer_attr;
    XSizeHints size_hints;

    if (buffer_is_open) {
      if (buffer_iconized) XMapWindow(display, bufwin);
      buffer_iconized=MP_False;
      XRaiseWindow(display, bufwin);
      return;
    }
    if (last_width) {
	win_width = last_width;
	win_height = last_height;
	x_pos = last_xpos;
	y_pos = last_ypos;
    } else {
	x_pos = (display_width - win_width)/2;
	y_pos = (display_height - win_height)/2;
    }
    buffer_attr.background_pixel = white_pixel;
    buffer_attr.border_pixel = black_pixel;
    buffer_attr.bit_gravity = NorthWestGravity;
    buffer_attr.colormap = colormap;
    buffer_attr.event_mask = (ExposureMask | ButtonPressMask
			      | ButtonReleaseMask | ButtonMotionMask
			      | PointerMotionHintMask | KeyPressMask
			      | FocusChangeMask
			      | StructureNotifyMask | VisibilityChangeMask);

    bufwin = XCreateWindow(display, root_window,
			   x_pos, y_pos, win_width, win_height,
			   BORDERWIDTH, CopyFromParent, InputOutput,
			   visual,
			   (CWBackPixel | CWBorderPixel | CWColormap |
			    CWBitGravity | CWEventMask), &buffer_attr);
    if (state_open) {
	size_hints.flags = USPosition | USSize | PMinSize;
	wm_hints.initial_state =
	    ((iconic || as_icon) ? IconicState : NormalState);
    } else
	size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width =
	size_hints.min_height = pos_y_with + SCROLLBARSIZE*3;
    i=0;
    XSetWMProperties(display, bufwin, &buffer_name, &icon_name,
		     NULL, 0,
		     &size_hints, &wm_hints, &class_hints);
    set_protocols(bufwin);
    if (add_window(bufwin, MAINBUFFERWINDOW, root_window,
		   NULL, translate(helpname[BUFFERHELP]))) {
	while (i<NR_BUTTON && button_make(i,bufwin,translate(bufferbutton[i]), &x, y, 1,
					  NULL, helpname[bufferhelp[i]],
					  NULL, NULL, NULL,
					  buffer_handle_button, NULL, NULL))
	    i++,x+=BINTERSPACE;
	sw = sub_width(win_width);
	sh = sub_height(win_height);
	if (i==NR_BUTTON) {
	    bufdrawwin = XCreateWindow(display, bufwin,
				       pos_x_with, pos_y_with, sw-2, sh-2, 1,
				       CopyFromParent, InputOutput,
				       visual,
				       (CWBackPixel | CWBorderPixel |
					CWBitGravity | CWEventMask),
				       &buffer_attr);
	    if (add_window(bufdrawwin, BUFFERWINDOW, bufwin,
			   NULL, translate(helpname[BUFFERHELP])))
		i++;
	}
	if (i==NR_BUTTON +1 &&
	    (scrollhor = scrollbar_make(HORIZONTAL, bufwin, pos_x_with,
					pos_y_without, sw, font_width(),
					buffer_scrollto, NULL)))
	    i++;
	if (i==NR_BUTTON+2 &&
	    (scrollver = scrollbar_make(VERTICAL, bufwin, pos_x_without,
					pos_y_with, sh, line_height(),
					buffer_scrollto, NULL)))
	    i++;
    }
    if (i<NR_BUTTON+3) {
	XDestroyWindow(display, bufwin);
	destroy_window(bufwin);
    } else {
	scrollbar_set(scrollver, 0, 1);
	scrollbar_set(scrollhor, 0, 80);
	buffer_info = open_scratchwindow((void *) &bufdrawwin, sw-2, sh-2);
	buffer_iconized = MP_False;
	buffer_is_open = MP_True;
	XMapSubwindows(display, bufwin);
	XMapWindow(display, bufwin);
    }
}

void buffer_close(void)
{
    close_scratchwindow();
    buffer_iconized = MP_False;
    buffer_is_open = MP_False;
    XDestroyWindow(display, bufwin);
    destroy_window(bufwin);
    bufwin = 0;
    bufdrawwin = 0;
}

