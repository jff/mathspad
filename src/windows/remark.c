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
**  File  : remark.c
**  Datum : 21-6-92
**  Doel  : Het zetten van een opmerking die niet gemist mag worden
**          en mogelijk het vragen van een string.
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
/* #include "keymap.h" */
#include "getstring.h"
#include "scrollbar.h"
#include "output.h"
#include "remark.h"
#include "intstack.h"
#include "helpfile.h"

#define MAX_BUTTON      5
#define MARGIN         10

int kind_of_remark = NO_REMARK;
Bool select_line = MP_False;
int selected_line = -1;
Bool remark_no_button = MP_False;

static void *string_data = NULL;
static Window remark_win = 0, main_win = 0;
static Char *text = NULL;
static int nr_lines = 0;
static Char **str_text = NULL;
static void *scrollbar = NULL;
static int oldxpos=0, oldypos=0;
static int draw_pos = 0, nr_visible = 0;
static int black_width = 0;
static void *return_info;
static void (*return_func)(void*, int);
static INTSTACK *heights=NULL;

static unsigned long remark_mask;
static XSetWindowAttributes remark_attr;
static XTextProperty wname;
static char *win_name="Remark";

static void redraw_remark_lines(void)
{
    int i=0,tls = line_space, j;
    Char *temptxt = text;

    if (!remark_is_open || !text) return;
    line_space = 3;
    set_output_window(&remark_win);
    set_margin(draw_pos);
    set_x_y(0,MARGIN);

    if (scrollbar)
	j = scrollbar_line(scrollbar,0);
    else
	j=0;
    while (i<j) {
	while (*temptxt && !IsNewline(*temptxt)) temptxt++;
	i++;
	if (!*temptxt)
	    i=j;
	else
	    temptxt++;
    }
    if (i==selected_line) set_text_mode(Reverse);
    while (*temptxt && i<j+nr_visible) {
	if (IsNewline(*temptxt)) {
	    if (where_x() && where_x()<black_width)
		thinspace(black_width-where_x());
	    if (i==selected_line) set_text_mode(Normal);
	    out_char(*temptxt);
	    i++;
	    if (i==selected_line) set_text_mode(Reverse);
	} else
	    out_char(*temptxt);
	temptxt++;
    }
    if (where_x() && where_x()<black_width) thinspace(black_width-where_x());
    out_char(Newline);
    clear_to_end_of_page();
    unset_output_window();
    line_space = tls;
}

static void remark_draw(void *data __attribute__((unused)))
{
    push_fontgroup(POPUPFONT);
    redraw_remark_lines();
    pop_fontgroup();
}

static void remark_bad_end(void *data __attribute__((unused)))
{
    if (str_text) {
	free(*str_text);
	str_text = NULL;
	string_destroy(string_data);
    }
    free_int(heights);
    heights=NULL;
    free(text);
    text = NULL;
    destroy_window(remark_win);
    remark_is_open = MP_False;
    kind_of_remark = NO_REMARK;
}

static void detect_line(int xpos, int ypos)
{
    if (select_line) {
	if (xpos <0 || ypos<0)
	    selected_line = -1;
	else {
	    int i,j,h;
	    INTSTACK *tl=heights;
	    if (scrollbar)
		j = scrollbar_line(scrollbar,0);
	    else
		j=0;
	    i=j;
	    while (i) {
		tl = tail_stack(tl);
		i--;
	    }
	    i=j; h=MARGIN+head_int(tl);
	    tl = tail_stack(tl);
	    while (ypos>h && i<j+nr_visible) {
		i++;
		h+= head_int(tl);
		tl = tail_stack(tl);
	    }
	    if (i>=j+nr_visible || i>=nr_lines)
		selected_line = -1;
	    else
		selected_line = i;
	}
    }
}

static void remark_handle_button(void *data __attribute__((unused)), int bnr)
{
    if (str_text) {
	free(*str_text);
	*str_text = string_text(string_data);
	string_destroy(string_data);
    }
    free_int(heights);
    heights = NULL;
    free(text);
    text = NULL;
    XDestroyWindow(display, remark_win);
    destroy_window(remark_win);
    scrollbar = NULL;
    remark_is_open = MP_False;
    if (return_func && exist_window(main_win))
	(*return_func)(return_info, bnr);
}

static void return_pressed(void)
{
    remark_handle_button(NULL, 0);
}

