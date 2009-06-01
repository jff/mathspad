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
**  File  : getstring.c
**  Datum : 26-4-92
**  Doel  : De invoer van strings mogelijk maken
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
/*#include "keymap.h"*/
#include "output.h"
#include "message.h"
#include "getstring.h"
#include "symbol.h"
#include "helpfile.h"
#include "unitype.h"

#define INPUTCURSOR XC_xterm
#define TEXTX        3
#define TEXTY        3
#define STACKD      20

typedef struct STRINGINFO STRINGINFO;
struct STRINGINFO {
           Window win_id;
           unsigned int width, height;
           Bool is_integer, is_mapped;
           int charcurs, pixcurs;
           int maxpix, firstpix;
           unsigned int maxlen;
           Char *text;
           STRINGINFO *prev, *next;
        } ;

static unsigned long string_mask;
static XSetWindowAttributes string_attr;
static STRINGINFO *strinput = NULL;
static STRINGINFO *stackstr[STACKD];
static int stackdpt = 0;


#define compens       ((font_height() -line_height())/2)

#define INPUT_OK(A)   if (strinput && strinput->is_mapped && (A))

static void switch_cursor(STRINGINFO *sinfo)
{
    INPUT_OK(sinfo->pixcurs>=0) {
	set_output_window((void *) &sinfo->win_id);
	set_x_y(0,TEXTY+compens);
	set_drawstyle(INVISIBLE);
	thinspace(sinfo->pixcurs);
	set_drawstyle(VISIBLE);
	out_cursor(CURSOR);
	unset_output_window();
    }
}

static void remove_old_cursor(STRINGINFO *sinfo)
{
    if (strinput) {
	switch_cursor(strinput);
	strinput->pixcurs = strinput->charcurs= -1;
	strinput = NULL;
    }
    if (strinput!=sinfo )
	strinput = sinfo;
}

static void push_stack(void)
{
    if (stackdpt<STACKD)
	stackstr[stackdpt++] = strinput;
}

static void pop_stack(void)
{
    if (stackdpt)
	strinput = stackstr[--stackdpt];
    else
	strinput = NULL;
}

static void remove_from_stack(STRINGINFO *sinfo)
{
    int i, d=0;

    for (i=0; i<stackdpt; i++) {
	if (stackstr[i]== sinfo)
	    d++;
	else
	    stackstr[i-d] = stackstr[i];
    }
    stackdpt -= d;
}

#define char2width(A) (2 + TEXTX*2 + (A)*char_width('x'))
#define width2char(A) (((A)-2-TEXTX*2)/char_width('x'))
#define max_curspix(A) ((A) - TEXTX - 1)
#define min_curspix(A) (TEXTX+1)

unsigned int string_height(void)
{
    int i;
    push_fontgroup(POPUPFONT);
    i = (TEXTY * 2 + font_height());
    pop_fontgroup();
    return i;
}

unsigned int string_window_width(unsigned int nchars)
{
    int i;
    push_fontgroup(POPUPFONT);
    if (nchars>MAX_GETSTRING) nchars = MAX_GETSTRING;
    i = char2width(nchars);
    pop_fontgroup();
    return i;
}

static void string_draw(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;
    int i;

    if (sinfo && sinfo->is_mapped) {
	push_fontgroup(POPUPFONT);
	set_output_window( (void *) &sinfo->win_id);
	out_clear();
	set_x_y(sinfo->firstpix, TEXTY+compens);
	for (i=0; sinfo->text[i]; i++) {
	    if (i==sinfo->charcurs) out_cursor(CURSOR);
	    out_char(sinfo->text[i]);
	}
	if (sinfo->charcurs==i) out_cursor(CURSOR);
	out_char(Newline);
	unset_output_window();
	pop_fontgroup();
    }
}

