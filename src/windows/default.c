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
**   File : default.c
**   Datum: 11-4-92
**   Doel : Instellingen van het programma veranderen.
**          Vastleggen van de layout.
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
/* #include "keymap.h" */
#include "getstring.h"
#include "message.h"
#include "default.h"
#include "popup.h"
#include "helpfile.h"
#include "checkbox.h"
#include "output.h"
#include "fileselc.h"

#include "parse.h"

#define DEFAULTNAME  "MathSpad Properties"
#define ICONNAME     "Default"
#define LEFT_LINE    5
#define MIDDLE_LINE  10
#define RIGHT_LINE   5
#define LINE_SP      5

enum button {SETBUTTON, SAVEBUTTON, DONEBUTTON, NR_BUTTON };

static
char *defaultbutton[NR_BUTTON] = { "Set", "Save", "Done" };
static
int defaulthelp[NR_BUTTON] =
{ DEFAULTSETHELP, DEFAULTSAVEHELP, DEFAULTDONEHELP };

#define NR_FIELD  23

static int saveponquit=0;

static
struct {
    char *defdescript;
    Char *descript;
    void *data;
    unsigned int maxlen;
    Char *text;
    Bool is_integer;       /* tevens layout_change-factor */
    int min, max, helpnr;
    void *variable;
    int fileselc;
} field[NR_FIELD] = {
{"Screen",       0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"  Line space", 0, 0, 15,0, MP_True,0, 20,DEFAULTSCREENHELP,&line_space},
{"  Tab size",   0, 0, 15,0, MP_True,0,200,DEFAULTSCREENHELP,&screen_tab},
{"  Elastic space",0,0,15,0, MP_True,0, 20,DEFAULTSCREENHELP,&screen_space},
{"Latex",        0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"  Line space", 0, 0, 40,0,MP_False,0,  0, DEFAULTLATEXHELP,&latex_line_unit},
{"  Tab size",   0, 0, 40,0,MP_False,0,  0, DEFAULTLATEXHELP,&latex_tab_unit},
{"  Elastic space",0,0,40,0,MP_False,0,  0, DEFAULTLATEXHELP,&latex_space_unit},
{"Left margin",  0, 0, 15,0, MP_True,0, 10,DEFAULTLMARGINHELP,&latex_side},
{"",             0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"Directory",    0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"  Documents",  0, 0,512,0,MP_False,0,  0,   DEFAULTDIRHELP,&userdir, 1},
{"  Stencils",   0, 0,512,0,MP_False,0,  0,   DEFAULTDIRHELP,&notationdir, 1},
{"  Output",     0, 0,512,0,MP_False,0,  0,   DEFAULTDIRHELP,&latexdir, 1},
{"",             0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"Fonts",        0, 0,100,0,MP_False,0,  0,  DEFAULTFONTHELP,&fontfile[EDITFONT]},
{"  Stencils",   0, 0,100,0,MP_False,0,  0,  DEFAULTFONTHELP,&fontfile[NOTATIONFONT]},
{"  Symbols",    0, 0,100,0,MP_False,0,  0,  DEFAULTFONTHELP,&fontfile[SYMBOLFONT]},
{"  Popups",     0, 0,100,0,MP_False,0,  0,  DEFAULTFONTHELP,&fontfile[POPUPFONT]},
{"",             0, 0,  0,0,MP_False,0,  0,                0,NULL},
{"Keys",         0, 0,100,0,MP_False,0,  0,  DEFAULTKEYSHELP,&keypath},
{"Save Time",    0, 0, 15,0, MP_True,1, 60, DEFAULTSTIMEHELP,&save_minute},
/* to add a checkbox, set third item to 0, last item to variable */
{"Save Project", 0, 0,  0,0, MP_True,0,  1, DEFAULTSTIMEHELP,&saveponquit}
};

#define intvar(A) *((int *) field[A].variable)
#define charrefvar(A) (*((Char **) field[A].variable))
#define FONTFIELD 15
#define KEYFIELD  20

static Window defaultwin;
static int last_xpos = 0, last_ypos = 0;
static unsigned int last_width = 0, last_height = 0;
static int x_offset, y_offset;
static int compens;
static int string_pos_x;
static XTextProperty default_name, icon_name;
static Char *name, *iconname;

