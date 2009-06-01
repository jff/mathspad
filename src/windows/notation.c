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
**  File : notation.c
**  Datum: 14-5-92
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "button.h"
#include "scrollbar.h"
#include "remark.h"
#include "message.h"
#include "output.h"
#include "keyboard.h"
#include "notatype.h"
#include "notadef.h"
#include "notation.h"
#include "fileselc.h"
#include "menu.h"
#include "popup.h"
#include "helpfile.h"
#include <limits.h>

#define TemplateKey 'T'
#define TemplateMode ModeAdd(ModeAdd(ModeAdd(ModeAdd(0,ModShift),ModMeta),ModAlt),ModHyper)


#define NOTATIONNAME  " Stencil"
#define ICONNAME "Stencil"
#define MODIFIEDSTRING " (modified)"

#define NAME_LENGTH  300

enum button { LOADBUTTON,   SAVEBUTTON, RENAMEBUTTON,
	      DEFINEBUTTON,
    DONEBUTTON, NR_BUTTON };

static
char *notationbutton[NR_BUTTON] = {"Load",  "Save",   "Rename",
				   "Define",
    "Done"};

static
int notahelp[NR_BUTTON] = 
{ STENCILLOADHELP, STENCILSAVEHELP, STENCILRENAMEHELP,
  STENCILDEFINEHELP,
    STENCILDONEHELP };

typedef struct {
    int kind, anr, mnr, height;
    unsigned long code;
} LISTELM;

typedef struct {
    LISTELM *list;
    unsigned int listsize, maxlist, listpos;
    int fnr;
    void *scrollver;
    Char *name,*subname;
    Window notawin, notadrawwin;
    unsigned win_width, win_height;
    int win_xpos, win_ypos;
    Bool is_icon;
} NOTATIONINFO;

static int last_width=0, last_height=0, last_xpos = 0, last_ypos = 0;
static int is_opened = MP_False;
static Bool state_open = MP_False, as_icon = MP_False;
static NOTATIONINFO *state_window = NULL;
static unsigned long notation_mask;
static XSetWindowAttributes notation_attr;
static char *notation_name = NOTATIONNAME, *icon_name = ICONNAME;
static char *notation_filename = NULL;
static XTextProperty notaname, iconname;
static Char *kinddes[MAX_KIND];
static NOTATIONINFO *nisel=NULL;
static int anrsel= -1, kindsel= -1;
static int nnr_last = -1, vnr_last = 0;
static int linesel, colsel;
static int number_open = 0;
static int number_icon = 0;
static char *remark_text = NULL;

#define sub_width(A)   ((A) - INTERSPACE*3 -SCROLLBARSIZE)
#define sub_height(A)  ((A) - INTERSPACE*3 - button_height)
#define pos_x_with     (INTERSPACE*2 +SCROLLBARSIZE)
#define pos_x_without  (INTERSPACE)
#define pos_y_with     (INTERSPACE*2 +(int)button_height)
#define pos_y_without  (INTERSPACE*2 +(int)button_height)

static void set_name(NOTATIONINFO *ninf)
{
    XTextProperty prop_name;
    Char number[8];
    int nr,i;

    free(ninf->subname);
    free(ninf->name);
    nr=get_notation_number(ninf->fnr);
    i=0;
    if (nr) {
	number[i++]=' ';
	number[i++]='<';
	if (nr>9) number[i++]='0'+nr/10;
	number[i++]='0'+nr%10;
	number[i++]='>';
    }
    number[i]='\0';
    ninf->subname = concat(get_notation_filename(ninf->fnr), number);
    if (notation_not_saved(ninf->fnr))
	ninf->name = concat(ninf->subname, translate(NOTATIONNAME MODIFIEDSTRING));
    else
	ninf->name=concat(ninf->subname, translate(NOTATIONNAME));
    {
      char *tn;
      tn = (char*)UstrtoLocale(ninf->name);
      if (!XStringListToTextProperty(&tn, 1, &prop_name)) {
	message(MP_ERROR, translate("No location for stencilname."));
	return;
      }
      XSetWMName(display, ninf->notawin, &prop_name);
      tn = (char*)UstrtoLocale(ninf->subname);
      if (!XStringListToTextProperty(&tn, 1, &prop_name)) {
	message(MP_ERROR, translate("No location for stencil icon."));
	return;
      }
      XSetWMIconName(display, ninf->notawin, &prop_name);
    }
}

