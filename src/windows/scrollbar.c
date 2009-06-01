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
**  File  : scrollbar.c
**  Datum : 29-3-92
**  Doel  : Het maken, tekenen en gebruiken van scrollbars
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "message.h"
#include "scrollbar.h"
#include "helpfile.h"

#define minimum_size(A )  if ((A) < 2*SCROLLBARSIZE) A = 2*SCROLLBARSIZE
#define SCROLLBARSIZED 0

Cursor cursors[] = { XC_sb_down_arrow,
                     XC_sb_h_double_arrow,
                     XC_sb_left_arrow,
                     XC_sb_right_arrow,
                     XC_sb_up_arrow,
                     XC_sb_v_double_arrow };
int n_cursors = 6;

/* A -> verticale scrollbar */

#define normalcursor(A)    ((A) ? cursors[5] : cursors[1] )
#define upleftcursor(A)    ((A) ? cursors[4] : cursors[2] )
#define middlecursor(A)    ((A) ? cursors[3] : cursors[0] )
#define downrightcursor(A) ((A) ? cursors[0] : cursors[3] )

/*
#define UPTRIANGLE    0
#define DOWNTRIANGLE  1
#define LEFTTRIANGLE  2
#define RIGHTTRIANGLE 3
*/

/*
**  Button1    tekst naar links/boven
**  Button2    tekst schuiven (draggen)
**  Button3    tekst naar rechts/onder
*/

#define remap_button(A, B)  ((A) ? ((B)==Button1 ? Button3 : \
                                    ((B)==Button3 ? Button1 : Button2)) \
                                 : (B))

/*
XPoint triangle[4][4] = {{{  SCROLLBARSIZE/2,   SCROLLBARSIZE/4},
                          {3*SCROLLBARSIZE/4, 3*SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/4, 3*SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/2,   SCROLLBARSIZE/4}},

                         {{   SCROLLBARSIZE/4,   SCROLLBARSIZE/4},
                          {3*SCROLLBARSIZE/4,   SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/2, 3*SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/4,   SCROLLBARSIZE/4}},

                         {{   SCROLLBARSIZE/4,   SCROLLBARSIZE/2},
                          {3*SCROLLBARSIZE/4,   SCROLLBARSIZE/4},
                          {3*SCROLLBARSIZE/4, 3*SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/4,   SCROLLBARSIZE/2}},

                         {{   SCROLLBARSIZE/4,   SCROLLBARSIZE/4},
                          {3*SCROLLBARSIZE/4,   SCROLLBARSIZE/2},
                          {  SCROLLBARSIZE/4, 3*SCROLLBARSIZE/4},
                          {  SCROLLBARSIZE/4,   SCROLLBARSIZE/4}}
                        };
*/

typedef struct {
           Window win_id;
           Bool vertical;
	   Bool shrt;
           int startpix, sizepix, sb_size, nr_lines,
	       first_line, line_size;
	   void (*func)(void*,int);
	   void *data;
        } SCROLLBARINFO;

static unsigned long scrollbar_mask;
static XSetWindowAttributes scrollbar_attr;

/*static void draw_triangle(Window win, int x_pos, int y_pos, int kind)
{
    XPoint temptri[4];
    int i;
    
    for (i=0; i<4; i++) {
	temptri[i].x = x_pos + triangle[kind][i].x;
	temptri[i].y = y_pos + triangle[kind][i].y;
    }
    XDrawRectangle(display, win, get_GC(Normal,0,0), x_pos, y_pos,
		   SCROLLBARSIZE, SCROLLBARSIZE);
    XDrawLines(display, win, get_GC(Normal,0,0), temptri, 4, CoordModeOrigin);
} */

static void draw_rectangle(Window win, int x, int xs, int kind, TextMode col)
{
    if (kind)
	XFillRectangle(display, win, get_GC(col,0,0),
		       1, x, SCROLLBARSIZE-1, xs);
    else
	XFillRectangle(display, win, get_GC(col,0,0),
		       x, 1, xs, SCROLLBARSIZE-1);
}

static int pos_to_line(int pos, int size, int nr_lines)
{
    int i;

    i= ((pos - SCROLLBARSIZED) * nr_lines) / (size - 2*SCROLLBARSIZED);
    if (i>=nr_lines) i=nr_lines-1;
    if (i<0) return 0;
    return i;
}

static int line_to_pos(int line, int size, int nr_lines)
{
    int i;

    if (nr_lines<=0) return SCROLLBARSIZED+1;
    i = ((size- 2*SCROLLBARSIZED) * line / nr_lines + SCROLLBARSIZED);
    if (i> size-SCROLLBARSIZED-1) return size-SCROLLBARSIZED-1;
    if (i< SCROLLBARSIZED+1) return SCROLLBARSIZED+1;
    return i;
}

static int start_pixel(SCROLLBARINFO *sbi)
{
    return line_to_pos(sbi->first_line, sbi->sb_size, sbi->nr_lines);
}