static void remark_motion(void *data __attribute__((unused)), int x, int y)
{
    int oldline = selected_line;
    push_fontgroup(POPUPFONT);
    detect_line(x, y);
    if (oldline != selected_line) redraw_remark_lines();
    pop_fontgroup();
}

static void remark_press(void *data, XButtonEvent *event)
{
    remark_motion(data, event->x, event->y);
    get_motion_hints(remark_win, -1);
}

static void remark_release(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
    stop_motion_hints();
    if (select_line) {
	select_line = MP_False;
	remark_handle_button(NULL, selected_line);
	selected_line = -1;
    }
}

static void remark_map(void)
{
    if (remark_is_open && remark_iconized) {
	XMapWindow(display, remark_win);
	if (str_text)
	    string_map(string_data);
	remark_iconized = MP_False;
    }
}

static void remark_scrollto(void *data __attribute__((unused)), int kind __attribute__((unused)))
{
    if (scrollbar) {
	push_fontgroup(POPUPFONT);
	redraw_remark_lines();
	pop_fontgroup();
    }
}

FUNCTIONS remarkfuncs = {
    remark_bad_end, remark_draw, NULL, remark_press, remark_release,
    remark_motion };


#include "language.h"

static int call_noarg(int (*func)(), void **argl __attribute__((unused)))
{
  return (*func)();
}

static int call_open_prompt(int (*func)(), void **argl)
{
  return (*func)(*((Char**)argl[0]), *((Char**)argl[1]),
		 *((Char**)argl[2]), *((void**)argl[3]),
		 *((Char**)argl[4]), *((void**)argl[5]));
}

static Type typelist[8] = {
  StringType, StringType, StringType, LazyEvalType,
  StringType, LazyEvalType };


static Char *prompt_button[4];
static Char *prompt_text[4];
static void *prompt_lazyfunction[3];
static int prompt_lastbutton=0;
static Char *prompt_answer=NULL;
static int prompt_int=0;    /* integer value of answer */

static void prompt_handle(void *data  __attribute__((unused)), int bnr)
{
  if (bnr>=0 && bnr<prompt_lastbutton) {
    prompt_int = Ustrtol(prompt_answer, 0,0);
    if (prompt_lazyfunction[bnr]) {
      calculate_lazy_expression(prompt_lazyfunction[bnr]);
    }
  }
}

static void open_prompt(Char *comment, Char *defaultans,
			Char *button1, void *function1,
			Char *button2, void *function2)
{
  if (!can_open_remark) return;
  /* fill information needed for the prompt function */
  prompt_lastbutton=0;
  if (button1 && button1[0]) {
    prompt_button[prompt_lastbutton]=button1;
    prompt_lazyfunction[prompt_lastbutton]=function1;
    prompt_lastbutton++;
  }
  if (button2 && button2[0]) {
    prompt_button[prompt_lastbutton]=button2;
    prompt_lazyfunction[prompt_lastbutton]=function2;
    prompt_lastbutton++;
  }
  prompt_button[prompt_lastbutton]=translate("Cancel");
  prompt_lazyfunction[prompt_lastbutton]=NULL;
  prompt_lastbutton++;
  prompt_button[prompt_lastbutton]=0;
  prompt_lazyfunction[prompt_lastbutton]=NULL;
  if (!prompt_answer) {
    prompt_answer= malloc(sizeof(Char)*1024);
  }
  Ustrcpy(prompt_answer,defaultans);
  /* open a remark window where the string can be filled in */
  remark_make(wm_hints.window_group, NULL, prompt_handle, REMARK_LASTPOS,
	      comment, prompt_button, &prompt_answer, 1024, NULL);
}

void remark_init(void)
{
  void *pt;
    remark_mask = (CWBackPixel | CWBorderPixel | CWEventMask |
		   CWSaveUnder | CWOverrideRedirect | CWColormap);
    remark_attr.background_pixel = white_pixel;
    remark_attr.border_pixel = black_pixel;
    remark_attr.colormap = colormap;
    remark_attr.event_mask = (ExposureMask | ButtonPressMask |
			      ButtonReleaseMask | KeyPressMask |
			      ButtonMotionMask | PointerMotionHintMask |
			      FocusChangeMask |
			      StructureNotifyMask | VisibilityChangeMask);
    remark_attr.override_redirect = MP_False;
    remark_attr.save_under = MP_True;
    pt= define_prototype(0,0,0,call_noarg);
    define_function("remark_default_action",
		    "Perform the default action for a remark window.",
		    pt,return_pressed);
    pt = define_prototype(typelist, 6, 0, call_open_prompt);
    define_function("open_prompt",
		    "Open a prompt to ask for a string.",
		    pt,open_prompt);
    define_program_variable(IntType, "prompt_int", &prompt_int);
    define_program_variable(StringType, "prompt_answer", &prompt_answer);
    XStringListToTextProperty(&win_name, 1, &wname);
}