static void notation_file_use_state(
   int x __attribute__((unused)), int y __attribute__((unused)),
   int w __attribute__((unused)), int h __attribute__((unused)),
   int i __attribute__((unused)), int s __attribute__((unused)),
   Char *str)
{
    int j;
    Char *dirs[9];
    Char name[128];
    Char buf[2048];
    Char *filename,*hc,*g;

    filename = strip_name(str);
    Ustrcpy(name,filename);
    *filename='\0';
    g=buf;
    concat_in(g,str, translate("%.mps"));
    dirs[0] = g;
    g = g+Ustrlen(g)+1;
    concat_in(g,str,translate("%.nota"));
    dirs[1]=g;
    g = g+Ustrlen(g)+1;
    dirs[2]=str;
    concat_in(g,notationdir,translate("/%.mps"));
    dirs[3]=g;
    g = g+Ustrlen(g)+1;
    concat_in(g,notationdir,translate("/%.nota"));
    dirs[4]=g;
    g = g+Ustrlen(g)+1;
    dirs[5]=notationdir;
    concat_in(g,program_notationdir,translate("%.mps"));
    dirs[6]=g;
    g = g+Ustrlen(g)+1;
    concat_in(g,program_notationdir,translate("%.nota"));
    dirs[7]=g;
    g = g+Ustrlen(g)+1;
    dirs[8]=program_notationdir;
    hc=Ustrrchr(name,'.');
    if (hc && (!Ustrcmp(hc,translate(".nota")) || !Ustrcmp(hc,translate(".mps")))) *hc='\0';
    g = search_through_dirs(dirs,9,name);
    if (g) {
	j = load_notation_window(-1, g);
	add_file_ref(j);
    } else {
	if (hc) *hc='.';
	*filename=name[0];
	message2(MP_CLICKREMARK, translate("Unable to load stencil file "), str);
    }
    free(str);
}

static int decode_pos(int col, unsigned long code)
{
    unsigned long stop = 0x1<<(col+1),mask = 0x1;
    int i=0;
    Bool last = MP_False;

    while (mask!=stop) {
	if (((mask&code)>0)!=last) {
	    last = !last;
	    i++;
	}
	mask = mask<<1;
    }
    return i;
}

static void redraw_notation(NOTATIONINFO *ninf, int kind,int anr)
{
    TextMode tmode;
    Char *k = NULL;
    Char *c = NULL;
    NOTATION *nota;

    if (!(nota = get_notation_kind(ninf->fnr, kind, anr))) return;
    if (kind==kindsel && anr==anrsel && ninf==nisel)
	tmode = Reverse;
    else
	tmode = Normal;
    set_text_mode(tmode);
    out_char(' ');
    c= nota->name;
    if (!c || !*c) {
	k = nota->vers[0].format[NAMEFORMAT];
	if (!k || !*k) k = nota->vers[0].format[SCREENFORMAT];
	while (*k) {
	    if (!IsTab(*k) || *k<SoftNewline) out_char(Char2Ph(*k));
	    k++;
	}
    } else
	out_string(c);
    out_char(' ');
    set_text_mode(Normal);
    out_char(Rtab);
}

static void make_list(NOTATIONINFO *ninf)
{
    int k, i, n, notanr, xmax, newpos=0, kind=0, ts=0,
	anr = -1;
    
#define set_listelm(A,B,C) ((ninf->list[A].kind = (B)), \
                            (ninf->list[A].anr = (C)), \
                            (ninf->list[A].mnr = 0),(ninf->list[A].code=0L))