static int end_pixel(SCROLLBARINFO *sbi)
{
    if (sbi->nr_lines)
	return line_to_pos(sbi->first_line + sbi->sb_size/sbi->line_size,
			   sbi->sb_size, sbi->nr_lines);
    else
	return sbi->sb_size - SCROLLBARSIZED-1;
}

static void redraw_rectangle(SCROLLBARINFO *sbi)
{
    int ns, ne, os, oe, h;
    
    ns = start_pixel(sbi);
    ne = end_pixel(sbi);
    if (ne==ns)
	if (ns>SCROLLBARSIZED+1)
	    ns--;
	else
	    ne++;
    os = sbi->startpix;
    oe = os + sbi->sizepix;
    sbi->startpix = ns;
    sbi->sizepix = ne-ns;
    if (oe<ns) h=oe,oe=ns,ns=h;
    if (os<ne) h=os,os=ne,ne=h;
    if (ns<ne)
	draw_rectangle(sbi->win_id, ns, ne-ns, sbi->vertical, Normal);
    if (ns>ne)
	draw_rectangle(sbi->win_id, ne, ns-ne, sbi->vertical, Reverse);
    if (os>oe)
	draw_rectangle(sbi->win_id, oe, os-oe, sbi->vertical, Normal);
    if (os<oe)
	draw_rectangle(sbi->win_id, os, oe-os, sbi->vertical, Reverse);
}

void *scrollbar_make(int kind, Window parent, int x_offset, int y_offset,
                     unsigned int size, int linesize,
		     void (*func)(void*,int), void *data)
{
    SCROLLBARINFO *sbi;
    int sb_height,sb_width;
    
    if ( (sbi = (SCROLLBARINFO *) malloc( sizeof(SCROLLBARINFO) )) == NULL) {
	message(MP_ERROR, translate("Out of memory in scrollbar."));
	return NULL;
    }
    
    minimum_size(size);
    sbi->sb_size = size;
    if (aig(sbi->vertical = kind&0x01 )) {
	sb_height = size;
	sb_width = SCROLLBARSIZE+1;
    } else {
	sb_width = size;
	sb_height = SCROLLBARSIZE+1;
    }
    sbi->shrt       = kind/2;
    sbi->func       = func;
    sbi->data       = data;
    sbi->nr_lines   = 0;
    sbi->first_line = 0;
    sbi->line_size  = linesize;
    sbi->startpix   = SCROLLBARSIZED+1;
    sbi->sizepix    = sbi->sb_size - (SCROLLBARSIZED +1)*2;
    scrollbar_attr.cursor = normalcursor( sbi->vertical);
    sbi->win_id = XCreateWindow(display, parent, x_offset, y_offset,
				sb_width, sb_height, 0, CopyFromParent,
				InputOutput, visual,
				scrollbar_mask, &scrollbar_attr);

    if (add_window(sbi->win_id, SCROLLBARWINDOW, parent, (void *) sbi,
		   translate(helpname[SCROLLBARHELP])))
	return (void *) sbi;

    XDestroyWindow(display, sbi->win_id);
    free(sbi);
    return NULL;
}

static void scrollbar_draw(void *data)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    int postri;
    
    postri = sbi->sb_size - SCROLLBARSIZED -1;
    
    if (sbi->vertical) {
	XDrawRectangle(display, sbi->win_id, get_GC(Normal,0,0),
		       0, 0, SCROLLBARSIZE, sbi->sb_size-1);
	/*draw_triangle(sbi->win_id, 0, 0, UPTRIANGLE);
	  draw_triangle(sbi->win_id, 0, postri, DOWNTRIANGLE); */
    } else {
	XDrawRectangle(display, sbi->win_id, get_GC(Normal,0,0),
		       0, 0, sbi->sb_size-1, SCROLLBARSIZE);
	/* draw_triangle(sbi->win_id, 0, 0, LEFTTRIANGLE);
	   draw_triangle(sbi->win_id, postri, 0, RIGHTTRIANGLE); */
    }
    draw_rectangle(sbi->win_id, sbi->startpix, sbi->sizepix,
		   sbi->vertical, Normal);
}


static void position_changed(SCROLLBARINFO *sbi, int pos)
{
    int max=sbi->nr_lines;
    int nr=pos/sbi->line_size;
    Bool changed = MP_False;

    if (sbi->shrt) {
	max = max - sbi->sb_size/sbi->line_size + 1;
	if (max<=0) max=1;
    }
    if (!nr) nr=1;

    switch (mouse_button) {
    case Button1:
	if (!sbi->first_line) break;
	if (sbi->first_line <= nr)
	    sbi->first_line=0;
	else
	    sbi->first_line -= nr;
	changed = MP_True;
	break;
    case Button2:
	{
	    int i;

	    i = pos_to_line(pos, sbi->sb_size, sbi->nr_lines);
	    if (i!=sbi->first_line) {
		if (i<max)
		    sbi->first_line = i;
		else
		    sbi->first_line = max-1;
		changed = MP_True;
	    }
	}
	break;
    case Button3:
	if (sbi->first_line==max-1) break;
	if (sbi->first_line >= max-1-nr)
	    sbi->first_line=max-1;
	else
	    sbi->first_line += nr;
	changed = MP_True;
	break;
    }
    if (changed) {
	redraw_rectangle(sbi);
	(*(sbi->func))(sbi->data, sbi->vertical);
    }
}

