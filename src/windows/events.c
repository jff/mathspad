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
**  File: events.c
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"

#include "unitype.h"
#include "uniconv.h"

/* #include "keymap.h" */
#include "keyboard.h"
#include "button.h"
#include "message.h"
#include "scrollbar.h"
#include "getstring.h"
#include "remark.h"
#include "symbol.h"
#include "edit.h"
#include "buffer.h"
#include "default.h"
#include "notatype.h"
#include "notadef.h"
#include "notation.h"
#include "output.h"
#include "find.h"
#include "menu.h"
#include "events.h"
#include "fileselc.h"
#include "popup.h"
#include "checkbox.h"

#define SelPasteKey 'P'
#define SelPasteMode ModeAdd(ModeAdd(ModeAdd(ModeAdd(0,ModShift),ModMeta),ModAlt),ModHyper)

void init_all(int argc, char **argv)
{
    unitype_init();
    
    UConvLoadDatabase("UniConvert");
    system_init(argc, argv);
    server_init();
    font_load_config("helvetica");
    /* font_set_attribute(2,4); */
    keyboard_init();
    variable_init();
    menu_keyboard();
    notadef_keyboard();
    string_keyboard();
    fileselc_init();
    remark_init();

    load_keypath();

    font_set_attributes(fontattributes[EDITFONT]);
    push_attributes(EDITFONT);
    button_init();
    scrollbar_init();
    string_init();
    checkbox_init();

    popup_init();
    output_init();
    edit_init();
    symbol_init();
    buffer_init();
    default_init();
    notatype_init();
    notation_init();
    notadef_init();
    find_init();
    menu_init();
    if (project_name)
	load_project(project_name);
    else
	menu_open(0,0,0,0,0,0,NULL);
    iconic = MP_False;
    create_needed_directories(0, MP_True);
    wm_hints.initial_state = NormalState;
    /* load_keypath(); */
}

static Window iwin = 0, parent=0, parentspar=0, first_click=0;
static void *data=NULL, *datapar=NULL;
static WINDOWTYPE wt=NOWINDOW, pwt=NOWINDOW;
static XEvent report;
static int mouse_old_x=0, mouse_old_y=0;


static void set_info(Window win)
{
    if (iwin != win || !exist_window(parent)) {
	iwin = win;
	wt = get_window_type(iwin, &parent, &data);
	if (wt) {
	    pwt = get_window_type(parent, &parentspar, &datapar);
	    if (!data) data = datapar;
	    if (!data) data = (void*) &(iwin);
	}
    } else if (!exist_window(iwin)) {
	iwin=0;
	wt=NOWINDOW;
	data=NULL;
	parent=0;
	datapar=NULL;
    }
}

static void do_motion_notify(int x, int y)
{
    Bool get_pos = (x==-1 && y==-1);
    if (when_motion_window && when_motion_window != motion_window) {
	motion_window=when_motion_window;
	get_pos = MP_True;
    }
    when_motion_window = 0;
    if (get_pos)
	if (!motion_get_pos(&x,&y)) return;
    if ((mouse_old_x == x && mouse_old_y == y)) return;
    set_info(motion_window);
    mouse_old_x=x;
    mouse_old_y=y;
    is_click = MP_False;
    if (eventfunc[wt]->motion)
	if (!remark_is_open || wt==REMARKWINDOW || pwt==REMARKWINDOW)
	    (*(eventfunc[wt]->motion))(data, x, y);
}