#define sub_width(A) ((unsigned int)((A) - string_pos_x - RIGHT_LINE))

#define nextfield(A)  do { if (A==NR_FIELD-1) A=0; else A++; } while (!field[A].maxlen)
#define prevfield(A)  do { if (!A) A=NR_FIELD-1;   else A--; } while (!field[A].maxlen)

static void close_window(Bool bad)
{
    int i;
    if (default_is_open) {
      default_iconized = MP_False;
      default_is_open = MP_False;
      for (i=0; i<NR_FIELD; i++)
	if (field[i].maxlen && field[i].data) string_destroy(field[i].data);
      popup_remove(defaultwin);
      if (!bad) {
	  XDestroyWindow(display, defaultwin);
	  destroy_window(defaultwin);
      }
    }
}

static void default_bad_end(void *data __attribute__((unused)))
{
    close_window(MP_True);
}

static void default_draw(void *data __attribute__((unused)))
{
    int i;

    if (!default_is_open) return;
    push_fontgroup(POPUPFONT);
    XDrawLine(display, defaultwin, get_GC(Normal,0,0),
	      0, INTERSPACE*2+(int)button_height,
	      (int)display_width, INTERSPACE*2+(int)button_height);
    set_output_window((void*)&defaultwin);
    for (i=0; i<NR_FIELD; i++) {
      set_x_y(LEFT_LINE, y_offset+i*((int)string_height()+LINE_SP)+compens);
      out_string(field[i].descript);
    }
    unset_output_window();
    pop_fontgroup();
}

static Char buffer[5000];

static Bool keypath_exist(Char *path)
{
    Char *c, *h, *g;
    Char t;
    Char *dir[4];
    Char *fname;

    if (!Ustrcmp(path,keypath)) return MP_False;
    concat_in(buffer, homedir, translate("mathspad/"));
    dir[1] = buffer;
    h = buffer+Ustrlen(buffer)+1;
    concat_in(h,buffer,translate("%.mpk"));
    dir[0] = h;
    dir[3] = program_dir;
    h = h+ Ustrlen(h)+1;
    concat_in(h, program_dir, translate("%.mpk"));
    dir[2] = h;
    g = buffer+Ustrlen(buffer)+1;
    c = h = path;
    while (*c) {
	c++;
	if (*h==':') h++;
	while (*c && *c!=':') c++;
	t = *c;
	*c = '\0';
	fname = search_through_dirs(dir, 4, h);
	*c = t;
	if (!fname) return MP_False;
	h=c;
    }
    free(keypath);
    keypath = path;
    return MP_True;
}

void load_keypath(void)
{
    Char *c, *h;
    Char t;

    c = h = program_keypath;
    while (*c) {
	c++;
	if (*h==':') h++;
	while (*c && *c!=':') c++;
	t = *c;
	*c = '\0';
	if (lex_open_file(UstrtoFilename(h))) {
	  parse_input();
	}
	*c = t;
	h=c;
    }
    if (keypath) {
	c = h = keypath;
	while (*c) {
	    c++;
	    if (*h==':') h++;
	    while (*c && *c!=':') c++;
	    t = *c;
	    *c = '\0';
	    if (lex_open_file(UstrtoFilename(h))) {
	      parse_input();
	    }
	    *c = t;
	    h=c;
	}
    }
}