static Bool adjust_firstpix(STRINGINFO *sinfo)
{
    int i;

    i=0;
    if (sinfo->pixcurs < (int)min_curspix(sinfo->width))
	i = min_curspix(sinfo->width);
    else
	if (sinfo->pixcurs > (int)max_curspix(sinfo->width))
	    i = max_curspix(sinfo->width);
    if (i) {
	sinfo->firstpix += i-sinfo->pixcurs;
	sinfo->pixcurs = i;
	return MP_True;
    } else
	return MP_False;
}

static void get_pos(STRINGINFO *sinfo, int x)
{
    int i;

    /* bepaal positie bij x en verplaats cursor */

    push_fontgroup(POPUPFONT);
    remove_old_cursor(sinfo);
    INPUT_OK(MP_True) {
	set_output_window(&sinfo->win_id);
	set_drawstyle(INVISIBLE);
	set_x_y(sinfo->firstpix, TEXTY+compens);
	for (i=0; x>where_x() && sinfo->text[i]; i++)
	    out_char(sinfo->text[i]);
	if (sinfo->charcurs !=i) {
	    sinfo->charcurs = i;
	    sinfo->pixcurs = where_x();
	}
	if (adjust_firstpix(sinfo)) {
	    unset_output_window();
	    string_draw( (void *) sinfo);
	} else {
	    if (sinfo->pixcurs>=0) {
		set_x_y(0,TEXTY+compens);
		thinspace(sinfo->pixcurs);
		set_drawstyle(VISIBLE);
		out_cursor(CURSOR);
	    }
	    unset_output_window();
	}
    }
    pop_fontgroup();
}

static void string_press(void *data, XButtonEvent *event)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;
    
    get_pos(sinfo,event->x);
    get_motion_hints(sinfo->win_id, -1);
}

static void string_release(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
    stop_motion_hints();
}

static void string_motion(void *data, int x_pos, int y_pos __attribute__((unused)))
{
    get_pos( (STRINGINFO *) data, x_pos);
}

void string_destroy(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    /*
    **  als niet hele lijst verdwijnt opnieuw koppelen
    */
    remove_from_stack(sinfo);
    if (sinfo == strinput) {
	pop_stack();
	if (strinput) {
	    strinput->charcurs = Ustrlen(strinput->text);
	    strinput->firstpix = min_curspix(strinput->width);
	    push_fontgroup(POPUPFONT);
	    strinput->pixcurs = strinput->firstpix +
		string_width(strinput->text, strinput->charcurs);
	    adjust_firstpix(strinput);
	    string_draw( (void *) strinput);
	    pop_fontgroup();
	}
    }
    free(sinfo->text);
    destroy_window(sinfo->win_id);
}

FUNCTIONS stringfuncs = {
    string_destroy, string_draw, NULL, string_press, string_release,
    string_motion };

void string_init(void)
{
    string_mask =
	(CWBackPixel | CWBorderPixel | CWEventMask |
	 CWCursor | CWColormap);

    string_attr.background_pixel = white_pixel;
    string_attr.border_pixel = black_pixel;
    string_attr.colormap = colormap;
    string_attr.event_mask = (ExposureMask | ButtonPressMask |
			      ButtonReleaseMask | KeyPressMask |
			      ButtonMotionMask | PointerMotionHintMask |
			      StructureNotifyMask | VisibilityChangeMask);
    string_attr.cursor = XCreateFontCursor(display, INPUTCURSOR);
}