static void do_key_press(XKeyEvent *event)
{
    KeyNum keynum;
    KeyMode keymode;
    char buffer[5000];
    
    if (remark_is_open) (void) remark_removable();
    if (!convert_event((void*)event, &keynum, &keymode)) return;
    print_key(buffer,4999,keynum,keymode);
    /* fprintf(stderr, "%s\n",buffer); */
    if (keynum==XK_Help && !keymode) {
	Char *c;
	/* the help key has a special meaning (on a sun)
	** pressing help       -> window = pointer-window
	**                        subwindow = 0
	** pressing other key  -> window = main-window (input window)
	**                        subwindow = pointer-window
	** in a emulator (exceed) it is completely different.
	** to make sure it works, find the window the pointer is on.
	*/
	Window rW=None, cW=root_window, dW;
	int rx, ry, x1, y1,tl=0;
	unsigned int mask;
	do {
	    dW=cW;
	    if (!XQueryPointer(display, dW, &rW, &cW, &rx, &ry,
			       &x1, &y1, &mask)) {
		tl=20;
	    }
	    tl++;
	} while (cW && tl<20);
	c = window_help(dW);
	if (!c)
	    /* use event information */
	    if (event->subwindow)
		c = window_help(event->subwindow);
	    else
		c = window_help(event->window);
	if (c) open_helpfile(c, 0);
    } else
	handle_key(keynum,keymode);
    call_auto_save(event->time);
}

static Bool all_iconized(void)
{
    return ((!menu_is_open || menu_iconized) &&
	    (!notation_is_open || notation_iconized) &&
	    (!notadef_is_open || notadef_iconized) &&
	    (!buffer_is_open || buffer_iconized) &&
	    (!find_is_open || find_iconized) &&
	    (!default_is_open || default_iconized) &&
	    (!symbol_is_open || symbol_iconized) &&
	    (!edit_is_open || edit_iconized)) ;
}