static void default_set(void)
{
    Char *newstring;
    int i,j;
    Bool lchanged = MP_False;
    
    for (i=0; i<NR_FIELD; i++)
	if (field[i].variable) {
	    if (field[i].maxlen)
		if (field[i].is_integer) {
		    newstring = string_text(field[i].data);
		    /* Ustrtol autodetects base (and start character) */
		    if (aig(j=Ustrtol(newstring, NULL, 0))) {
		        Char *fres;
			if (j<field[i].min) j = field[i].min;
			if (j>field[i].max) j = field[i].max;
			/* Ultostr converts the string back a standard format */
			fres = Ultostr(j,field[i].text+field[i].maxlen);
			Ustrcpy(field[i].text, fres);
			if (intvar(i) != j) {
			    intvar(i) = j;
			    lchanged = MP_True;
			}
		    }
		    free(newstring);
		    string_refresh(field[i].data, field[i].text);
		} else {
		    if (i>=FONTFIELD && i<KEYFIELD) {
			if (new_fontfile(i-FONTFIELD,string_text(field[i].data))) {
			    field[i].text = charrefvar(i);
			    lchanged = MP_True;
			} else
			    string_refresh(field[i].data, field[i].text);
		    } else if (i==KEYFIELD) {
			if (keypath_exist(string_text(field[i].data))) {
			    load_keypath();
			    field[i].text = charrefvar(i);
			} else
			    string_refresh(field[i].data, field[i].text);
		    } else {
			free(field[i].text);
			field[i].text = charrefvar(i) = standard_dir(string_text(field[i].data));
			string_refresh(field[i].data, field[i].text);
		    }
		}
	    else
		intvar(i) = checkbox_value(field[i].data);
	}
    if (lchanged)
	call_layout_change();
}

static void default_handle_button(void *data __attribute__((unused)), int bnr)
{
    switch (bnr) {
    case SETBUTTON:
	default_set();
	message(MP_MESSAGE,translate("Defaults set."));
	break;
    case SAVEBUTTON:
        default_set();
	save_defaults();
	message(MP_MESSAGE, translate("Defaults saved."));
	break;
    case DONEBUTTON:
	if (can_close_default) default_close();
	break;
    }
}

static void default_layout_change(void *data __attribute__((unused)))
{
    int i,j, wl, wr;
    unsigned int win_width, win_height;
    XSizeHints size_hints;

    push_fontgroup(POPUPFONT);
    if (default_is_open) {
	win_height = INTERSPACE*4+button_height +
	    (string_height()+LINE_SP)* NR_FIELD;
	wl = wr = 0;
	for (i=0; i<NR_FIELD; i++) {
	    if ((j=string_width(field[i].descript,-1)) > wl) wl=j;
	    if ((j=string_window_width(field[i].maxlen)) >wr) wr=j;
	}
	win_width = wl+wr+LEFT_LINE+ MIDDLE_LINE + RIGHT_LINE;
	size_hints.flags = PMinSize;
	size_hints.min_width = win_width;
	size_hints.min_height = win_height;
	XSetWMNormalHints(display, defaultwin, &size_hints);
	XResizeWindow(display, defaultwin, win_width, win_height);
	XClearWindow(display, defaultwin);
	x_offset = LEFT_LINE + wl + MIDDLE_LINE;
	y_offset = button_height+INTERSPACE*3 + LINE_SP;
	string_pos_x = x_offset;
	compens = (string_height() + LINE_SP - font_height())/2 - 2;
	for (i=0; i<NR_FIELD; i++) {
	    if (field[i].maxlen) {
		string_move(field[i].data, x_offset,
			    (int)(y_offset+i*(string_height()+LINE_SP)));
		string_resize(field[i].data, sub_width(win_width));
	    }
	}
	if (!default_iconized)
	    default_draw(NULL);
    }
    pop_fontgroup();
}

static MENU fontfilemenu;
static int popup_nr=0;

/*
static void remove_fontfile(unsigned int c __attribute__((unused)))
{
}
*/
/*
static void handle_fontfilename(void *data __attribute__((unused)), int nr)
{
    if (default_is_open && fontfilemenu.line && popup_nr) {
	int i=0;
	Char *c=fontfilemenu.line[nr].txt;
	char *h;
	while (c[i]) i++;
	h= (char*) malloc(sizeof(char)*(i+1));
	i=0;
	if (h) {
	    while (aig(h[i]=(char) c[i])) i++;
	    string_refresh(field[popup_nr].data, h);
	    free(h);
	}
    }
    remove_fontfile(0);
}
*/
/*
static int check_name(char *ext, char *cname)
{
    int i,j;
    if ((i=strlen(ext))>=(j=strlen(cname))) return 0;
    return !strcmp(cname+j-i,ext);
}
*/
#define FONTFILEEXT ".mpf"