#define next_listelm    ninf->list[++ninf->listsize].mnr=0
#define encode_pos(A)   ninf->list[ninf->listsize].code ^= (ULONG_MAX<<(A))

    push_fontgroup(NOTATIONFONT);
    if (ninf->listsize) {
	kind = ninf->list[ninf->listpos].kind;
	anr = ninf->list[ninf->listpos].anr;
    }
    n = nr_visible(ninf->fnr);
    if (nisel && ninf==nisel) {
	nisel=NULL;
	kindsel = -1;
	anrsel = -1;
    }
    if (n+MAX_KIND > (int)ninf->maxlist) {
	free(ninf->list);
	ninf->list = (LISTELM *) malloc( sizeof(LISTELM) * (n+MAX_KIND));
	ninf->maxlist = n+MAX_KIND;
    }
    ninf->listsize = 0;
    set_output_window(&ninf->notadrawwin);
    set_text_mode(Normal);
    set_drawstyle(INVISIBLE);
    set_margin(2*INTERSPACE);
    set_x_y(0,0);
    ts = getsimpletabsize();
    xmax = sub_width(ninf->win_width);
    for (k=0; k<MAX_KIND; k++) {
	if (((notanr = get_notation_nr(ninf->fnr, k, 0)))>=0) {
	    set_listelm(ninf->listsize, k,-1);
	    ninf->list[ninf->listsize].height = line_height();
	    if (kind==k && anr == -1) newpos = ninf->listsize;
	    next_listelm;
	    i=0;
	    while (notanr>=0) {
		if (kind==k && anr==i) newpos = ninf->listsize;
		if (!ninf->list[ninf->listsize].mnr) {
		    set_listelm(ninf->listsize, k, i);
		    redraw_notation(ninf, k, i);
		    ninf->list[ninf->listsize].mnr = 1;
		    i++;
		    notanr = get_notation_nr(ninf->fnr, k,i);
		} else {
		    int p = where_x()/ts;
		    redraw_notation(ninf, k, i);
		    if (where_x()>xmax) {
			out_char(Newline);
			ninf->list[ninf->listsize].height = where_y();
			next_listelm;
			set_x_y(0,0);
		    } else {
			encode_pos(p);
			ninf->list[ninf->listsize].mnr++;
			i++;
			notanr = get_notation_nr(ninf->fnr, k ,i);
		    }
		}
	    }
	    if (ninf->list[ninf->listsize].mnr) {
		out_char(Newline);
		ninf->list[ninf->listsize].height = where_y();
		next_listelm;
		set_x_y(0,0);
	    }
	}
    }
    if (ninf->listsize>1) ninf->listpos = newpos;
    unset_output_window();
    pop_fontgroup();
}

static void notation_bad_end(void *data)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    if (!--number_open) {
	notation_is_open = MP_False;
    }
    number_icon--;
    if (notadef_is_open && edit_fnr==ninf->fnr && last_window(ninf->fnr))
	notadef_close();
    if (nisel==ninf) nisel= NULL;
    free_notation_window(ninf->fnr);
    free(ninf->list);
    free(ninf->subname);
    free(ninf->name);
    destroy_window(ninf->notawin);
}

static void get_position(NOTATIONINFO *ninf, int kind, int anr,
			 int *line, int *col)
{
    unsigned int i;

    i=0;
    while (i<ninf->listsize && 
	   (ninf->list[i].kind != kind ||
	    ninf->list[i].anr+ninf->list[i].mnr <=anr)) {
	i++;
    }
    *line = i;
    *col = anr - ninf->list[i].anr;
}

static int position_detect(NOTATIONINFO *ninf, int *xp, int *yp)
{
    LISTELM *tmpl;
    unsigned int i;
    int x,y;
    int ts = getsimpletabsize();

    if ((*yp)<0 || *xp<0) return -1;
    if (*yp>(int)sub_height(ninf->win_height)) return -1;
    y=0;
    i=ninf->listpos;
    while (y+ninf->list[i].height<*yp && i<ninf->listsize) {
	y+=ninf->list[i].height;
	i++;
    }
    if (i >= ninf->listsize) return -1;
    tmpl = ninf->list+i;
    if (!tmpl->mnr) return -1;
    x = (*xp - 2*INTERSPACE)/ts;
    *xp = decode_pos(x,tmpl->code);
    *yp = i;
    kindsel=tmpl->kind;
    nisel = ninf;
    return tmpl->anr + *xp;
}

static void draw_all(NOTATIONINFO *ninf)
{
    int idx = ninf->listpos;
    LISTELM *tmpl;
    int i,j;
    int y=0;

    if (ninf->is_icon || idx<0) return;
    set_output_window(&ninf->notadrawwin);
    set_text_mode(Normal);
    set_margin(2*INTERSPACE);
    set_x_y(0,0);
    while (idx<(int)ninf->listsize && y<(int)sub_height(ninf->win_height)) {
	tmpl = ninf->list+idx;
	y = where_y();
	if (!(i=tmpl->mnr)) {
	    set_text_mode(Normal);
	    set_underline(ON);
	    out_string(kinddes[tmpl->kind]);
	    set_underline(OFF);
	} else {
	    j= tmpl->anr;
	    while (i>0) {
		redraw_notation(ninf, tmpl->kind, j);
		j++;
		i--;
	    }
	}
	out_char(Newline);
	tmpl->height = where_y()-y;
	y=where_y();
	idx++;
    }
    clear_to_end_of_page();
    unset_output_window();
}

static void notation_draw(void *data)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;

    push_fontgroup(NOTATIONFONT);
    draw_all(ninf);
    pop_fontgroup();
}