void *string_make(Window parent, Char *txt, unsigned int maxlen,
		  unsigned int width, char *helpfile, int x_offset,
		  int y_offset, Bool is_integer)
{
    STRINGINFO *sinfo = 0;

    if ( !(sinfo = (STRINGINFO *) malloc( sizeof(STRINGINFO))) ||
	 !(sinfo->text =  (Char *) malloc((sizeof(Char)*((size_t) maxlen+1)))))  {
	message(MP_ERROR, translate("Out of memory in string. "));
	if (sinfo) free(sinfo);
	return NULL;
    }
    push_fontgroup(POPUPFONT);
    sinfo->width  = char2width(width2char(width));
    sinfo->is_integer = is_integer;
    sinfo->is_mapped = MP_False;
    if (txt)
	Ustrncpy(sinfo->text, txt, maxlen);
    else
	sinfo->text[0] = '\0';
    sinfo->text[maxlen] = '\0';
    sinfo->maxlen = maxlen;
    sinfo->maxpix = max_curspix(sinfo->width);
    sinfo->firstpix = min_curspix(sinfo->width);
    sinfo->charcurs = sinfo->pixcurs = -1;
    sinfo->prev = sinfo;
    sinfo->next = sinfo;
    sinfo->win_id = XCreateWindow(display, parent, x_offset, y_offset,
				 sinfo->width, string_height(),
				 0, CopyFromParent, InputOutput,
				 visual,
				 string_mask, &string_attr);
    pop_fontgroup();
    if (!helpfile) helpfile=helpname[GETSTRINGHELP];
    if (add_window(sinfo->win_id, STRINGWINDOW, parent, (void *) sinfo,
		   translate(helpfile)))
	return (void *) sinfo;
    XDestroyWindow(display, sinfo->win_id);
    free(sinfo->text);
    free(sinfo);
    return NULL;
}

void string_relation(void *data, void *prev, void *next)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    sinfo->prev = (STRINGINFO *) prev;
    sinfo->next = (STRINGINFO *) next;
}

void  string_resize(void *data, unsigned int new_width)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    push_fontgroup(POPUPFONT);
    sinfo->width = char2width(width2char(new_width));
    XResizeWindow(display, sinfo->win_id, sinfo->width, string_height());
    if (strinput == sinfo) {
	sinfo->pixcurs  = min_curspix(sinfo->width);
	sinfo->charcurs = 0;
    }
    sinfo->firstpix = min_curspix(sinfo->width);
    string_draw(data);
    pop_fontgroup();
}

void string_move(void *data, int newx, int newy)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    XMoveWindow(display, sinfo->win_id, newx, newy);
}

void string_refresh(void *data, Char *txt)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    Ustrncpy(sinfo->text, txt, sinfo->maxlen);
    sinfo->firstpix = min_curspix(sinfo->width);
    if (strinput == sinfo) {
	get_pos(strinput, string_width(strinput->text, -1) +
		min_curspix(strinput->width));
    }
    string_draw(data);
}

Char *string_text(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;
    Char *temp;

    temp = (Char *) malloc(sizeof(Char)*(Ustrlen(sinfo->text)+1));
    Ustrcpy(temp, sinfo->text);
    return temp;
}

void string_get_input(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    push_stack();
    push_fontgroup(POPUPFONT);
    remove_old_cursor(sinfo);
    sinfo->charcurs = Ustrlen(sinfo->text);
    sinfo->pixcurs = sinfo->firstpix +
	string_width(sinfo->text, -1);
    adjust_firstpix(sinfo);
    pop_fontgroup();
}

static Bool move_left(Char *str, int pos, int delta)
{
    int j;

    if (delta<=0) return MP_False;
    for (j=0; j<delta && str[pos+j]; j++);
    if (j<delta)
	str[pos] = '\0';
    else
	do {
	    str[pos] = str[pos+delta];
	} while (str[pos++]);
    return MP_True;
}

static Bool move_right(Char *str, int pos, int delta, unsigned int max)
{
    int i;

    if (delta<=0) return MP_False;
    i=Ustrlen(str);
    if (i+delta-1== (int)max)
	return MP_False;
    for ( ; i>=pos; i--)
	str[i+delta] = str[i];
    return MP_True;
}

static void str_delete(Index arg)
{
    INPUT_OK( move_left(strinput->text, strinput->charcurs, arg))
	string_draw( (void *) strinput);
}

static void str_m_delete(void)
{
    INPUT_OK( strinput->text[strinput->charcurs]) {
	strinput->text[strinput->charcurs] = '\0';
	string_draw( (void *) strinput);
    }
}