static void make_fontfile_popup(int nr, Bool motion __attribute__((unused)))
{
    if (fontfilemenu.ownwin) return;
    fontfilemenu.menu = popup_define(translate("FontFiles"));
    fontfilemenu.freesub=MP_False;
    fontfilemenu.parentwin=defaultwin;
    get_motion_hints(root_window,0);
    motion_get_pos(&fontfilemenu.x,&fontfilemenu.y);
    stop_motion_hints();
    (void) popup_make(&fontfilemenu);
    popup_nr = nr;
}

static void default_handle_fileselc(void *data, Char *name)
{
  int i= (int)data;
  if (field[i].fileselc) {
    free(field[i].text);
    field[i].text = charrefvar(i) = Ustrdup(name);
    string_refresh(field[i].data, field[i].text);
  }
}

static void default_press(void *data __attribute__((unused)), XButtonEvent *event)
{
    int i;
    if (!default_is_open) return;
    push_fontgroup(POPUPFONT);
    i = (event->y- y_offset)/(string_height()+LINE_SP);
    if (field[i].fileselc) {
      dirselc_open(default_handle_fileselc, (void*)i,
		   translate(field[i].defdescript),
		   string_text(field[i].data), NULL, NULL,
		   defaultwin);
    }
    pop_fontgroup();
    get_motion_hints(defaultwin,-1);
}

static void default_motion(void *data __attribute__((unused)), int x __attribute__((unused)), int y __attribute__((unused)))
{
}

static void default_release(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
    stop_motion_hints();
}

static void default_resize(void *data __attribute__((unused)), XConfigureEvent *event)
{
    int i,x,y;

    if (default_is_open) {
	last_width = event->width;
	last_height = event->height;
	window_manager_added(defaultwin, &x,&y);
	last_xpos = event->x-x;
	last_ypos = event->y-y;
	for (i=0; i<NR_FIELD; i++)
	    if (field[i].maxlen)
		string_resize(field[i].data, sub_width(event->width));
    }
}

static void default_iconize(void *data __attribute__((unused)))
{
    int i;

    default_iconized = MP_True;
    for (i=0; i<NR_FIELD; i++)
	if (field[i].maxlen)
	    string_unmap(field[i].data);
}

static void default_deiconize(void *data __attribute__((unused)))
{
    int i;

    default_iconized = MP_False;
    for (i=0; i<NR_FIELD; i++)
	if (field[i].maxlen)
	    string_map(field[i].data);
}

static int default_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return MP_False;
}

static void default_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

FUNCTIONS defaultfuncs = {
    default_bad_end, default_draw, default_resize, default_press,
    default_release, default_motion, default_iconize, default_deiconize,
    NULL, NULL, default_layout_change, NULL, NULL, NULL, NULL, NULL, 
    default_last_pos, default_set_last_pos };


void default_update(void)
{
    int i;
    for (i=0; i<NR_FIELD; i++) {
	if (field[i].variable) {
	    if (field[i].is_integer) {
		if (field[i].maxlen) {
		  Char *s,*t;
		  if (!field[i].text)
		    field[i].text = (Char *) malloc(sizeof(Char)*((size_t)field[i].maxlen+1));
		  field[i].text[field[i].maxlen]=0;
		  s = Ultostr(intvar(i), field[i].text+field[i].maxlen);
		  t=field[i].text;
		  while (aig(*t++ = *s++));
		}
	    } else
		field[i].text = charrefvar(i);
	}
    }
}

void default_init(void)
{
    int i,j, wl,wr, lw, lh;

    default_update();
    push_fontgroup(POPUPFONT);
    lh = INTERSPACE*4+button_height + (string_height()+LINE_SP)* NR_FIELD;
    wl = wr = 0;
    for (i=0; i<NR_FIELD; i++) {
        field[i].descript = translate(field[i].defdescript);
	if ((j=string_width(field[i].descript,-1)) > wl) wl=j;
	if ((j=string_window_width(field[i].maxlen)) >wr) wr=j;
    }
    lw = wl+wr+LEFT_LINE+ MIDDLE_LINE + RIGHT_LINE;
    x_offset = LEFT_LINE + wl + MIDDLE_LINE;
    y_offset = button_height+INTERSPACE*3 + LINE_SP;
    string_pos_x = x_offset;
    compens = (string_height() + LINE_SP - font_height())/2 - 2;
    if (!last_width) {
	last_width = lw;
	last_height = lh;
	last_xpos = (display_width - lw)/2;
	last_ypos = (display_height - lh)/2;
    }
    name = translate(DEFAULTNAME);
    iconname = translate(ICONNAME);
    { char *tn, *icn;
      tn = (char*)UstrtoLocale(name);
      icn = (char*)UstrtoLocale(iconname);
      if (!XStringListToTextProperty(&tn, 1, &default_name) ||
	  !XStringListToTextProperty(&icn, 1, &icon_name)) {
	message(MP_EXIT-1, translate("No location for defaultname."));
      }
    }
    pop_fontgroup();
}