static void notation_layout_change(void *data)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;

    if (!data) return;
    XClearWindow(display, ninf->notadrawwin);
    push_fontgroup(NOTATIONFONT);
    scrollbar_linesize(ninf->scrollver, line_height());
    make_list(ninf);
    scrollbar_set(ninf->scrollver, ninf->listpos, ninf->listsize);
    draw_all(ninf);
    pop_fontgroup();
}

static void make_notation_file(void *data, Char *name)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *)data;

    save_notation_window(ninf->fnr, name);
    if (edit_fnr == ninf->fnr)
	notadef_rename();
    changed_notation();
}

static void handle_filename(void *data, Char *name)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    Bool must_rename;

    must_rename = notadef_is_open && edit_fnr==ninf->fnr
	&& last_window(ninf->fnr);
    ninf->fnr = load_notation_window(ninf->fnr, name);
    if (must_rename) {
	edit_fnr = ninf->fnr;
	notadef_rename();
    }
    notation_filename = NULL;
    changed_notation();
}

static void rename_notation_file(void *data, Char *name)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;

    rename_notation_window(ninf->fnr, name);
    if (edit_fnr == ninf->fnr)
	notadef_rename();
    free(name);
    changed_notation();
}

/*
static void clear_notation_file(void *data, int nr)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *)data;
    if (remark_set) {
	if (!nr) {
	    if (edit_fnr == ninf->fnr)
		notadef_close();
	    ninf->fnr = clear_notation_window(ninf->fnr);
	    changed_notation();
	}
	kind_of_remark = NO_REMARK;
	remark_set = MP_False;
    } else {
	if (can_open_remark) {
	    Char *buttons[3];
	    kind_of_remark = CLEARBUTTON;
	    remark_set = MP_True;
	    sprintf(remark_text, "WARNING:\nRemoving stencils can disable\n"
		    "the LaTeX output.\nRemove stencils ?");
	    buttons[0]=translate(" Remove ");
	    buttons[1]=translate(" Cancel ");
	    buttons[2=]0;
	    remark_make(ninf->notawin, data, clear_notation_file,
			REMARK_BUTTON,	translate(remark_text),
			buttons, NULL, 0);
	}
    }
}
*/

static void notation_auto_save(void *data, int dump)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    auto_save_window(ninf->fnr, dump);
}

static Char* save_name=NULL;
static void notation_changed(void);

static void handle_done(void *data, int bnr)
{
    NOTATIONINFO *ninf = (NOTATIONINFO*) data;
    if (!bnr && save_name) {
	save_notation_window(ninf->fnr,save_name);
	notation_changed();
    } else if (bnr<2) auto_save_window(ninf->fnr, 0);
    if (bnr<2) {
	if (last_window(ninf->fnr)) {
	    if (notadef_is_open && edit_fnr == ninf->fnr) notadef_close();
	    ninf->fnr=clear_notation_window(ninf->fnr);
	}
	XDestroyWindow(display, ninf->notawin);
	notation_bad_end(data);
    }
}

static void notation_handle_button(void *data, int b_num)
{
    NOTATIONINFO *ninf = (NOTATIONINFO*) data;
    Char *c;
    Char *h;
    switch (b_num) {
    case LOADBUTTON:
	fileselc_open(handle_filename, data, translate("Load stencil:"),
		      notationdir, translate("*.mps"), NULL, ninf->notawin);
	break;
    case SAVEBUTTON:
	c = get_notation_filename(ninf->fnr);
	h = c+Ustrlen(c)-Ustrlen(translate(".mpd"));
	if (h>=c && !Ustrcmp(h, translate(".mpd"))) *h='\0';
	c = concat(c,translate(".mps"));
	if (h>=c && *h=='\0') *h='.';
	h=get_notation_dirname(ninf->fnr);
	if (!h) h=notationdir;
	fileselc_open(make_notation_file, data, translate("Save stencil:"),
		      h, translate("*.mps"), c, ninf->notawin);
	free(c);
	break;
    case RENAMEBUTTON:
	c = get_notation_filename(ninf->fnr);
	h = c+Ustrlen(c)-Ustrlen(translate(".mpd"));
	if (h>=c && !Ustrcmp(h, translate(".mpd"))) *h='\0';
	c = concat(c,translate(".mps"));
	if (h>=c && *h=='\0') *h='.';
	h=get_notation_dirname(ninf->fnr);
	if (!h) h=notationdir;
	fileselc_open(rename_notation_file, data, translate("Rename stencil:"),
		      h, translate("*.mps"),c, ninf->notawin);
	break;
    case DEFINEBUTTON:
	if (can_open_notadef) {
	    edit_fnr = ninf->fnr;
	    notadef_open();
	} else {
	    if (notadef_is_open) {
		edit_fnr = ninf->fnr;
		notadef_rename();
		notadef_raise();
	    }
	}
	break;
    case DONEBUTTON:
	if (can_close_notation) {
	    if (notation_not_saved(ninf->fnr)) {
	        Char *buttons[4];
		buttons[0]=translate(" Yes ");
		buttons[1]=translate(" No ");
		buttons[2]=translate(" Cancel ");
		buttons[3]=0;
		c = get_notation_filename(ninf->fnr);
		h = c+Ustrlen(c)-4;
		if (h>=c && !Ustrcmp(h, translate(".mpd"))) *h='\0'; else h=NULL;
		c = concat(c,translate(".mps"));
		if (h && *h=='\0') *h='.';
		h=concat(get_notation_dirname(ninf->fnr),c);
		free(c);
		save_name=h;
		remark_make(ninf->notawin,data,handle_done, REMARK_BUTTON,
			    translate(" Stencil not saved. \n Save it ? "),
			    buttons, &save_name, 1000, NULL);
		free(h);
		save_name=NULL;
	    } else {
		if (last_window(ninf->fnr)) {
		    if (notadef_is_open && edit_fnr==ninf->fnr)
			notadef_close();
		    ninf->fnr=clear_notation_window(ninf->fnr);
		}
		XDestroyWindow(display, ninf->notawin);
		notation_bad_end(data);
	    }
	}
	break;
    }
}