static void scrollbar_press(void *data, XButtonEvent *event)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    int pos;
    XSetWindowAttributes attr;

    if (sbi->vertical)
	pos = event->y;
    else
	pos = event->x;

    if (pos<SCROLLBARSIZED)
	mouse_button = Button1;
    else
	if (pos> (sbi->sb_size - SCROLLBARSIZED -1))
	    mouse_button = Button3;
	else
	    mouse_button = remap_button(sbi->vertical, mouse_button);
    switch (mouse_button) {
    case Button1:
	attr.cursor = upleftcursor(sbi->vertical );
	break;
    case Button2:
	attr.cursor = middlecursor(sbi->vertical );
	break;
    case Button3:
	attr.cursor = downrightcursor(sbi->vertical );
	break;
    }
    XChangeWindowAttributes(display, sbi->win_id, CWCursor, &attr);
    if (mouse_button==Button2)
	get_motion_hints(sbi->win_id, -1);
    position_changed(sbi, pos);
}

static void scrollbar_release(void *data, XButtonEvent *event __attribute__((unused)))
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    XSetWindowAttributes attr;

    attr.cursor = normalcursor(sbi->vertical);
    XChangeWindowAttributes(display, sbi->win_id, CWCursor, &attr);
    stop_motion_hints();
}

static void scrollbar_motion(void *data, int x_pos, int y_pos)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    int pos;
    
    if (sbi->vertical)
	pos = y_pos;
    else
	pos = x_pos;
    position_changed(sbi, pos);
}

void scrollbar_move(void *data, int xpos, int ypos)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;

    XMoveWindow(display, sbi->win_id, xpos, ypos);
}

void scrollbar_resize(void *data, unsigned int newsize)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    
    minimum_size(newsize );
    sbi->sb_size = newsize;
    sbi->startpix = start_pixel(sbi);
    sbi->sizepix = end_pixel(sbi) - sbi->startpix;
    if (sbi->vertical)
	XResizeWindow(display, sbi->win_id, SCROLLBARSIZE+1, newsize);
    else
	XResizeWindow(display, sbi->win_id, newsize, SCROLLBARSIZE+1);
    scrollbar_draw(data);
}

FUNCTIONS scrollbarfuncs = {
    NULL, scrollbar_draw, NULL, scrollbar_press, scrollbar_release,
    scrollbar_motion };

void scrollbar_init(void)
{
    int i;
    
    scrollbar_mask =
        (CWBackPixel | CWBorderPixel | CWCursor | CWEventMask | CWColormap);
    
    scrollbar_attr.background_pixel = white_pixel;
    scrollbar_attr.border_pixel = black_pixel;
    scrollbar_attr.colormap = colormap;
    scrollbar_attr.event_mask = (ExposureMask | ButtonPressMask
				 | ButtonMotionMask | PointerMotionHintMask
				 | ButtonReleaseMask | StructureNotifyMask
				 | VisibilityChangeMask);
    for (i=0; i<n_cursors; i++)
	cursors[i] = XCreateFontCursor(display,cursors[i]);
}

void scrollbar_set(void *data, int f_line, int total_lines)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    
    sbi->nr_lines = total_lines;
    sbi->first_line = f_line;
    redraw_rectangle(sbi);
}

int scrollbar_line(void *data, int delta)
{
    SCROLLBARINFO *sbi  = (SCROLLBARINFO *) data;
    
    if (!delta)
	return sbi->first_line;
    sbi->first_line+=delta;
    if (sbi->first_line<0) sbi->first_line=0;
    if (sbi->first_line>=sbi->nr_lines) sbi->first_line = sbi->nr_lines-1;
    redraw_rectangle(sbi);
    return sbi->first_line;
}

/* int scrollbar_nr_lines(void *data, int delta)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    
    if (!delta)
	return sbi->nr_lines;
    sbi->nr_lines+=delta;
    if (sbi->nr_lines<=0)
	sbi->nr_lines = sbi->first_line = 0;
    else if (sbi->first_line>=sbi->nr_lines)
	sbi->first_line = sbi->nr_lines-1;
    redraw_rectangle(sbi);
    return sbi->nr_lines;
}

int scrollbar_goto(void *data, int pos)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    
    if (pos>=0 && pos<sbi->nr_lines && pos!=sbi->first_line) {
	sbi->first_line = pos;
	redraw_rectangle(sbi);
	return 1;
    }
    return 0;
}
*/

void scrollbar_linesize(void *data, int newsize)
{
    SCROLLBARINFO *sbi = (SCROLLBARINFO *) data;
    
    sbi->line_size = newsize;
    redraw_rectangle(sbi);
}