void get_events(void)
{
    Atom matom;
    int x,y;
    unsigned long auto_save = 0; 
    KeyNum keynum;
    KeyMode keymode;
    char buffer[5000];

    while (1) {
	(void) XEventsQueued(display, QueuedAfterFlush);
	XNextEvent(display, &report);
	set_info(report.xany.window);
	if (convert_event((void*)(&report), &keynum, &keymode)) {
	  /* print_key(buffer, 4999,keynum,keymode);
	  ** fprintf(stderr, "Key: %s\n", buffer);
	  */
	}
	can_auto_save = MP_False;
	switch (report.type) {

	case GraphicsExpose:
	case Expose:
	    if (!report.xexpose.count && eventfunc[wt]->draw)
	        (*(eventfunc[wt]->draw))(data);
	    break;

	case MotionNotify:
	    while (XCheckMaskEvent(display, ButtonMotionMask, &report));
	    if (when_motion_window && when_motion_window != motion_window) {
		motion_window=when_motion_window;
	    }
	    when_motion_window = 0;
	    if (motion_get_pos(&x,&y) && is_drag(report.xmotion.time))
		do_motion_notify(x,y);
	    break;

	case ConfigureNotify:
	    if (eventfunc[wt]->configure)
		(*(eventfunc[wt]->configure))(data, &report.xconfigure);
	    break;

	case EnterNotify:
	    auto_save = last_time = report.xcrossing.time;
	    if (eventfunc[wt]->enter) (*(eventfunc[wt]->enter))(data);
	    break;

	case LeaveNotify:
	    auto_save = last_time = report.xcrossing.time;
	    if (eventfunc[wt]->leave) (*(eventfunc[wt]->leave))(data);
	    break;

	case ButtonPress:
	    if (mouse_press(report.xbutton.state, report.xbutton.button)) {
		press_time = report.xbutton.time;
		double_click = (first_click==report.xany.window) &&
		               (press_time-release_time)<(unsigned int)wait_time;
		mouse_old_x = report.xbutton.x;
		mouse_old_y = report.xbutton.y;
		push_keymap(0);
		if (double_click && eventfunc[wt]->dbl_click) {
		  if (!remark_is_open || remark_removable() ||
		      wt==REMARKWINDOW || pwt==REMARKWINDOW)
		    (*(eventfunc[wt]->dbl_click))(data);
	    	} else {
		  double_click = MP_False;
		  first_click = report.xany.window;
		  if (eventfunc[wt]->press) {
		    if (!remark_is_open || remark_removable() ||
			wt==REMARKWINDOW || pwt==REMARKWINDOW)
		      (*(eventfunc[wt]->press))(data, &report.xbutton);
		    else message(MP_MESSAGE, translate("Please handle the popup first. "
				 "(It should be visible by now)"));
		  }
		}
	    } else
		do_motion_notify(-1,-1);
	    break;

	case ButtonRelease:
	    if (mouse_release(report.xbutton.state, report.xbutton.button)) {
		auto_save = release_time = last_time = report.xbutton.time;
		if (motion_window) set_info(motion_window);
		pop_keymap();
		if (eventfunc[wt]->release) {
		    if (!remark_is_open || remark_removable() ||
			wt==REMARKWINDOW || pwt==REMARKWINDOW)
			(*(eventfunc[wt]->release))(data, &report.xbutton);
		    else
			remark_raise();
		}
		when_motion_window=0;
	    } else
		do_motion_notify(-1,-1);
	    break;
	case PropertyNotify:
	    if (eventfunc[wt]->property_change)
		(*eventfunc[wt]->property_change)(data, &report.xproperty);
	    break;

	case DestroyNotify:
	    destroy_window(report.xany.window);
	    break;

	case KeyPress:
	    auto_save = last_time = report.xkey.time;
	    do_key_press(&report.xkey);
	    break;

	case FocusIn:
	  if (window_keyboard(report.xany.window)) {
	    set_keyboard_stack(window_keyboard(report.xany.window));
	  }
	  break;

	case FocusOut:
	  break;

	case MappingNotify:
	    if (report.xmapping.request == MappingKeyboard)
		XRefreshKeyboardMapping(&report.xmapping);
	    break;

	case SelectionRequest:
	    send_selection(&report.xselectionrequest);
	    break;

	case SelectionNotify:
	    if (report.xselection.property!= None) {
		long start=0;
		unsigned long nit, byte_af=0;
		Atom act_type;
		int act_form;
		int l;
		Bool freeit=MP_False;
		unsigned char *wmdata;

		XGetWindowProperty(display, report.xselection.requestor,
				   report.xselection.property, 0, 3000, MP_False,
				   AnyPropertyType, &act_type, &act_form,
				   &nit, &byte_af, &wmdata);
		auto_save = last_time = report.xselection.time;
		if (!wmdata || !nit) {
		    freeit = MP_True;
		    wmselection = XFetchBytes(display, &l);
		    nit = l;
		} else {
		    while (byte_af) {
			wmselection = (char*) wmdata;
			handle_key(SelPasteKey,SelPasteMode);
			start+=3000;
			XGetWindowProperty(display,report.xselection.requestor,
					   report.xselection.property, start,
					   3000, MP_False, AnyPropertyType,
					   &act_type, &act_form, &nit,
					   &byte_af, &wmdata);
		    }
		    wmselection = (char*) wmdata;
		}
		if (nit && wmselection)
		    handle_key(SelPasteKey,SelPasteMode);
		if (freeit && wmselection) XFree(wmselection);
		wmselection = NULL;
		XDeleteProperty(display, report.xselection.requestor,
				report.xselection.property);
	    } else {
		int l;
		wmselection = XFetchBytes(display, &l);
		if (l && wmselection)
		    handle_key(SelPasteKey, SelPasteMode);
		XFree(wmselection);
		wmselection = NULL;
	    }
	    break;
	case ClientMessage:
	    matom =  (Atom) report.xclient.data.l[0];
	    if (matom == WM_SAVE_YOURSELF) {
		save_project(translate(".mpproject"));
		menu_set_command();
	    } else if (matom == WM_DELETE_WINDOW) {
		XDestroyWindow(display, report.xany.window);
		if (eventfunc[wt]->bad_end) (*(eventfunc[wt]->bad_end))(data);
	    }
	    break;

	case VisibilityNotify:
	    change_visibility(report.xvisibility.window, report.xvisibility.state);
	    break;

	case UnmapNotify:
	    change_mapped(report.xany.window, MP_False);
	    if (eventfunc[wt]->iconize) (*(eventfunc[wt]->iconize))(data);
	    if (remark_is_open && all_iconized()) remark_unmap();
	    break;

	case MapNotify:
	    change_mapped(report.xany.window, MP_True);
	    if (eventfunc[wt]->deiconize) {
		(*(eventfunc[wt]->deiconize))(data);
		if (remark_is_open) remark_raise();
	    }
	    break;

	default:
	    break;
	}
	can_auto_save = MP_True;
        call_auto_save(auto_save);
    }
}
