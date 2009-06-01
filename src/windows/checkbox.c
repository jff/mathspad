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
#include "checkbox.h"
#include "message.h"

typedef struct CHECKBOXINFO CHECKBOXINFO;

struct CHECKBOXINFO {
    Window win_id;
    Bool check;
    CHECKBOXINFO *next,*prev;
} ;

static unsigned long checkbox_mask;
static XSetWindowAttributes checkbox_attr;

static void checkbox_draw(void *data)
{
    CHECKBOXINFO *cbinfo = (CHECKBOXINFO *) data;
    if (cbinfo->check) {
        XDrawLine(display, cbinfo->win_id, get_GC(Normal, 0,0),
		  0, 0, CHECKBOXSIZE-1, CHECKBOXSIZE-1);
	XDrawLine(display, cbinfo->win_id, get_GC(Normal, 0,0),
		  0, CHECKBOXSIZE-1, CHECKBOXSIZE-1, 0);
    } else {
	XClearWindow(display, cbinfo->win_id);
    }
}

/*
static void checkbox_press(void *data, XButtonEvent *event)
{
    CHECKBOXINFO *cbinfo = (CHECKBOXINFO *) data;

    checkbox_draw(cdata);
}
*/

static void checkbox_release(void *data, XButtonEvent *event __attribute__((unused)))
{
    CHECKBOXINFO *cbinfo = (CHECKBOXINFO *) data;

    if (aig(cbinfo->check = (!cbinfo->check))) {
	CHECKBOXINFO *h= cbinfo->next;
	while (h != cbinfo) {
	    if (h->check) { h->check=MP_False; checkbox_draw((void*) h); }
	    h=h->next;
	}
    }
    checkbox_draw(data);
}

FUNCTIONS checkboxfuncs = {
    NULL, checkbox_draw, NULL, NULL, checkbox_release };


void checkbox_init(void)
{
    checkbox_mask =
	(CWBackPixel | CWBorderPixel | CWWinGravity |
	 CWEventMask | CWColormap );
    
    checkbox_attr.background_pixel = white_pixel;
    checkbox_attr.border_pixel = black_pixel;
    checkbox_attr.win_gravity = NorthWestGravity;
    checkbox_attr.colormap = colormap;
    checkbox_attr.event_mask = (ExposureMask | ButtonPressMask | 
			      ButtonReleaseMask | StructureNotifyMask |
			      VisibilityChangeMask);
}

void checkbox_set(void *data, Bool on)
{
    CHECKBOXINFO *cbinfo = (CHECKBOXINFO*) data;
    if (cbinfo->check != on) checkbox_release(data, NULL);
}

Bool checkbox_value(void *data)
{
    return ((CHECKBOXINFO*)data)->check;
}

void checkbox_connect(void *box, void *other_box)
{
    CHECKBOXINFO *cb1 = (CHECKBOXINFO*)box;
    CHECKBOXINFO *cb2 = (CHECKBOXINFO*)other_box;
    CHECKBOXINFO *h;

    cb1->next->prev = cb2;
    cb2->next->prev = cb1;
    h=cb1->next;
    cb1->next=cb2->next;
    cb2->next=h;
}

void *checkbox_make(Window parent, int xpos, int ypos, int value __attribute__((unused)))
{
    CHECKBOXINFO *cbinfo;

    if ( (cbinfo = (CHECKBOXINFO *) malloc( sizeof(CHECKBOXINFO) )) == NULL) {
	message(MP_ERROR, translate("Out of memory in checkbox."));
	return NULL;
    } else {
	cbinfo->check  = MP_False;
	cbinfo->prev = cbinfo->next = cbinfo;
	cbinfo->win_id = XCreateWindow(display, parent, xpos, ypos,
				       CHECKBOXSIZE,CHECKBOXSIZE, 1,
				       CopyFromParent, InputOutput,
				       visual,
				       checkbox_mask, &checkbox_attr);
	if (add_window(cbinfo->win_id, CHECKBOXWINDOW, parent,
		       (void *)cbinfo, NULL))
	    return ((void *) cbinfo);
	XDestroyWindow(display, cbinfo->win_id);
	free(cbinfo);
	return NULL;
    }
}
