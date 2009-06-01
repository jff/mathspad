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
**  File  : button.c
**  Datum : 29-3-92
**  Doel  : Het maken, tekenen en inspecteren van buttons/icons
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
#include "message.h"
#include "helpfile.h"
#include "output.h"

#define STARTCURSOR  XC_hand2       /* cursor in button            */
#define TEXTX        3              /* pixels tussen rand en tekst */
#define TEXTY        3
#define INSIDEWIDTH  1

typedef struct {
            Window win_id;
	    int number;
	    unsigned int but_width, but_height;
	    Bool pressed, inside;
	    Char *text;
	    int length;              /* lengte van text (char) */
	    int border;
	    void *data;
	    void (*funcpress[3])(void *,int);
	    void (*funcrelease[3])(void *,int);
	} BUTTONINFO;

unsigned int button_height;

static unsigned long button_mask;
static XSetWindowAttributes button_attr;

static void button_draw(void *data)
{
    BUTTONINFO *binfo = (BUTTONINFO *) data;
    GC tempgc2;
    int i,j;

    tempgc2 = get_GC(((binfo->pressed && binfo->inside) ? Normal
		      : Reverse), 0, 0);
    j=binfo->border + (binfo->inside && !binfo->pressed ? INSIDEWIDTH:0);
    XFillRectangle(display, binfo->win_id, tempgc2, j, j, 
		   binfo->but_width-j*2, binfo->but_height-j*2);
    push_fontgroup(EDITFONT);
    set_output_window( (void*)&binfo->win_id);
    if (binfo->pressed && binfo->inside) switch_reverse();
    set_x_y(0,TEXTY);
    thinspace(TEXTX);
    out_string(binfo->text);
    if (binfo->pressed && binfo->inside) switch_reverse();
    unset_output_window();
    pop_fontgroup();
    for (i=0; i<j; i++)
	XDrawRectangle(display, binfo->win_id, get_GC(Normal, 0, 0), i, i,
		       binfo->but_width-1-i*2, binfo->but_height-1-i*2);
}

static void button_press(void *data, XButtonEvent *event __attribute__((unused)))
{
    BUTTONINFO *binfo = (BUTTONINFO *) data;
    int i;

    if (!binfo->pressed) {
	binfo->pressed = MP_True;
	button_draw(data);
	get_motion_hints(binfo->win_id, -1);
	i = (mouse_button>3 ? 0 : mouse_button-1);
	if (binfo->funcpress[i]) {
	    binfo->pressed = MP_False;
	    button_draw(data);
	    (*(binfo->funcpress[i]))(binfo->data, binfo->number);
	}
    }
}

static void button_release(void *data, XButtonEvent *event __attribute__((unused)))
{
    BUTTONINFO *binfo = (BUTTONINFO *) data;
    int i;

    if (!binfo->pressed) return;
    binfo->pressed = MP_False;
    button_draw(data);
    stop_motion_hints();
    if (binfo->inside) {
	i = (mouse_button>3 ? 0: mouse_button-1);
	if (!binfo->funcrelease[i] && !binfo->funcpress[i]) i=0;	    
	if (binfo->funcrelease[i])
	    (*(binfo->funcrelease[i]))(binfo->data, binfo->number);
    }
}

static void button_leave(void *data)
{
    BUTTONINFO *binfo = (BUTTONINFO *) data;
    
    binfo->inside = MP_False;
    button_draw(data);
}

static void button_enter(void *data)
{ 
    BUTTONINFO *binfo = (BUTTONINFO *) data;
    
    binfo->inside = MP_True;
    button_draw(data);
}

unsigned int button_width(Char *txt)
{
    int i;
    push_fontgroup(EDITFONT);
    i = 2+TEXTX*2 + string_width(txt, -1);
    pop_fontgroup();
    return  i;
}

FUNCTIONS buttonfuncs = {
    NULL, button_draw, NULL, button_press, button_release, NULL, NULL,
    NULL, button_leave, button_enter };

void button_init(void)
{
    button_mask =
	(CWBackPixel | CWBorderPixel | CWCursor | CWWinGravity |
	 CWColormap | CWEventMask);
    
    button_attr.background_pixel = white_pixel;
    button_attr.border_pixel = black_pixel;
    button_attr.colormap = colormap;
    button_attr.win_gravity = NorthWestGravity;
    button_attr.event_mask = (ExposureMask | ButtonPressMask | 
			      ButtonReleaseMask | ButtonMotionMask |
			      StructureNotifyMask | PointerMotionHintMask |
                              EnterWindowMask | LeaveWindowMask |
			      VisibilityChangeMask);
    button_attr.cursor = XCreateFontCursor(display,STARTCURSOR);
    push_fontgroup(EDITFONT);
    button_height = TEXTY*2+font_height();
    pop_fontgroup();
}

void button_stick(int gravity)
{
    button_attr.win_gravity=gravity;
}

void button_move(void *data, int x, int y)
{
    BUTTONINFO *binfo = (BUTTONINFO*) data;

    XMoveWindow(display, binfo->win_id, x, y);
}

void *button_make(int bnr, Window parent, Char *txt,
		  int *x_offset, int y_offset, int border,
		  void *data, char *helpfile,
		  BTFUNC func1p, BTFUNC func2p, BTFUNC func3p,
		  BTFUNC func1r, BTFUNC func2r, BTFUNC func3r)
{
    BUTTONINFO *binfo;
    
    if ( (binfo = (BUTTONINFO *) malloc( sizeof(BUTTONINFO) )) == NULL) {
	message(MP_ERROR, translate("Out of memory in button."));
	return NULL; }
    else {
	binfo->but_width  = button_width(txt);
	binfo->but_height = button_height;
	binfo->text       = txt;
	binfo->length     = Ustrlen(txt);
	binfo->pressed    = MP_False;
	binfo->inside     = MP_False;
	binfo->number     = bnr;
	binfo->border     = border;
	binfo->funcpress[0] = func1p;
	binfo->funcpress[1] = func2p;
	binfo->funcpress[2] = func3p;
	binfo->funcrelease[0] = func1r;
	binfo->funcrelease[1] = func2r;
	binfo->funcrelease[2] = func3r;
	binfo->data       = data;
	binfo->win_id = XCreateWindow(display, parent, *x_offset, y_offset,
				      binfo->but_width, binfo->but_height, 0,
				      CopyFromParent, InputOutput,
				      visual,
				      button_mask, &button_attr);
	*x_offset += binfo->but_width;
	if (!helpfile) helpfile=helpname[BUTTONHELP];
	if (add_window(binfo->win_id, BUTTONWINDOW, parent,
		       (void *) binfo, translate(helpfile)))
	    return ((void *) binfo);
	XDestroyWindow(display, binfo->win_id);
	free(binfo);
	return NULL;
    }
}