static void draw_notation_line(NOTATIONINFO *ninf, int line, int col)
{
    int i,j,ypos;

    if (line<(int)ninf->listpos) return;
    ypos = 0;
    i = ninf->listpos;
    while (i!=line && i<(int)ninf->listsize) {
	ypos+= ninf->list[i].height;
	i++;
    }
    if (i!=line || !ninf->list[i].mnr) return;
    set_output_window(&ninf->notadrawwin);
    set_text_mode(Normal);
    set_drawstyle(INVISIBLE);
    set_margin(2*INTERSPACE);
    set_x_y(0,ypos);
    j=0;
    while (j<ninf->list[i].mnr) {
	if (j==col) set_drawstyle(VISIBLE);
	redraw_notation(ninf, ninf->list[i].kind, ninf->list[i].anr+j);
	if (j==col) set_drawstyle(INVISIBLE);
	j++;
    }
    out_char(Newline);
    unset_output_window();
}

static void get_notation(NOTATIONINFO *ninf, int x, int y)
{
    int oldanr = anrsel, oldkind=kindsel;
    NOTATIONINFO *oldni = nisel;

    anrsel = position_detect(ninf, &x, &y);

    if (nisel!=oldni || anrsel!=oldanr || kindsel!=oldkind) {
	if (oldni) {
	    draw_notation_line(oldni, linesel, colsel);
	}
	colsel = x;
	linesel = y;
	draw_notation_line(ninf, linesel, colsel);
    }
}

static int notation_nr = 0;

static void notation_handle_popup(void* data __attribute__((unused)), int inr)
{
    if (inr>=0) {
	nnr_last = which_notation(inr)->innr;
	vnr_last = which_version_nr(inr);
	if (nnr_last>=0 && vnr_last>=0) {
	  handle_key(TemplateKey, TemplateMode);
	}
    }
}

static void dummy_function(void *data __attribute__((unused)),
			   int inr __attribute__((unused)))
{
}

extern void open_helpfile(void* data, int inr);
static void show_notation_info(void *data __attribute__((unused)), int inr)
{
    MENU *m;
    m = build_menu(translate("Template Info"));
    if (!m) return;
    add_item(m, make_info(inr),dummy_function, NULL, 0);
    popup_make(m);
}

static void unlock_stencil_wrapper(void *data)
{
  unlock_stencil((int) data);
}

Bool make_notation_popup(int nnr, int vnr, void (*func)(void*,int),
			 Char *title, Bool stick)
{

    NOTATION *nota = which_notation(nnr_vnr2innr(nnr, 0));
    int j;
    MENU *m;
    Char *cps;

    if (!nota || !nota->versions) return MP_False;

    cps = nota->name;
    if (!cps || !*cps) cps=nota->vers[0].format[NAMEFORMAT];
    if (!cps || !*cps) cps=nota->vers[0].format[SCREENFORMAT];
    m = build_menu(cps);
    if (!m) return 0;
    lock_stencil(nota->vers[0].ivnr);
    popup_set_termfunc(m, unlock_stencil_wrapper, (void*)(nota->vers[0].ivnr));
    for (j=0; j<nota->versions; j++) {
      cps=nota->vers[j].format[NAMEFORMAT];
      if (!cps || !*cps)
	cps = nota->vers[j].format[SCREENFORMAT];
      add_item(m,cps, func, NULL, nota->vers[j].ivnr);
    }
    /* add a seperator */
    add_item(m,NULL,NULL, NULL,0);
    add_item(m,translate("Info"),
	     show_notation_info, NULL, nota->vers[0].ivnr);
    if (nota->helpfilename) {
      add_item(m, translate("Help"), open_helpfile, nota->helpfilename,0);
    }
    notation_nr = nnr;
    return (popup_make(m)!=NULL);
}