static Char *standardbutton[2]= {0,0};

int remark_make(Window mwin, void* info, void (*func)(void*,int), int where,
                Char *remarktext, Char **buttontext, Char **stringtext,
		int maxlength, Char *helpfile)
{
    int i, lh, tempwidth, ystring=0, ybutton,
	xpos, ypos, pxpos, pypos, buttons;
    unsigned int width, height;
    Char *tt;
    INTSTACK *ts=NULL;

    if (!can_open_remark) { /* free(remarktext); */return 0; }
    if (!mwin) mwin = root_window;
    return_info = info;
    return_func = func;
    main_win = mwin;
    text = Ustrdup(remarktext);
    nr_lines = 0;
    while (*remarktext) {
	nr_lines += IsNewline(*remarktext);
	remarktext++;
    }
    if (*text) nr_lines++;
    if (buttontext) {
	buttons=0;
	while (buttontext[buttons]) buttons++;
    } else
	buttons=0;
    if (!buttons) {
	if (kind_of_remark == SHORT_REMARK || remark_no_button) {
	    buttontext = NULL;
	    buttons = 0;
	} else {
	    buttontext = standardbutton;
	    standardbutton[0]=translate(" OK ");
	    buttons = 1;
	    kind_of_remark = LONG_REMARK;
	}
    } else
	if (buttons > MAX_BUTTON)
	    buttons = MAX_BUTTON;
    str_text = stringtext;
    width = 0;
    height = MARGIN;
    tt = text;
    lh = line_space;
    line_space=3;
    push_fontgroup(POPUPFONT);
    set_output_window(test_window());
    set_drawstyle(INVISIBLE);
    for (i=0; i<nr_lines; i++) {
	set_x_y(0,0);
	while (*tt && !IsNewline(*tt)) out_char(*tt++);
	if (where_x()>(int)width) width = where_x();
	out_char(Newline);
	push_int(&ts, where_y());
	height+=where_y();
	tt++;
    }
    line_space = lh;
    lh = line_height()-line_space+3;
    unset_output_window();
    black_width = width;
    height += MARGIN;
    if (str_text) {
	if ((tempwidth = string_window_width(maxlength)) > width)
	    width = tempwidth;
	ystring = height;
	height += string_height() + MARGIN;
    }
    tempwidth = -MARGIN;
    ybutton = height;
    if (buttons>0)
	height+= button_height+MARGIN;
    for (i=0; i<buttons; i++) {
	tempwidth += button_width(buttontext[i]) + MARGIN;
    }
    if (tempwidth > (int)width)  width = tempwidth;
    width += 2*MARGIN;
    draw_pos = MARGIN;
    if (height>(display_height<<2)/5) {
	height -= nr_lines * lh;
	nr_visible = ((display_height<<2)/5-height)/lh;
	height += nr_visible * lh;
	if (nr_visible != nr_lines) {
	    width += SCROLLBARSIZE+2*INTERSPACE;
	    draw_pos = MARGIN+SCROLLBARSIZE+2*INTERSPACE;
	    i = (nr_lines-nr_visible) * lh;
	    ystring -= i;
	    ybutton -= i;
	}
    } else
	nr_visible = nr_lines;
    xpos = display_width - width;
    ypos = display_height - height;
    if (where & (REMARK_POINTER|REMARK_VCENTRE|REMARK_BUTTON)) {
	Window rt, cld;
	int tx, ty;
	unsigned int kb;

	XQueryPointer(display, root_window, &rt, &cld,
		      &pxpos, &pypos, &tx, &ty, &kb);
    }
    switch (where & REMARK_POSITION) {
    case REMARK_VCENTRE:
	pxpos = oldxpos;
	if (pypos-(int)height/2<0) pypos=0; else pypos=pypos-(int)height/2;
	break;
    case REMARK_BUTTON:
	pxpos = pxpos - (width-tempwidth)/2-button_width(buttontext[0])/2;
	pypos = pypos - ybutton-button_height/2;
	if (pxpos<0) pxpos = 0;
	if (pypos<0) pypos = 0;
	break;
    case REMARK_LASTPOS:
	pxpos = oldxpos;
	pypos = oldypos;
	break;
    case REMARK_CENTRE:
	pxpos = xpos/2;
	pypos = ypos/2;
	break;
    default:
	break;
    }
    if (pxpos<xpos) xpos = pxpos;
    if (pypos<ypos) ypos = pypos;
    oldxpos=xpos;
    oldypos=ypos;
    remark_win = XCreateWindow(display, root_window,
			       xpos, ypos, width, height, 2,
			       CopyFromParent, InputOutput,
			       visual,
			       remark_mask, &remark_attr);
    i=0;
    if (!helpfile) helpfile=translate(helpname[POPUPHELP]);
    if (add_window(remark_win, REMARKWINDOW, mwin, NULL, helpfile)) {
	if (!stringtext ||
	    (string_data = string_make(remark_win, *stringtext, maxlength,
				       width-2*MARGIN, NULL, MARGIN,
				       ystring, MP_False)))
	    i = 0;
	else
	    i = -1;
	if (!i && nr_visible != nr_lines) {
	    scrollbar = scrollbar_make(VERTICAL_SHORT, remark_win, MARGIN,
				       MARGIN, lh*nr_visible, lh,
				       remark_scrollto, NULL);
	    if (!scrollbar)
		i=-1;
	    else
		scrollbar_set(scrollbar, 0, nr_lines);
	}
	if (!i) {
	    tempwidth = (width - tempwidth)/2;
	    while (i<buttons &&
		   button_make(i, remark_win, buttontext[i], &tempwidth,
			       ybutton,
			       (stringtext && !i ? 2 : 1), NULL, NULL,
			       NULL, NULL, NULL,
			       remark_handle_button, NULL, NULL)) {
		i++;
		tempwidth += MARGIN;
	    }
	}
    }
    pop_fontgroup();
    if (i != buttons) {
	XBell(display, 0);
	XDestroyWindow(display, remark_win);
	kind_of_remark = NO_REMARK;
	free(text);
	text = NULL;
	free_int(ts);
	return 0;
    } else {
        XSizeHints size_hints;
        XClassHint classhints;
        XWMHints wmhints;
        Atom rsatom[2],olatom,atatom;
        XSetWindowAttributes override;

	if (stringtext) {
	    string_relation(string_data, string_data, string_data);
	    string_get_input(string_data);
	    string_map(string_data);
	}
	wmhints=wm_hints;
	wmhints.flags = StateHint | InputHint | WindowGroupHint;
	wmhints.input = MP_True;
	size_hints.flags = USPosition | USSize | PMinSize | PMaxSize;
	size_hints.min_height=size_hints.max_height=height;
	size_hints.min_width=size_hints.max_width=width;
	classhints.res_name = "popup";
	classhints.res_class = "MathEdit";
	rsatom[0] = XInternAtom(display, "_OL_DECOR_RESIZE", MP_True);
	rsatom[1] = XInternAtom(display, "_OL_DECOR_HEADER", MP_True);
	olatom = XInternAtom(display, "_OL_DECOR_DEL", MP_True);
	atatom = XInternAtom(display, "ATOM", MP_True);
	if (rsatom[0] && rsatom[1] && olatom) {
	  XChangeProperty(display, remark_win, olatom, atatom, 32,
			  PropModeReplace, (void*) rsatom, 2);
	}
	XSetWMProperties(display, remark_win, &wname, NULL, NULL, 0,
			 &size_hints, &wmhints, &classhints);
	set_protocols(remark_win);
	XSetTransientForHint(display, remark_win, mwin);
	
	XMapSubwindows(display, remark_win);
	XMapWindow(display, remark_win);
	if (where & REMARK_MOTION && select_line)
	    when_motion_window = remark_win;
	while (ts) {
	    i = pop_int(&ts);
	    push_int(&heights,i);
	}
	remark_is_open = MP_True;
	return 1;
    }
}

Bool remark_removable(void)
{
    if (kind_of_remark == SHORT_REMARK) {
	remark_bad_end(NULL);
	XDestroyWindow(display, remark_win);
	destroy_window(remark_win);
	return MP_True;
    } else
	return MP_False;
}

void remark_raise(void)
{
    remark_map();
    if (remark_is_open)
	XRaiseWindow(display, remark_win);
}

void remark_unmap(void)
{
    if (remark_is_open)
	if (!remark_iconized) {
	    XUnmapWindow(display, remark_win);
	    if (str_text)
		string_unmap(string_data);
	    remark_iconized = MP_True;
	}
}