void default_open(void)
{
    XSetWindowAttributes default_attr;
    unsigned long default_mask;
    int x,y;
    int i,j;
    XSizeHints size_hints;

    if (default_is_open) {
      if (default_iconized) XMapWindow(display, defaultwin);
      default_iconized=MP_False;
      XRaiseWindow(display, defaultwin);
      return;
    }

    default_mask = (CWBackPixel | CWBorderPixel | CWBitGravity |
		    CWColormap | CWEventMask);
    default_attr.background_pixel = white_pixel;
    default_attr.border_pixel = black_pixel;
    default_attr.colormap = colormap;
    default_attr.bit_gravity = NorthWestGravity;
    default_attr.event_mask = (ExposureMask | ButtonPressMask |
			       ButtonMotionMask | PointerMotionHintMask |
			       ButtonReleaseMask | KeyPressMask |
			       FocusChangeMask |
			       StructureNotifyMask | VisibilityChangeMask);
    defaultwin = XCreateWindow(display, root_window,
			       last_xpos, last_ypos, last_width, last_height,
			       BORDERWIDTH, CopyFromParent, InputOutput,
			       visual,
			       default_mask, &default_attr);
    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width = last_width;
    size_hints.min_height = last_height;
    XSetWMProperties(display, defaultwin, &default_name, &icon_name,
		     NULL, 0, &size_hints, &wm_hints, &class_hints);
    set_protocols(defaultwin);
    i=0;
    x = LEFT_LINE;
    y = INTERSPACE;
    push_fontgroup(POPUPFONT);
    if (add_window(defaultwin, DEFAULTWINDOW, root_window,
		   NULL,translate(helpname[DEFAULTHELP]))) {
	while (i<NR_BUTTON &&
	       button_make(i, defaultwin, translate(defaultbutton[i]), &x, y, 1, NULL,
			   helpname[defaulthelp[i]], NULL, NULL, NULL,
		           default_handle_button, NULL, NULL))
	    i++,x+=BINTERSPACE;
	if (i==NR_BUTTON) {
	    i=0;
	    while (i<NR_FIELD &&
		   (!field[i].variable ||
		    (field[i].maxlen &&
		     (field[i].data =
		      string_make(defaultwin,
				  (field[i].is_integer?field[i].text:charrefvar(i)),
				  field[i].maxlen, sub_width(last_width),
				  helpname[field[i].helpnr],x_offset,
				  (int)(y_offset+i*(string_height()+LINE_SP)),
				  field[i].is_integer ))) ||
		    (!field[i].maxlen &&
		     (field[i].data =
		      checkbox_make(defaultwin, x_offset,
				    y_offset+i*((int)string_height()+LINE_SP),
				    intvar(i))))))
		i++;
	    if (i<NR_FIELD) i=0;
	} else
	    i=0;
    }
    if (!i)
	XDestroyWindow(display, defaultwin);
    else {
	x = 0; prevfield(x);   /* x == prevfield(i)      */
	i = x; nextfield(i);   /* i == element           */
	y = i; nextfield(y);   /* y == nextfield(i)      */
	j = i;                 /* houd startpositie bij  */
	do {
	    string_relation(field[i].data, field[x].data, field[y].data);
	    x = i;               /*  eq. met  nextfield(x) */
	    i = y;               /*  eq. met  nextfield(i) */
	    nextfield(y);
	} while (i!=j);
	XMapSubwindows(display, defaultwin);
	XMapWindow(display, defaultwin);
	string_get_input(field[j].data);
	default_iconized = MP_False;
	default_is_open = MP_True;
    }
    pop_fontgroup();

}

void default_close(void)
{
    close_window(MP_False);
}