static void do_backspace(int delta)
{
    int i;

    if (delta<=0) return;
    INPUT_OK(strinput->charcurs) {
	strinput->charcurs -= delta;
	i=strinput->charcurs;	
	if (i<0) {
	    delta -=i;
	    i = strinput->charcurs =0;
	}
	push_fontgroup(POPUPFONT);
	strinput->pixcurs -= string_width(strinput->text+i, delta);
	(void) adjust_firstpix(strinput);
	(void) move_left(strinput->text, i, delta);
	string_draw((void *) strinput);
	pop_fontgroup();
    }
}

static void str_backspace(int arg)
{
    do_backspace((int)arg);
}

static void str_m_backspace(void)
{
    do_backspace(strinput->charcurs);
}

static void str_left(int count)
{
    INPUT_OK(strinput->charcurs) {
      int i;
	push_fontgroup(POPUPFONT);
	switch_cursor(strinput);
	i=count;
	while (i && strinput->charcurs) {
	  strinput->charcurs--;
	  strinput->pixcurs -=
	    char_width(strinput->text[strinput->charcurs]);
	  i--;
	}
	if (adjust_firstpix(strinput))
	    string_draw( (void *) strinput);
	else
	    switch_cursor(strinput);
	pop_fontgroup();
    }
}

static void str_right(int count)
{
    INPUT_OK(strinput->text[strinput->charcurs]) {
      int i;
	push_fontgroup(POPUPFONT);
	switch_cursor(strinput);
	i=count;
	while (strinput->text[strinput->charcurs] && i) {
	  strinput->pixcurs +=
	    char_width(strinput->text[strinput->charcurs]);
	  strinput->charcurs++;
	  i--;
	}
	if (adjust_firstpix(strinput))
	    string_draw(strinput);
	else
	    switch_cursor(strinput);
	pop_fontgroup();
    }
}

static void str_up(void)
{
    INPUT_OK(MP_True) get_pos(strinput->prev, strinput->pixcurs);

}

static void str_down(void)
{
    INPUT_OK(MP_True) get_pos(strinput->next, strinput->pixcurs);
}

static void str_home(void)
{
    INPUT_OK(MP_True) get_pos(strinput, strinput->firstpix -1);
}

static void str_end(void)
{
    INPUT_OK(MP_True) 
	get_pos(strinput, string_width(strinput->text, -1) +
		min_curspix(strinput->width));
}

static void str_next_field(void)
{
    INPUT_OK(MP_True) 
	get_pos(strinput->next, string_width(strinput->next->text,-1)+
		min_curspix(strinput->next->width));
}

static void str_prev_field(void)
{
    INPUT_OK(MP_True) 
	get_pos(strinput->prev, string_width(strinput->prev->text,-1)+
		min_curspix(strinput->prev->width));
}

static void str_insert_char(int keycode, int arg)
{
    INPUT_OK(((strinput->is_integer && Uisdigit(keycode)) ||
	      (!strinput->is_integer && (Uisprint(keycode)))) &&
	     move_right(strinput->text,strinput->charcurs, (int) arg,
			strinput->maxlen)) {
	int i;
        for (i=0; i<(int)arg; i++)
	    strinput->text[strinput->charcurs+i]= (Char) keycode;
	strinput->charcurs+= arg;
	push_fontgroup(POPUPFONT);
	strinput->pixcurs += arg*char_width(keycode);
	(void) adjust_firstpix(strinput);
	string_draw((void *) strinput);
	pop_fontgroup();
    }
}

static void str_insert_symbol(void)
{
  Char selchar;
  selchar = symbol_last();
  INPUT_OK(selchar) {
    str_insert_char((int)selchar, 1);
  }
}

static void str_insert_string(void)
{
    INPUT_OK(wmselection) {
	char *p = wmselection;
	push_fontgroup(POPUPFONT);
	for (; *p; p++) {
	    if ((strinput->is_integer && *p>='0' && *p<='9') ||
		(!strinput->is_integer && *p>=' ' && *p<='~')) {
		if (move_right(strinput->text, strinput->charcurs,
			       1, strinput->maxlen)) {
		    strinput->text[strinput->charcurs] = *p;
		    strinput->charcurs++;
		    strinput->pixcurs+= char_width(*p);
		}
	    }
	}
	(void) adjust_firstpix(strinput);
	string_draw((void*) strinput);
	pop_fontgroup();
    }
}