static void notation_press(void *data, XButtonEvent *event)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;

    if (event->window == ninf->notadrawwin) {
	push_fontgroup(NOTATIONFONT);
	get_notation(ninf, event->x, event->y);
	pop_fontgroup();
	get_motion_hints(ninf->notadrawwin, -1);
	if (mouse_button==Button3 && nisel) {
	    nnr_last = get_notation_nr(nisel->fnr, kindsel, anrsel);
	    if (nnr_last>=0)
		make_notation_popup(nnr_last, 0, notation_handle_popup,
				    translate("Template"), MP_True);
	}
    }
}

static void notation_release(void *data, XButtonEvent *event)
{
    if (event->window == ((NOTATIONINFO *)data)->notadrawwin) {
	stop_motion_hints();
	if (nisel==NULL) return;
	nnr_last = get_notation_nr(nisel->fnr, kindsel, anrsel);
	vnr_last = 0;
	if (nnr_last<0 || mouse_button==Button3) return;
	handle_key(TemplateKey, TemplateMode);
    }
}

static void notation_motion(void *data, int x, int y)
{
    push_fontgroup(NOTATIONFONT);
    get_notation((NOTATIONINFO *) data, x, y);
    pop_fontgroup();
}

static void notation_resize(void *data, XConfigureEvent *event)
{
    int new_width, new_height;
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;

    ninf->win_xpos = last_xpos = event->x;
    ninf->win_ypos = last_ypos = event->y;
    if (((int)ninf->win_width == event->width) &&
	((int)ninf->win_height == event->height))
	return;
    ninf->win_width = last_width = event->width; 
    new_width  = sub_width( event->width );
    ninf->win_height = last_height = event->height;
    new_height = sub_height(event->height );
    XResizeWindow(display, ninf->notadrawwin, new_width-2, new_height-2);
    push_fontgroup(NOTATIONFONT);
    make_list(ninf);
    scrollbar_resize(ninf->scrollver, new_height);
    scrollbar_set(ninf->scrollver, ninf->listpos, ninf->listsize);
    draw_all(ninf);
    pop_fontgroup();
}

static void notation_scrollto(void *data, int kind __attribute__((unused)))
{
    unsigned int i;
    NOTATIONINFO *ninf = (NOTATIONINFO *)data;
    
    i = scrollbar_line(ninf->scrollver, 0);
    if (i!=ninf->listpos) {
	ninf->listpos = i;
	push_fontgroup(NOTATIONFONT);
	draw_all(ninf);
	pop_fontgroup();
    }
}

static void notation_changed(void)
{
    int i;
    void *data;
    NOTATIONINFO *ninf;
    
    i=0;
    while (aig(data = next_data_with_type(MAINNOTATIONWINDOW, &i))) {
	push_fontgroup(NOTATIONFONT);
	ninf = (NOTATIONINFO *) data;
	make_list(ninf);
	scrollbar_set(ninf->scrollver, ninf->listpos, ninf->listsize);
	set_name(ninf);
	draw_all(ninf);
	pop_fontgroup();
	i++;
    }
}

static void redraw_move(int newanrsel)
{
    int oldkindsel=kindsel;
    NOTATIONINFO *oldsel = nisel;

    changed_notation();
    push_fontgroup(NOTATIONFONT);
    get_position(oldsel, oldkindsel, newanrsel, &linesel, &colsel);
    nisel = oldsel;
    kindsel = oldkindsel;
    anrsel = newanrsel;
    draw_notation_line(nisel, linesel, colsel);
    pop_fontgroup();
}

void notation_move_begin(void)
{
    int newanrsel = anrsel;

    if (nisel) {
	while (move_nota_left(nisel->fnr, kindsel,newanrsel))
	    newanrsel--;
	if (newanrsel != anrsel) redraw_move(newanrsel);
    }    
}

void notation_move_end(void)
{
    int newanrsel = anrsel;

    if (nisel) {
	while (move_nota_right(nisel->fnr, kindsel,newanrsel))
	    newanrsel++;
	if (newanrsel != anrsel) redraw_move(newanrsel);
    }
}

void notation_move_left(void)
{
    int newanrsel = anrsel;

    if (nisel) {
        if (move_nota_left(nisel->fnr, kindsel,newanrsel)) newanrsel--;
	if (newanrsel != anrsel) redraw_move(newanrsel);
    }
}

void notation_move_right(void)
{
    int newanrsel = anrsel;

    if (nisel) {
        if (move_nota_right(nisel->fnr, kindsel,newanrsel)) newanrsel++;
	if (newanrsel != anrsel) redraw_move(newanrsel);
    }
}

int notation_last(void)
{
    if (nnr_last>=0) return nnr_vnr2innr(nnr_last, vnr_last);
    return -1;
}

int notation_version(void)
{
    return vnr_last;
}

static void notation_iconize(void *data)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    
    if (!ninf->is_icon) {
	ninf->is_icon = MP_True;
	number_icon++;
	notation_iconized= (number_icon == number_open);
    }
}

static void notation_deiconize(void *data)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    
    if (ninf->is_icon) {
	ninf->is_icon=MP_False;
	number_icon--;
	notation_iconized=MP_False;
    }
}

static void notation_state(void *data, int *x, int*y, int *w, int *h,
		    int *i, int *s, Char **str)
{
    NOTATIONINFO *ninf = (NOTATIONINFO *) data;
    int xm,ym;

    window_manager_added(ninf->notawin, &xm, &ym);
    *x = ninf->win_xpos-xm;
    *y = ninf->win_ypos-ym;
    *w = ninf->win_width;
    *h = ninf->win_height;
    *i = ninf->is_icon;
    *s = scrollbar_line(ninf->scrollver, 0);
    *str = get_notation_filename(ninf->fnr);
}

static void notation_use_state(int x, int y, int w, int h,
			int i, int s, Char *str)
{
    Char *c;
    Bool opened;

    as_icon = i;
    state_open = MP_True;
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
    i = -1;
    while ((i=get_next_filename(i, &c, &opened))>=0 && Ustrcmp(c,str));
    use_file_nr = i;
    notation_open();
    use_file_nr = -1;
    if (state_window) {
	scrollbar_set(state_window->scrollver, s, state_window->listsize);
	state_window->listpos = s;
    }
    state_open = MP_False;
    as_icon = MP_False;
    free(str);
}

static int notation_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return is_opened;
}

static void notation_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

FUNCTIONS mainnotationfuncs = {
    notation_bad_end, NULL, notation_resize, NULL,
    NULL, NULL, notation_iconize, notation_deiconize,
    NULL, NULL, notation_layout_change, notation_auto_save, notation_use_state,
    notation_state, NULL, NULL, notation_last_pos, notation_set_last_pos };

FUNCTIONS notationfuncs = {
    NULL, notation_draw, NULL, notation_press,
    notation_release, notation_motion, NULL, NULL,
    NULL, NULL, NULL, NULL, notation_file_use_state };

void notation_init(void)
{
    int i,lw,lh;
    
    notation_mask =
	(CWBackPixel | CWBorderPixel | CWBitGravity |
	 CWColormap | CWEventMask);
    
    notation_attr.background_pixel = white_pixel;
    notation_attr.border_pixel = black_pixel;
    notation_attr.colormap = colormap;
    notation_attr.bit_gravity = NorthWestGravity;
    notation_attr.event_mask = (ExposureMask
				| ButtonPressMask | ButtonReleaseMask
				| ButtonMotionMask | PointerMotionHintMask
				| KeyPressMask | StructureNotifyMask
				| VisibilityChangeMask);
    lw = 0;
    for (i=0; i<NR_BUTTON; i++)
	lw += button_width(translate(notationbutton[i]))+BINTERSPACE;
    lh = 8*lw/5;
    if (!last_width) {
	last_width = lw;
	last_height = lh;
	last_xpos = (display_width-lw)/2;
	last_ypos = (display_height-lh)/2;
    }
    for (i=0; i<MAX_KIND; i++) {
      kinddes[i] = kind_description(i);
    }
    set_change_function(notation_changed);
    remark_text = (char *) malloc( NAME_LENGTH *2);
    if (!XStringListToTextProperty(&notation_name, 1, &notaname))
	message(MP_EXIT-1, translate("No memory for the stencil windowname."));
    if (!XStringListToTextProperty(&icon_name, 1, &iconname))
    message(MP_EXIT-1, translate("No memory for the stencil iconname."));
}