static void ask_selection(void)
{
    get_wm_selection();
}

#include "language.h"

static int call_noarg(int (*func)(), void **argl __attribute__((unused)))
{
  return (*func)();
}

static int call_intarg(int (*func)(), void **argl)
{
  return (*func)(*((int*)argl[0]));
}

static int call_intintarg(int (*func)(), void **argl)
{
  return (*func)(*((int*)argl[0]),*((int*)argl[1]));
}

typedef struct {
  Type tlist[4];
  int listlen;
  int (*callfunc)(int (*func)(), void **argl);
  Prototype *pt;
} PROTOLIST;

#define PRONOARG 0
#define PROINTARG 1
#define PROINTINTARG 2

static PROTOLIST protolist[] = 
{
  { {0}, 0, call_noarg, 0 },
  { {IntType}, 1, call_intarg, 0 },
  { {IntType,IntType}, 2, call_intintarg, 0},
  { {0}, 0, 0, 0}
};

typedef struct {
  void (*func)();
  char *name;
  char *description;
  int protopos;
} KeyFunc;

static KeyFunc keyfunclist[] =
{
  { str_insert_string,
    "StringPasteSel",
    "To insert the selection from the window system in the current string. "
    "It should be called after the selection is requested from the window system.",
    PRONOARG },
  { str_next_field,
    "S_next_field",
    "Move to the next input field.",
    PRONOARG },
  { str_prev_field,
    "S_prev_field",
    "Move to the previous input field.",
    PRONOARG },
  { str_delete,
    "S_delete_char",
    "Remove the N characters after the cursor.",
    PROINTARG },
  { str_m_delete,
    "S_kill_line",
    "Remove everything after the cursor.",
    PRONOARG },
  { str_backspace,
    "S_backward_delete_char",
    "Remove the N characters before the cursor.",
    PROINTARG },
  { str_m_backspace,
    "S_backward_kill_line",
    "Remove all the characters before the cursor.",
    PRONOARG },
  { str_left,
    "S_backward_char",
    "Move N characters backward.",
    PROINTARG },
  { str_right,
    "S_forward_char",
    "Move N characters forward.",
    PROINTARG },
  { str_up,
    "S_previous_line",
    "Move to the previous input field (with the cursor at a simular position).",
    PRONOARG },
  { str_down,
    "S_next_line",
    "Move to the next input field (with the cursor at a simular position).",
    PRONOARG },
  { str_home,
    "S_beginning_of_line",
    "Move to the start of the input field.",
    PRONOARG },
  { str_end,
    "S_end_of_line",
    "Move to the end of the input field.",
    PRONOARG },
  { ask_selection,
    "S_insert_selection",
    "Insert the selection from the window system.",
    PRONOARG },
  { str_insert_char,
    "S_self_insert",
    "Insert the pressed key N times.",
    PROINTINTARG },
  { str_insert_symbol,
    "S_symbol_click",
    "Insert the selected symbol",
    PRONOARG },
  { 0,0,0,0 }
};

void string_keyboard(void)
{
  int i;

    for (i=0; protolist[i].callfunc; i++) {
      protolist[i].pt = define_prototype(protolist[i].tlist,
				       protolist[i].listlen,
				       0,
				       protolist[i].callfunc);
    }
    for (i=0; keyfunclist[i].func!=NULL; i++) {
      define_function(keyfunclist[i].name, keyfunclist[i].description,
		      protolist[keyfunclist[i].protopos].pt,
		      keyfunclist[i].func);
    }
}


void string_map(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    sinfo->is_mapped = MP_True;
}

void string_unmap(void *data)
{
    STRINGINFO *sinfo = (STRINGINFO *) data;

    sinfo->is_mapped = MP_False;
}