void notation_open(void)
{
    int x = BINTERSPACE/2;
    int y = BINTERSPACE/2;
    int i;
    XSizeHints size_hints;
    NOTATIONINFO *ninf;

    state_window = NULL;
    if (!(ninf= (NOTATIONINFO *) malloc (sizeof(NOTATIONINFO)))) {
	message(MP_ERROR, translate("No memory for stencil window."));
	return;
    }
    ninf->win_width = last_width;
    ninf->win_height = last_height;
    if (!state_open)
	if (!last_xpos) {
	    last_xpos = (display_width - last_width)/2;
	    last_ypos = (display_height - last_height)/2;
	}
    ninf->win_xpos = last_xpos;
    ninf->win_ypos = last_ypos;
    ninf->list = NULL;
    ninf->is_icon = MP_True;
    ninf->name = NULL;
    ninf->subname = NULL;
    ninf->listsize = ninf->maxlist = ninf->listpos = 0;
    ninf->notawin = XCreateWindow(display, root_window,
				  last_xpos, last_ypos,
				  ninf->win_width, ninf->win_height,
				  BORDERWIDTH, CopyFromParent, InputOutput,
				  visual,
				  notation_mask, &notation_attr);
    if (state_open)
	size_hints.flags = USPosition | USSize | PMinSize;
    else
	size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width =
	size_hints.min_height = pos_y_with + SCROLLBARSIZE*3;
    wm_hints.initial_state = (iconic || as_icon ? IconicState : NormalState);

    XSetWMProperties(display, ninf->notawin, &notaname, &iconname,
		     NULL, 0, &size_hints, &wm_hints, &class_hints);
    set_protocols(ninf->notawin);
    i=0;
    if (add_window(ninf->notawin, MAINNOTATIONWINDOW,
		   root_window, (void*) ninf, translate(helpname[STENCILHELP]))) {
	while (i<NR_BUTTON &&
	       button_make(i, ninf->notawin, translate(notationbutton[i]), &x, y, 1,
			   (void*) ninf, helpname[notahelp[i]],
			   NULL, NULL, NULL,
			   notation_handle_button, NULL, NULL))
	    i++,x+=BINTERSPACE;
	x = sub_width(ninf->win_width);
	y = sub_height(ninf->win_height);
	if (i==NR_BUTTON) {
	    ninf->notadrawwin = XCreateWindow(display, ninf->notawin,
					      pos_x_with, pos_y_with, x-2, y-2,
					      1, CopyFromParent, InputOutput,
					      visual,
					      notation_mask, &notation_attr);
	    if (add_window(ninf->notadrawwin, NOTATIONWINDOW,
			   ninf->notawin, NULL, translate(helpname[STENCILHELP])))
		i++;
	}
	push_fontgroup(NOTATIONFONT);
	if (i==NR_BUTTON+1 &&
	    (ninf->scrollver = scrollbar_make(VERTICAL, ninf->notawin,
					      pos_x_without, pos_y_with, y,
					      line_height(), notation_scrollto,
					      (void*) ninf)))
	    i++;
	pop_fontgroup();
    }
    if (i<NR_BUTTON+2) {
	XDestroyWindow(display, ninf->notawin);
	destroy_window(ninf->notawin);
    } else {
	is_opened = MP_True;
	ninf->fnr = new_notation_window();
	make_list(ninf);
	set_name(ninf);
	scrollbar_set(ninf->scrollver, ninf->listpos, ninf->listsize);
	if (!number_open++) notation_is_open = MP_True;
	number_icon++;
	state_window = ninf;
	XMapSubwindows(display, ninf->notawin);
	XMapWindow(display, ninf->notawin);
    }
}


static int save_nr = 0;
static Char *save_file=NULL;

static void handle_confirm(void *data __attribute__((unused)), int i)
{
    if (!i) {
	Char *c = concat(save_file, translate(".mps"));
	save_notation_window(save_nr, c);
	free(c);
	free(save_file);
	save_file = NULL;
    } else if (i==1) {
	auto_save_window(save_nr, 0);
	saved_notation_file(save_nr);
    }
    if (i<2) menu_close();
}

void notation_confirm_backup(int i)
{
    Char *buttons[4];
    buttons[0]=translate(" Yes ");
    buttons[1]=translate(" No ");
    buttons[2]=translate(" Cancel ");
    buttons[3]=0;
    save_nr = i;
    save_file = concat(get_notation_dirname(i),
		       get_notation_filename(i));
    remark_make(0, 0, handle_confirm, REMARK_BUTTON,
		translate("Stencilfile not saved. Save it?"),
		buttons, &save_file, 1000, NULL);
    free(save_file);
    save_file = NULL;
}
