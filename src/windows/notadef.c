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
**   File : notadef.c
**   Datum: 14-5-92
**   Doel : Het definieren van notaties (toevoegen, verwijderen,
**          verschillende versies)
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
/* #include "keymap.h" */
#include "button.h"
#include "symbol.h"
#include "output.h"
#include "latexout.h"
#include "scrollbar.h"
#include "message.h"
#include "notatype.h"
#include "notadef.h"
#include "notation.h"
#include "popup.h"
#include "helpfile.h"

/* from editor.h */
extern int ss_notation(int *nr);
extern int ss_notation_size(void);
extern int ss_notation_recursive(Char *buffer, int size);

#define NOTADEFNAME  "Define "
#define ICONNAME     "Define"
#define MAX_VERSION    100
#define NAME_SIZE      100
#define CU_REM           8

enum button { INSTALLBUTTON, MODIFYBUTTON, DELETEBUTTON,
              DOUBLEBUTTON, DONEBUTTON, NR_BUTTON };

static
char *notadefbutton[NR_BUTTON] = { "New", "Update", "Remove",
					 "Double", "Done" };
static
int notahelp[NR_BUTTON] =
{ DEFINENEWHELP, DEFINEUPDATEHELP, DEFINEREMOVEHELP,
  DEFINEDOUBLEHELP, DEFINEDONEHELP };

#define NAMEDESC   "Name"
#define HELPNAMEDESC "Help file"
#define SCREENDESC "Screen"
#define LATEXDESC  "Output"
#define INPUT_OK(A)  if (notadef_is_open && !notadef_iconized && (A))

#define PRECPOS  1
#define SPACEPOS 2
#define KINDPOS  0
#define PLACEPOS 3
#define LATEXPOS 4
#define FONTPOS  5
#define SIZEPOS  6
#define TABPOS   7
#define MISCPOS  8
#define MAX_ITEMLIST  9
#define MAX_SPACING 11
#define BUFSIZE  1000

typedef
struct { char *description;
         int selection, nr_items, def, x, max, helpnr;
         Bool stay;
	 void *buttondata;
         MENU *menu;
       } ITEMLIST;

ITEMLIST itemlist[MAX_ITEMLIST] =
{ { "Kind",   0, MAX_KIND,       0, 0,0, DEFINEKINDHELP,  MP_True,  NULL },
  { "Prec",   0, MAX_PRECEDENCE, 0, 0,0, DEFINEPRECHELP,  MP_True,  NULL },
  { "Space",  0, MAX_SPACING,    0, 0,0, DEFINESPACEHELP, MP_True,  NULL },
  { "Place", -1, 6,              0, 0,0, DEFINEPLACEHELP, MP_False, NULL },
  { "Mode",  -1, 4,              0, 0,0, DEFINEMODEHELP,  MP_False, NULL }, 
  { "Font",  -1, 0,              0, 0,0, DEFINEFONTHELP,  MP_False, NULL },
  { "Size",  -1, 11,             5, 0,0, DEFINESIZEHELP,  MP_False, NULL },
  { "Tabs",  -1, 10,             0, 0,0, DEFINETABSHELP,  MP_False, NULL },
  { "Special",-1,7,              0, 0,0, DEFINEMISCHELP,  MP_False, NULL }
};

static Window notadefwin;

static unsigned long notadef_mask;
static XSetWindowAttributes notadef_attr;
static Char *notadef_name;
static char *icon_name = ICONNAME;
static XTextProperty notadefname, iconname;
static int win_xpos, win_ypos, win_width, win_height;
static VERSION *editversion = NULL;
static void *scrollver = NULL;
static int editnotation=-1, afternotation=-1;
static unsigned long unique_number = 0;
static int xpos, ypos, xposversion, xposname, yposname;
static int yposhelpname;
static int editnr = 0;
static int verscurs=-1, formatcurs=0, poscurs = 0;
static int posstart=-1, possel=0;
static Char notname[NAME_SIZE], nothelpname[NAME_SIZE];
static Bool showtabs = MP_True;
static int lines[MAX_VERSION+3];
static int nr_lines[MAX_VERSION][MAXFORMAT];
static int *heights[MAX_VERSION][MAXFORMAT];
static int drawstyle=VISIBLE;
static int invis = MP_False;
static int detect_line = -1;
static int detect_pos = 0;

#define pop_return { pop_fontgroup();return; }


static void ltx_initfunc(void)
{ tex_set_file(NULL); tex_open_proof(); }
static void scr_nwlnfunc(Char j)
{ if (showtabs) display_tab(j); }
static void scr_ntnlfunc(Char j)
{ if (Ph(j)==MP_Text) out_text_delim(MP_True);
  if (!showtabs || !display_tab(j)) out_char(j);
  if (Ph(j)==MP_Text) out_text_delim(MP_False);
}
static void ltx_finifunc(void)
{ pop_math_pref(); tex_close_proof(); tex_unset(); }

static void (*initfunc[MAXFORMAT])(void) =
        { open_tabbing, ltx_initfunc };
static void (*nwlnfunc[MAXFORMAT])(Char) =
        { scr_nwlnfunc, out_latex_char };
static void (*ntnlfunc[MAXFORMAT])(Char) =
        { scr_ntnlfunc, out_latex_char, out_char };
static void (*finifunc[MAXFORMAT])(void) =
        { close_tabbing, ltx_finifunc };

static Char lmodecode[4]= {0, AskText, AskMath, AskBoth};
static Bool lmodepref[4]= {MP_False, MP_False, MP_True, MP_False};

static int cursor_x=0, cursor_y=0, cursor_target=0;

static void draw_format(int vnr, int fmnr, char *desc,
			Char *form, Bool cursor)
{
    int i,lnr,y;
    Char j;

    i=0;
    lnr=0;
    push_fontgroup(EDITFONT);
    set_output_window(&notadefwin);
    set_text_mode(Normal);
    set_margin(0);
    set_index(MP_True);
    y = lines[vnr+2]+line_height()/2;
    while (i<fmnr) y+= heights[vnr][i++][0];
    set_x_y(0, y);
    set_drawstyle(drawstyle);
    if (y<lines[1] || invis) set_drawstyle(INVISIBLE);
    thinspace(xpos+SCROLLBARSIZE+INTERSPACE+2);
    out_string(translate(desc));
    thinspace(xposversion-where_x());
    if (initfunc[fmnr]) initfunc[fmnr]();
    out_char(Settab);
    out_char(Tabplus);
    if (fmnr==LATEXFORMAT) {
	display_tab(lmodecode[editversion[vnr].latexmode]);
	push_math_pref(lmodepref[editversion[vnr].latexmode]);
    }
    i=0;
    while (aig(j = form[i])) {
	if (cursor)
	    if (poscurs==i || possel==i)
		if (poscurs==i && possel==i) {
		    if (where_y()>lines[1]) {
		      out_cursor(CURSOR);
		      cursor_x=where_x();
		      cursor_y=where_y();
		    }
		} else {
		  switch_reverse();
		  cursor_x=where_x();
		  cursor_y=where_y();
		}
	if (IsNewline(j)) {
	    if (nwlnfunc[fmnr]) nwlnfunc[fmnr](j);
	    out_char(j);
	    heights[vnr][fmnr][1+lnr] = where_y()-y;
	    y = where_y();
	    if (y>=lines[1] && !invis) set_drawstyle(drawstyle);
	    if (detect_line==lnr) {
		detect_pos=i;
		detect_line=-1;
	    }
	    lnr++;
	    if (where_x()<xposversion) thinspace(xposversion-where_x());
	} else
	    if (ntnlfunc[fmnr]) ntnlfunc[fmnr](j);
	if (detect_line==lnr && where_x()>detect_pos) {
	    detect_pos = i;
	    detect_line=-1;
	}
	i++;
    }
    if (detect_line==lnr) { detect_pos=i; detect_line=-1; }
    if (cursor && where_y()>lines[1]) {
      if (poscurs==i || possel==i) {
	if (poscurs==i && possel==i)
	  out_cursor(CURSOR);
	else
	  switch_reverse();
	cursor_x=where_x();
	cursor_y=where_y();
      }
    }
    if (finifunc[fmnr]) finifunc[fmnr]();
    out_char(Newline);
    heights[vnr][fmnr][1+lnr] = where_y()-y;
    heights[vnr][fmnr][2+lnr] = heights[vnr][fmnr][0] = 0;
    for (i=0; i<=lnr; i++) heights[vnr][fmnr][0] += heights[vnr][fmnr][1+i];
    unset_output_window();
    pop_fontgroup();
}

static void draw_format_args(int vnr, int fmnr)
{
    Char *fm;
    Char b[1];
    Bool dc;
    char *frm;

    b[0]=0;
    dc = (verscurs == vnr && formatcurs==fmnr && invis <=MP_True);
    fm = editversion[vnr].format[fmnr];
    if (!fm) {
	if (fmnr==LATEXFORMAT) {
	    dc = dc || (verscurs == vnr && formatcurs==SCREENFORMAT && invis <=MP_True);
	    fm = editversion[vnr].format[SCREENFORMAT];
	} else fm=b;
    }
    switch (fmnr) {
    case SCREENFORMAT: frm = SCREENDESC; break;
    case LATEXFORMAT:  frm = LATEXDESC;  break;
    case NAMEFORMAT:   frm = NAMEDESC;   break;
    default:           frm = "";         break;                  
    }
    draw_format(vnr, fmnr, frm, fm, dc);
}

#define draw_screen(A) draw_format_args(A,SCREENFORMAT)
#define draw_latex(A) draw_format_args(A,LATEXFORMAT)
#define draw_name(A)  draw_format_args(A,NAMEFORMAT)

static void draw_notaname_args(int y, Char *desc,
			       Char *name, int fmnr)
{
    int i,c;

    push_fontgroup(POPUPFONT);
    set_output_window(&notadefwin);
    set_text_mode(Normal);
    set_x_y(xpos, y);
    /* if (invis) set_drawstyle(SHADES); */
    out_string(desc);
    thinspace(xposname-where_x());
    c= verscurs==-1 && formatcurs==fmnr;
    for (i=0; name[i]; i++) {
	if (c && poscurs==i) out_cursor(CURSOR);
	out_char(name[i]);
    }
    if (c && poscurs==i) out_cursor(CURSOR);
    out_char(Newline);
    unset_output_window();
    pop_fontgroup();
}

#define draw_notaname() draw_notaname_args(yposname,translate(NAMEDESC),notname,0)
#define draw_notahelpname() draw_notaname_args(yposhelpname,translate(HELPNAMEDESC),\
					       nothelpname,1)

static void swap_cursor_off(void)
{
    poscurs= -1;possel=-1;
    if (verscurs == -1) {
        invis=MP_True;
	if (!formatcurs)
	    draw_notaname();
	else
	    draw_notahelpname();
    } else {
        invis=MP_False;
	drawstyle=VISIBLE;
	if (formatcurs == SCREENFORMAT &&
	    !editversion[verscurs].format[LATEXFORMAT])
		draw_format_args(verscurs, LATEXFORMAT);
	draw_format_args(verscurs, formatcurs);
	drawstyle=VISIBLE;
    }
    invis=MP_False;
}

static void swap_cursor_on(void)
{
    if (verscurs == -1) {
        invis=MP_True;
	if (!formatcurs)
	    draw_notaname();
	else
	    draw_notahelpname();
    } else {
        invis=MP_False;
	drawstyle=VISIBLE;
	if (formatcurs == SCREENFORMAT &&
	    !editversion[verscurs].format[LATEXFORMAT])
		draw_format_args(verscurs, LATEXFORMAT);
	draw_format_args(verscurs, formatcurs);
	drawstyle=VISIBLE;
    }
    invis=MP_False;
}

static void make_selection(int vnr, int fmnr, int pos1, int pos2);

static void detect_format_pos(int vnr, int fmnr, int x, int y)
{
    int i, yt;

    if (y<lines[1]) return;
    push_fontgroup(EDITFONT);
    yt=lines[vnr+2]+line_height()/2;
    pop_fontgroup();
    i=0;
    while (i<fmnr) yt += heights[vnr][i++][0];
    if (y<yt || y>yt+heights[vnr][i][0]) return;
    i = 0;
    while (i<nr_lines[vnr][fmnr] && y>yt) {
	yt+=heights[vnr][fmnr][1+i];
	i++;
    }
    i--;
    swap_cursor_off();
    if (verscurs !=vnr || formatcurs!=fmnr) posstart=-1;
    formatcurs=0;
    verscurs=-1;
    poscurs=possel=0;
    detect_line = i;
    detect_pos = x;
    invis=MP_True;
    draw_format_args(vnr, fmnr);
    if (posstart>=0) {
	make_selection(vnr, fmnr, posstart, detect_pos);
    } else
	posstart=poscurs=possel=detect_pos;
    formatcurs = fmnr;
    verscurs = vnr;
    detect_line = -1;
    detect_pos=0;
    invis=MP_False;
    swap_cursor_on();
}

#define detect_latex_pos(A,B,C) detect_format_pos(A,LATEXFORMAT,B,C)
#define detect_name_pos(A,B,C)  detect_format_pos(A,NAMEFORMAT,B,C)

static void adjust_scrollbar(void)
{
    if (scrollver) {
	int i,j,h,k,d,l,n;
	push_fontgroup(EDITFONT);
	n = scrollbar_line(scrollver, 0);
	h=0;
	for (i=0; i<editnr; i++) {
	    h+=1;
	    for (j=0; j<MAXFORMAT; j++) h+= nr_lines[i][j];
	}
	if (n>=h) n = (h ? h-1 : 0);
	lines[2] = lines[1];
	k=0;d=0;
	for (i=0; i<editnr; i++) {
	    lines[i+3]=lines[i+2];
	    for (j=0; j<MAXFORMAT; j++) {
		for (l=0; l<nr_lines[i][j]; l++) {
		    lines[i+3]+=heights[i][j][l+1];
		    k++;
		    if (k==n) d = lines[i+3]-lines[1];
		}
	    }
	    lines[i+3]+= line_height();
	    k++;
	    if (k==n) d = lines[i+3]-lines[1];
	}
	if (d)
	    for (i=0; i<=editnr; i++)
		lines[i+2]-=d;
	pop_fontgroup();
	scrollbar_set(scrollver, n, h);
    } else {
	int i,j;
	push_fontgroup(EDITFONT);
	lines[2] = lines[1];
	for (i=0; i<editnr; i++) {
	    lines[i+3] = lines[i+2]+line_height();
	    for (j=0; j<MAXFORMAT; j++) lines[i+3]+= heights[i][j][0];
	}
	pop_fontgroup();
    }
}

static void reset_lines(void)
{
    push_fontgroup(POPUPFONT);
    lines[1]= yposhelpname + line_height() + INTERSPACE;
    pop_fontgroup();
    push_fontgroup(EDITFONT);
    adjust_scrollbar();
    pop_fontgroup();
}

static int positioncode(int y)
{
    /*   -2   : positie tussen buttons
    **   -1   : in blok met precedence, kind, name, ...
    **    0   : blok met versie 0
    **   ..   : blok met versie ..
    ** editnr : leeg blok na laatste versie
    */

    int i=0;

    if (y<lines[0]) return -2;
    if (y<lines[1]) return -1;
    while ( i<editnr && lines[i+3]<lines[1]) i++;
    while (i<editnr && y>lines[i+3]) i++;
    return i;
}

static void get_name_args(int x, int y, int yp, Char *name, int fmnr)
{
    int i,j,k;

    push_fontgroup(POPUPFONT);
    i = xposname-CU_REM;
    if (x>=i && y>=yp && y<=yp+line_height()) {
	swap_cursor_off();
	i = xposname;
	j = 0;
	while ((k=(i+char_width(name[j])))<x &&
	       name[j]) {
	    i=k;
	    j++;
	}
	verscurs=-1; formatcurs=fmnr;
	poscurs = j;
	swap_cursor_on();
    }
    pop_fontgroup();
}

#define get_name(A,B) get_name_args(A,B,yposname,notname,0)
#define get_helpname(A,B) get_name_args(A,B, yposhelpname, nothelpname, 1)

#define draw_line(A) XDrawLine(display,notadefwin,get_GC(Normal,0,0),\
			       INTERSPACE, lines[A],win_width,lines[A])

static void clear_bottom(void)
{
    if (lines[editnr+2]<win_height)
	XClearArea(display,notadefwin, 0,lines[editnr+2]+1, 0, 0, MP_False);
}

static void draw_itemlist(void)
{
    int i;

    push_fontgroup(POPUPFONT);
    set_output_window(&notadefwin);
    set_text_mode(Normal);
    set_margin(xpos);
    set_x_y(xpos, ypos);
    for (i=0; i<MAX_ITEMLIST; i++) {
	if (itemlist[i].stay) {
	    PopupLine *pl = itemlist[i].menu->menu->firstline;
	    int n = itemlist[i].selection;
	    while (pl && n) {
	      pl=pl->next;
	      n--;
	    }
	    thinspace(itemlist[i].x-where_x());
	    out_string(pl->string);
	    thinspace(itemlist[i].max+where_x()-itemlist[i].x);
	}
    }
    out_char(Newline);
    unset_output_window();
    pop_fontgroup();
}


static void draw_version(int nr)
{
    int ypost,width,i;

    push_fontgroup(EDITFONT);
    if (lines[nr+2]<lines[1]) ypost = lines[1]+1; else ypost = lines[nr+2]+1;
    if (lines[nr+3]<lines[1]) width = 0; else width = lines[nr+3]-ypost;
    if (width>0) {
	XClearArea(display, notadefwin, 0, ypost, xposversion, width, MP_False);
	XClearArea(display, notadefwin, 0, lines[nr+3]-line_height()/2-
		   line_height()%2, win_width, line_height(), MP_False);
    }
    if (lines[nr+2]>=lines[1])
	draw_line(nr+2);
    if (lines[nr+3]>=lines[1])
	draw_line(nr+3);
    for (i=MAXFORMAT-1; i>=0; i--)
	draw_format_args(nr,i);
    pop_fontgroup();
}

static void move_version(int i, int y)
{
    if (y)  {
	int ib=lines[i+2]+y;

	if (ib<lines[2]) ib = lines[2];
	XClearArea(display, notadefwin, 0, ib+1, 0, 0, MP_False);
	ib =i;
	for (; i<=editnr; i++)
	    lines[i+2]+=y;
	for (; ib<editnr; ib++)
	    draw_version(ib);
    }
}

static void draw_data(void)
{
    draw_notaname();
    draw_notahelpname();
    draw_line(0);
    draw_line(1);
    draw_itemlist();
}

static void draw_all(void)
{
    int i;
    
    draw_data();
    for (i=0; i<editnr; i++)
	draw_version(i);
}

static void set_nota(NOTATION *nota)
{
    nota->vers = minimize_version(editversion, editnr);
    nota->prec = itemlist[PRECPOS].selection;
    nota->kind = (Opkind) itemlist[KINDPOS].selection;
    nota->space = itemlist[SPACEPOS].selection;
    nota->nnr = unique_number;
    nota->locks = nota->fillocks = 0;
    nota->versions = editnr;
    if (notname && notname[0])
	nota->name = Ustrdup(notname);
    else
	nota->name = NULL;
    if (nothelpname && nothelpname[0])
	nota->helpfilename = Ustrdup(nothelpname);
    else
	nota->helpfilename = NULL;
}

static void get_nota(NOTATION *nota)
{
    editversion = maximize_version( nota->vers, nota->versions);
    editnr = nota->versions;
    itemlist[PRECPOS].selection = nota->prec;
    itemlist[KINDPOS].selection = nota->kind;
    itemlist[SPACEPOS].selection = nota->space;
    unique_number = nota->nnr;
    if (nota->name)
	Ustrncpy(notname, nota->name, NAME_SIZE);
    else
	notname[0] = '\0';
    if (nota->helpfilename)
	Ustrncpy(nothelpname, nota->helpfilename, NAME_SIZE);
    else
	nothelpname[0] = '\0';
}

static void new_editnotation(void)
{
    int i,j;

    itemlist[PRECPOS].selection = notation_precedence;
    itemlist[SPACEPOS].selection = 0;
    itemlist[KINDPOS].selection = 0;
    notname[0] = '\0';
    nothelpname[0] = '\0';
    verscurs = -1; formatcurs=0;
    poscurs = 0;
    destroy_version(editversion, editnr);
    for (i=0; i<editnr; i++) {
	free(heights[i][0]);
	for (j=0; j<MAXFORMAT; j++) heights[i][j]=NULL;
    }
    editversion = NULL;
    editnotation = -1;
    afternotation = -1;
    unique_number = new_number();
    editnr = 0;
}

static void notadef_iconize(void *data __attribute__((unused)))
{
    notadef_iconized = MP_True;
    popup_unmap(notadefwin);
}

static void notadef_deiconize(void *data __attribute__((unused)))
{
    notadef_iconized = MP_False;
    popup_map(notadefwin);
}

static void notadef_bad_end(void *data __attribute__((unused)))
{
    notadef_is_open = MP_False;
    edit_fnr = 0;
    new_editnotation();
    popup_remove(notadefwin);
    destroy_window(notadefwin);
}

static void notadef_draw(void *data __attribute__((unused)))
{
    draw_all();
}

static void new_cursor_pos(int nvers, int nformat, int npos)
{
    Char *h=NULL;
    int i;
    
    if (nvers<-1)
	verscurs = -1;
    else
	verscurs = nvers;
    formatcurs = nformat;
    if (npos<0) npos=0;
    if (verscurs==-1) {
	i = (!formatcurs? Ustrlen(notname):Ustrlen(nothelpname));
	poscurs = (npos>i? i: npos);
    } else {
	if (formatcurs >= MAXFORMAT) formatcurs = MAXFORMAT-1;
	h = editversion[verscurs].format[formatcurs];
	if (!h && formatcurs==LATEXFORMAT) {
	    h = editversion[verscurs].format[SCREENFORMAT];
	}
	i = (h ? Ustrlen(h) : 0);
	poscurs = possel = (npos>i ? i :  npos);
    }
    if (verscurs>=0 && scrollver &&
	(lines[verscurs+2]<lines[1] || lines[verscurs+3]>win_height)) {

	int ypos_curs= lines[verscurs+2];
	int j=0,k=0;

	while (j<formatcurs) {
	    ypos_curs += heights[verscurs][ j][0];
	    k += nr_lines[verscurs][j];
	    j++;
	}
	for (i=0,j=0; h && *h && i<poscurs; i++,h++) {
	    if (IsNewline(*h)) {
		ypos_curs+=heights[verscurs][formatcurs][1+j];
		j++;
		k++;
	    }
	}
	i=0;
	if (ypos_curs<lines[1] ||
	    ypos_curs+heights[verscurs][formatcurs][1+j]>win_height) {
	    int whc=0;
	    int l=0;
	    for (i=0; i<verscurs; i++) {
		whc++;
		for (l=0; l<MAXFORMAT; l++)
		    whc+=nr_lines[i][l];
	    }
	    k=whc;
	    whc+=j;
	    for (i=verscurs; i<editnr; i++) {
		k++;
		for (l=0; l<MAXFORMAT; l++)
		    k+=nr_lines[i][l];
	    }
	    scrollbar_set(scrollver,whc,k);
	    reset_lines();
	    XClearArea(display,notadefwin,0,lines[editnr+2],0,0,MP_False);
	    for (i=0; i<editnr; i++) draw_version(i);
	    /* swap_cursor_off(); */
	}
    }
}

static void notadef_layout_change(void *data __attribute__((unused)))
{
    int i,j,m;
    int interitem;

    xposversion = xposname = 0;
    push_fontgroup(POPUPFONT);
    interitem = (char_width(' ')*3);
    if ((j=string_width(translate(NAMEDESC) , -1)) > xposname) xposname = j;
    if ((j=string_width(translate(HELPNAMEDESC) , -1)) > xposname) xposname = j;
    xposname += interitem*2;
    set_output_window(test_window());
    set_drawstyle(INVISIBLE);
    for (i=0; i<MAX_ITEMLIST; i++) {
        PopupLine *pl;
	itemlist[i].max = 0;
	pl=itemlist[i].menu->menu->firstline;
	while (pl) {
	    set_x_y(0,0);
	    if (pl->string) out_string(pl->string);
	    m = where_x();
	    flush();
	    out_char(Newline);
	    if (m>itemlist[i].max) itemlist[i].max = m;
	    pl=pl->next;
	}
	if (!i) itemlist[i].x=xpos;
	else {
	    int bnw=button_width(translate(itemlist[i-1].description));
	    itemlist[i].x = itemlist[i-1].x+BINTERSPACE+
		(((!itemlist[i-1].stay) || bnw>itemlist[i-1].max)?bnw:itemlist[i-1].max);
	}
    }
    unset_output_window();
    yposname = ypos+line_height()+INTERSPACE*3;
    yposhelpname = yposname + line_height()+INTERSPACE;
    pop_fontgroup();
    push_fontgroup(EDITFONT);
    xposversion = string_width(translate(NAMEDESC), -1);
    if ((j=string_width(translate(LATEXDESC), -1)) > xposversion)
	xposversion = j;
    if ((j=string_width(translate(SCREENDESC), -1)) > xposversion)
	xposversion = j;
    xposversion += interitem*3 + SCROLLBARSIZE;
    reset_lines();
    if (notadef_is_open) {
	scrollbar_move(scrollver, INTERSPACE, lines[1]);
	scrollbar_resize(scrollver, win_height-lines[1]-INTERSPACE);
	scrollbar_linesize(scrollver, line_height());
	for (i=0; i<MAX_ITEMLIST; i++) {
	    button_move(itemlist[i].buttondata, itemlist[i].x,
			lines[0]+INTERSPACE);
	}
	if (!notadef_iconized)
	    XClearArea(display, notadefwin, 0,0,0,0,MP_True);
    }
    pop_fontgroup();
}

static void move_cursor_to(int nvers, int nformat, int npos)
{
    swap_cursor_off();
    new_cursor_pos(nvers, nformat, npos);
    swap_cursor_on();
}

static int bad_notation(void)
{
    Bool visible;                     /* notatie is zichtbaar         */
    Bool phusedfirst[5][16];          /* welke worden gebruikt in
					 eerste schermversie          */
    Bool phused[5][16];               /* gebruikt in controle versie  */
    Bool phcheckused[5][16];          /* gebruikt in te testen versie */
    int  phcountfirst[6],
	 phcount[6], phcheckcount[6]; /* tel aantal, ..[5]= totaal    */
    int  i,j,m,mi;
    Bool doubles=MP_False;
    int  newvers, newformat, newpos;
    int  warnpos=0, warnvers=-1, warnformat=-1;
    Char *check;
    
    /*
    **  Het resultaat geeft een indicatie over de vorm van de fout,
    **  zodat een help-tekst mogelijk is.
    **  Als er een fout optreedt, wordt de cursor gezet op de plaats
    **  waar de fout zit.
    **
    */

    if (editnr==0) {
	message(MP_ERROR, translate("No empty stencils allowed."));
	return 1;
    }
    if (aig(visible = notname[0])) {
	for (i=0; notname[i] && notname[i]==' '; i++);
	if (!notname[i]) {
	    message(MP_ERROR,
		    translate("This template is not visible in the stencil window."));
	    move_cursor_to(-1,0,i);
	    return 5;
	}
    } else {
	if (aig(visible = (editversion[0].format[NAMEFORMAT] &&
			editversion[0].format[NAMEFORMAT][0]))) {
	    check = editversion[0].format[NAMEFORMAT];
	    while (*check && (*check==' ')) check++;
	    if (!*check) {
		message(MP_ERROR,
			translate("This template is not visible in the stencil window."));
		move_cursor_to(0,NAMEFORMAT,
			       Ustrlen(editversion[0].format[NAMEFORMAT]));
		return 5;
	    }
	}
    }
    /*  determine the version with the most place holders */
    m = 0; mi=0;
    for (i=0; i<editnr; i++) {
	int c = 0;
	for (j=0; editversion[i].format[SCREENFORMAT][j]; j++)
	    c += IsPh(editversion[i].format[SCREENFORMAT][j]);
	if (c>m) {
	    m=c;
	    mi = i;
	}
    }
    for (i=0; i<5; phcountfirst[i++]=0)
	for (j=0; j<16; phusedfirst[i][j++] = MP_False);
    phcountfirst[5]=0;
    check = editversion[mi].format[SCREENFORMAT];
    /*
    **  format[SCREENFORMAT] of the version with the most place holders will
    **  be treaded special.
    */
    newvers = mi;
    newformat = 0;
    newpos = 0;
    m = visible;
    while (*check) {
	if (IsPh(*check)) {
	    i = Ph2Num(*check);
	    j = Num(*check);
	    m = MP_True;
	    if (phusedfirst[i][j]) {
		message(MP_ERROR,translate("This place holder has already been used."));
		move_cursor_to(newvers,newformat, newpos);
		return 4;
	    }
	    phusedfirst[i][j] = MP_True;
	    phcountfirst[i]++;
	    if ((++phcountfirst[5])>15) {
		message(MP_ERROR,translate("Only 15 place holders are supported."));
		move_cursor_to(newvers,newformat, newpos);
		return 6;
	    }
	} else
	    if (!IsTab(*check)) m |= (*check != ' ');
	check++;
	newpos++;
    }
    if (mi) {
	for (i=0; editversion[0].format[SCREENFORMAT][i]; i++)
	    if (!IsTab(editversion[0].format[SCREENFORMAT][i]))
		visible |= (editversion[0].format[SCREENFORMAT][i] != ' ');
    } else
	visible = m;
    if (!visible) {
	message(MP_ERROR,translate("This template is not visible in the stencil window."));
	move_cursor_to(0,SCREENFORMAT,0);
	return 5;
    }
    for (i=0; i<5; i++) {
	for (j=0; j<16; j++) {
	    phcheckused[i][j] = phusedfirst[i][j];
	}
	phcheckcount[i]=phcountfirst[i];
    }
    phcheckcount[5] = phcountfirst[5];
    newvers = 0;
    newformat = 0;
    while (newvers<editnr) {
	if (newformat==SCREENFORMAT) {
	    doubles = MP_False;
	    check = editversion[newvers].format[SCREENFORMAT];
	    for (i=0; i<5; i++) {
		for (j=0; j<16; j++) {
		    phused[i][j] = phusedfirst[i][j];
		    phcheckused[i][j] = MP_False;
		}
		phcount[i]=phcountfirst[i];
		phcheckcount[i]=0;
	    }
	    phcount[5] = phcountfirst[5];
	} else {
	    doubles = MP_True;
	    check = editversion[newvers].format[newformat];
	    for (i=0; i<5; i++) {
		for (j=0; j<16; j++) {
		    phused[i][j] = phcheckused[i][j];
		    phcheckused[i][j] = MP_False;
		}
		phcount[i]=phcheckcount[i];
		phcheckcount[i]=0;
	    }
	    phcount[5]=phcheckcount[5];
	}
	newpos = 0;
	phcheckcount[5]=0;
	if (check) {
	    while (*check) {
		if (IsPh(*check)) {
		    i = Ph2Num(*check);
		    j = Num(*check);
		    if (phcheckused[i][j] && !doubles) {
			message(MP_ERROR, translate("This place holder has already"
				" been used."));
			move_cursor_to(newvers, newformat, newpos);
			return 4;
		    }
		    if (!phused[i][j]) {
			if (!doubles || !phusedfirst[i][j]) {
			    message(MP_ERROR,translate("This place holder does not occur"
				    " in the screen format or main version."));
			    move_cursor_to(newvers, newformat, newpos);
			    return 7;
			} else if (warnvers<0) {
			    warnvers=newvers;
			    warnformat=newformat;
			    warnpos=newpos;
			}

		    }
		    phcheckused[i][j] = MP_True;
		    phcheckcount[i]++;
		    phcheckcount[5]++;
		}
		check++;
		newpos++;
	    }
	}
	newformat++;
	if (newformat==NAMEFORMAT) newformat++;
	if (newformat==MAXFORMAT) {
	    newformat=0;
	    newvers++;
	}
    }
    if (warnvers>=0) {
	message(MP_ERROR, translate("Warning: this place holder does not occur"
		" in the related screen format."));
	move_cursor_to(warnvers,warnformat, warnpos);
    }
    if (itemlist[KINDPOS].selection == 0 && itemlist[PRECPOS].selection >0) {
	itemlist[PRECPOS].selection = 0;
	draw_itemlist();
    }
    return 0;
}

static void notadef_handle_button(void *data __attribute__((unused)), int b_num)
{
    int i;
    NOTATION *nota;

    switch (b_num) {
    case DOUBLEBUTTON:
	if (!editnr || editnotation<0)
	    if (press_state & ControlMask)
		remove_multiple_files(edit_fnr);
	    else
		remove_double_file(edit_fnr);
	else
	    remove_double_template(editnotation);
	changed_notation();
	break;
    case MODIFYBUTTON:
	if (bad_notation()) break;
	nota = (NOTATION *) malloc(sizeof(NOTATION));
	set_nota(nota);
	editnotation = add_notation(editnotation, nota);
	afternotation = 0;
	changed_notation();
	break;
    case INSTALLBUTTON:
	if (bad_notation()) break;
	if (editnotation>=0) {
	    afternotation = editnotation;
	    editnotation = -1;
	    unique_number = new_number();
	    for (i=0; i<editnr; i++)
		editversion[i].vnr = new_number();
	}
	nota = (NOTATION*) malloc(sizeof(NOTATION));
	set_nota(nota);
	editnotation = add_notation(editnotation,nota);
	afternotation = 0;
	changed_notation();
	break;
    case DELETEBUTTON:
	/*
	** remove notation from the stencilfile.
	*/
	if (editnotation>=0) {
	    remove_notation(edit_fnr, editnotation);
	    changed_notation();
	}
	break;
    case DONEBUTTON:
	if (can_close_notadef) notadef_close();
	break;
    default:
	break;
    }
}

static int insert_in_version(Char sym, int nr);
static Char* disconnect(int vnr, int fmnr, int len);
static void reconnect(int vnr, int fmnr);
static int insert_String_in_version(Char *str);
static int insert_string_in_version(char *str);

static void draw_cursor(int vnr, int fmnr)
{
    if (vnr<0) {
	if (fmnr) draw_notahelpname();
	else draw_notaname();
    } else {
	int i=0, j=0,k;
	for (k=0; k<MAXFORMAT; k++)
	    i += heights[vnr][k][0];
	draw_version(vnr);
	for (k=0; k<MAXFORMAT; k++)
	    j += heights[vnr][k][0];
	if (i!=j) {
	    reset_lines();
	    for (i=vnr; i<editnr; i++) draw_version(i);
	    clear_bottom();
	}
    }
}

static void toggle_latexmode(void)
{
    INPUT_OK(verscurs>=0) {
	editversion[verscurs].latexmode = editversion[verscurs].latexmode%3 +1;
	draw_cursor(verscurs, formatcurs);
    }
}

static void handle_pos(int x, int y)
{
    int i;
    
    i=positioncode(y);
    switch (i) {
    case -2:
	/* between buttons */
	break;
    case -1:
	/* between items items */
	get_name(x, y);
	get_helpname(x,y);
	break;
    default:
	if (i==editnr) {
	    /* at the bottom, no version available, add one */
	    if (i==MAX_VERSION)
		message(MP_ERROR,translate("Too many versions of one stencil."));
	    else {
		add_version(&editversion, i, &editnr); /* i == editnr-1 */
		if (i!=editnr)
		    heights[i][0] = (int*) malloc(EDIT_SIZE*MAXFORMAT*
						  sizeof(int));
		else
		    heights[i][0] = NULL;
		if (i!=editnr && heights[i][0]) {
		    int j;
		    heights[i][0][0]=0;
		    editversion[i].vnr = new_number();
		    nr_lines[i][0] = 1;
		    for (j=1; j<MAXFORMAT; j++) {
			heights[i][j]=heights[i][j-1]+EDIT_SIZE;
			heights[i][j][0] = 0;
			nr_lines[i][j] = 1;
		    }
		    swap_cursor_off();
		    verscurs=i;
		    make_size_version(editversion+i, 0, 1);
		    if (i>0) { /* add  && default_format  if wanted */
			Char *h;
			editversion[i].latexmode=editversion[0].latexmode;
			for (j=MAXFORMAT-1; j>=0; j--) {
			    if (aig(h=editversion[0].format[j])) {
				formatcurs=j;
				poscurs=possel=0;
				insert_String_in_version(h);
			    } else if (j!=NAMEFORMAT)
				nr_lines[i][j] = nr_lines[0][j];
			}
		    } else {
			if (itemlist[KINDPOS].selection != 0)
			    editversion[i].latexmode= LMATHMODE;
			else
			    editversion[i].latexmode= LTEXTMODE;
		    }
		    formatcurs=0;
		    poscurs=possel=0;
		    push_fontgroup(EDITFONT);
		    invis=MP_True;
		    lines[i+3]=lines[i+2];
		    draw_version(i);
		    invis=MP_False;
		    adjust_scrollbar();
		    draw_version(i);
		    pop_fontgroup();
		} else {
		    message(MP_ERROR,translate("Out of memory."));
		}
		stop_motion_hints();
	    }
	} else if (x<xposversion/2) {
	  Char buffer[80];
	  Char *c;
	  buffer[79]=0;
	  c = Ultostr(editversion[i].vnr, buffer+79);
	  message2(MP_MESSAGE, translate("Internal version number: "),c);
	} else {
	    int k;
	    for (k=0; k<MAXFORMAT; k++)
		detect_format_pos(i, k, x, y);
	}
	break;
    }
}

static void notadef_press(void *data __attribute__((unused)), XButtonEvent *event)
{
    get_motion_hints(notadefwin, -1);
    posstart=-1;
    handle_pos(event->x, event->y);
}

static void insert_special_symbol(Char sym);
static void delete_char(Index arg);
static void insert_return(Index arg);

static void notadef_release(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
    stop_motion_hints();
}

static void notadef_motion(void *data __attribute__((unused)), int x, int y)
{
    handle_pos(x,y);
}

static void notadef_scrollto(void *data __attribute__((unused)), int kind __attribute__((unused)))
{
    int i;
    reset_lines();
    XClearArea(display, notadefwin, 0, lines[editnr+2],0,0,MP_False);
    for (i=0; i<editnr; i++)
	draw_version(i);
}

static void notadef_resize(void *data __attribute__((unused)), XConfigureEvent *event)
{
    int x,y;

    win_width  = event->width;
    win_height = event->height;
    window_manager_added(notadefwin, &x,&y);
    win_xpos = event->x-x;
    win_ypos = event->y-y;
    scrollbar_resize(scrollver, win_height-lines[1]-INTERSPACE);
}

static void insert_char_in_name(Char *name, Char ch, int n __attribute__((unused)))
{
    int i=0;

    i = Ustrlen(name)+2;
    if (i<NAME_SIZE) {
	while (i>poscurs) {
	    name[i]=name[i-1];
	    i--;
	}
	name[poscurs]=ch;
	poscurs++;
    } else
	message(MP_ERROR,translate("Name too big."));
}

static void delete_char_in_name(Char *name, int n)
{
    int i=poscurs;

    if (name[i])
	do {
	    name[i]=name[i+n];
	} while (name[i++]);
}

static int delete_sym(Char *string, int n)
{
    int i=poscurs,j=0,k=n;

    if (string[i])
	do {
	    if (k) {
		if (IsNewline(string[i])) j++;
		k--;
	    }
	    string[i]=string[i+n];
	} while (string[i++]);
    return j;
}

static Char new_ph(Char *str, Char kind)
{
    int i;
    Bool used[16];

    used[0] = MP_True;
    for (i=1; i<16; used[i++]=MP_False);
    
    while (*str) {
	if (IsPh(*str))
	    used[Num(*str)] = MP_True;
	str++;
    }
    if (used[Num(kind)]) {
	for (i=0; i<15 && used[i]; i++);
	return PhNum2Char(Ph(kind), i);
    } else
	return kind;
}

static Char* disconnect(int vnr, int fmnr, int len)
{
    if (!editversion[vnr].format[fmnr] && editversion[vnr].format[SCREENFORMAT]
	&& fmnr!=SCREENFORMAT && fmnr!=NAMEFORMAT) {
	/* copy default format */
	Char *h = editversion[vnr].format[SCREENFORMAT];
	if (make_size_version(editversion+vnr, fmnr, (h?Ustrlen(h):0)+len)) {
	    Char *g = editversion[vnr].format[fmnr];
	    while (aig(*g++=*h++));
	}
	return editversion[vnr].format[fmnr];
    } else {
        Char *h = editversion[vnr].format[fmnr];
	if (make_size_version(editversion+vnr, fmnr, (h?Ustrlen(h):0)+len))
	    return editversion[vnr].format[fmnr];
	else
	    return NULL;
    }
}

static void reconnect(int vnr, int fmnr)
{
    if (!editversion[vnr].format[fmnr][0]) {
	if (fmnr==SCREENFORMAT) {
	    int i;
	    for (i=1; i<MAXFORMAT;i++) {
		if (editversion[vnr].format[i] &&
		    !editversion[vnr].format[i][0]) {
		    free(editversion[vnr].format[i]);
		    editversion[vnr].format[i]=NULL;
		    editversion[vnr].max[i]=0;
		}
	    }
	} else if (!editversion[vnr].format[SCREENFORMAT] ||
		   !editversion[vnr].format[SCREENFORMAT][0]) {
	    free(editversion[vnr].format[fmnr]);
	    editversion[vnr].format[fmnr]=NULL;
	    editversion[vnr].max[fmnr]=0;
	} else if (fmnr==NAMEFORMAT) {
	    free(editversion[vnr].format[fmnr]);
	    editversion[vnr].format[fmnr]=NULL;
	    editversion[vnr].max[fmnr]=0;
	}
    }
}

static int insert_String_in_version(Char *str)
{
    int i,l,nl,k;
    Char *s1;

    if (formatcurs!=NAMEFORMAT)
	l = (str?Ustrlen(str):0);
    else {
	s1 = str;l=0;
	if (s1) while (*s1) l += !IsTab(*s1++);
    }
    if (!l) return 0;
    s1 = disconnect(verscurs, formatcurs, l);
    if (!s1) return 0;
    i=k=nl=0;
    while (s1[i++]);
    while (i>poscurs) { i--; s1[i+l]=s1[i]; }
    while (k<l) {
	if (formatcurs!=NAMEFORMAT || !IsTab(*str)) {
	    s1[i++]=*str;
	    k++;
	    nl+= IsNewline(*str);
	}
	str++;
    }
    nr_lines[verscurs][formatcurs] += nl;
    if (formatcurs==SCREENFORMAT && nl)
	for (i=0; i<MAXFORMAT; i++)
	    if (i!=SCREENFORMAT && i!=NAMEFORMAT &&
		!editversion[verscurs].format[i])
		nr_lines[verscurs][i] += nl;
    poscurs += l;
    possel=poscurs;
    return nl;
}

static int insert_string_in_version(char *str)
{
    int i,l,nl,k;
    Char *s1; Char h;

    if (formatcurs!=NAMEFORMAT)
	l = strlen(str);
    else {
	char *s2;
	s2 = str;l=0;
	if (s2) while (*s2) { l += (*s2 != '\n' && *s2 != '\t');s2++; }
    }
    if (!l) return 0;
    s1 = disconnect(verscurs, formatcurs, l);
    if (!s1) return 0;
    i=k=nl=0;
    while (s1[i++]);
    while (i>poscurs) { i--; s1[i+l]=s1[i]; }
    while (k<l) {
	h = (*str=='\n' ? Newline : (*str=='\t' ? Rtab: *str));
	if (formatcurs!=NAMEFORMAT || !IsTab(h)) {
	    s1[i++]= h;
	    k++;
	    nl+= (h==Newline);
	}
	str++;
    }
    nr_lines[verscurs][formatcurs] += nl;
    if (formatcurs==SCREENFORMAT && nl)
	for (i=0; i<MAXFORMAT; i++)
	    if (i!=SCREENFORMAT && i!=NAMEFORMAT &&
		!editversion[verscurs].format[i])
		nr_lines[verscurs][i] += nl;
    poscurs += l;
    possel=poscurs;
    return nl;
}

static int insert_in_version(Char sym, int n)
{
    int i=0,k=0;
    Char *s1=NULL;

    if (!sym || n<=0 || (formatcurs==NAMEFORMAT && sym>=SoftNewline)) return 0;
    s1 =  disconnect(verscurs, formatcurs, n);
    if (!s1) return 0;
    if (IsPh(sym) && (formatcurs==SCREENFORMAT || !Num(sym)))
	sym = new_ph(s1, sym);
    while (s1[i++]);
    if (i<=poscurs) poscurs=i-1;
    while (i>poscurs) {
	i--;
	s1[i+n]=s1[i];
    }
    while (k<n) {
	s1[i++]= sym;
	k++;
    }
    poscurs +=n;
    possel=poscurs;
    if (IsNewline(sym)) {
	nr_lines[verscurs][formatcurs] += n;
	if (formatcurs==SCREENFORMAT)
	    for (i=0; i<MAXFORMAT; i++)
		if (i!=SCREENFORMAT && i!=NAMEFORMAT &&
		    !editversion[verscurs].format[i])
		    nr_lines[verscurs][i] +=n;
	return n;
    } else return 0;
}

#define STOP 0
#define NEWL 1
#define OPENBRACE 2
#define SEPERATOR 3
#define CLOSEBRACE 4
#define OTHER 5

static int char_kind(Char c)
{
    /* 0: 0,
    ** 1: newline,
    ** 2: brace open,
    ** 3: field seperator,
    ** 4: brace close,
    ** 5: other
    */
    switch (c) {
    case 0:
	return STOP;
    case Newline: case SoftNewline:
	return NEWL;
    case StartHide: case OpenTop: case OpenGap: case OpenBottom:
    case StackB: case StackC: case TabOpen: case DisplayOpen:
    case PlName: case ColorStart:
	return OPENBRACE;
    case TopGap: case GapBottom: case ColorSep:
	return SEPERATOR;
    case CloseStack: case CloseTop: case CloseGap: case CloseBottom:
    case PopSize: case StackClose: case EndHide: case TabClose:
    case DisplayClose: case PlNameEnd: case ColorEnd:
	return CLOSEBRACE;
    default:
	switch (Char2Font(c)) {
	case FontFont: 	case StackFont: case SizeFont: case AttribFont:
	    return OPENBRACE;
	case PopFont: case AttrPopFont:
	    return CLOSEBRACE;
	default:
	    return OTHER;
	}
    }
}

static void make_selection(int vnr, int fmnr, int pos1, int pos2)
{
    int i,j, n,m;
    Char *c;
    Char b[1];
    if (pos1>pos2) pos1 ^= pos2 ^= pos1 ^= pos2;
    n=m=0;
    i=pos1;
    c = editversion[vnr].format[fmnr];
    if (!c)
	if (fmnr==LATEXFORMAT) c = editversion[vnr].format[SCREENFORMAT];
	else c=b;
    while (i<=pos2) {
	j=char_kind(c[i]);
	switch (j) {
	case STOP:
	    pos2=i;
	    break;
	case OPENBRACE:
	    n++;
	    break;
	case CLOSEBRACE:
	    n--;
	    if (n<m) m=n;
	    break;
	case SEPERATOR:
	    if (n-1<m) m=n-1;
	    break;
	default:
	    break;
	}
	i++;
    }
    i=0;
    while (i>m && pos1) {
	pos1--;
	j=char_kind(c[pos1]);
	if (j==OPENBRACE) i--;
	else if (j==CLOSEBRACE) i++;
    }
    i=n;
    while (i>m) {
	pos2++;
	j=char_kind(c[pos2]);
	if (j==OPENBRACE) i++;
	else if (j==CLOSEBRACE) i--;
	else if (j==STOP) i=m;
    }
    possel=pos1;
    if (!c[pos2]) poscurs=pos2;
    else poscurs=pos2+1;
}

static int open_match(Char *s, int i, int *k)
{
    int val = 1;

    while (val && i) {
	i--;
	switch (char_kind(s[i])) {
	case STOP:
	    *k=STOP;
	    val=0;
	    break;
	case OPENBRACE:
	    if (!--val) *k=OPENBRACE;
	    break;
	case SEPERATOR:
	    if (!--val) {
		*k=SEPERATOR;
		break;
	    }
	case CLOSEBRACE:
	    val++;
	    break;
	default:
	    break;
	}
    }
    if (val) *k = STOP;
    return i;	    
}

static int close_match(Char *s, int i, int *k)
{
    int val = 1;

    while (val) {
	switch (char_kind(s[i])) {
	case STOP:
	    *k=STOP;
	    val=0;
	    break;
	case SEPERATOR:
	    if (!--val) {
		*k=SEPERATOR;
		break;
	    }
	case OPENBRACE:
	    val++;
	    i++;
	    break;
	case CLOSEBRACE:
	    if (!--val) {
		*k=CLOSEBRACE;
		break;
	    }
	default:
	    i++;
	    break;
	}
    }
    return i;	    
}
static int kill_length(Char *s, int pos)
{
    int j,val,lpos;

    j=lpos=pos; val=1;
    while (val) {
	switch (char_kind(s[j])) {
	case STOP:
	case NEWL:        val=0;          break;
	case SEPERATOR:   if (!--val)     break;
	case OPENBRACE:   val++; j++;     break;
	case CLOSEBRACE:  if (--val) j++; break;
	default:          j++;            break;
	}
	if (val==1) lpos = j;
    }
    if (lpos>pos) return lpos-poscurs; else return 1;
}

static int delete_check(Char *s, int max)
{
    int i,l=0,n=0;
    for (i=0; i<max && l>=0; i++) {
	switch(char_kind(s[i])) {
	case OPENBRACE:
	    if (!l++) n=i;
	    break;
	case SEPERATOR:
	    if (!l) {
		l--;
		n=i;
	    }
	    break;
	case CLOSEBRACE:
	case STOP:
	    if (!l--) n=i;
	    break;
	default:
	    break;
	}
    }
    if (i==max && !l) return max;
    else return n;
}

static int delete_in_version(int n)
{
    int i,l,j,val,lpos;
    Char *s1=NULL;

    if (!n) return 0;
    s1 = disconnect(verscurs, formatcurs, 0);
    j = char_kind(s1[poscurs]);
    n=delete_check(s1+poscurs, n);
    if (!n) n=1;
    i = delete_sym(s1, n);
    nr_lines[verscurs][formatcurs] -= i;
    if (n==1)
	switch (j) {
	case OPENBRACE:
	case SEPERATOR:
	    l=val=poscurs;
	    do {
		val = close_match(s1, val, &lpos);
		if (lpos!=STOP) {
		    poscurs = val;
		    delete_sym(s1,1);
		}
	    } while (lpos!=STOP && lpos!=CLOSEBRACE);
	    poscurs = l;
	    if (j==OPENBRACE) break;
	case CLOSEBRACE:
	    l=val=poscurs;
	    do {
		val = open_match(s1, val, &lpos);
		if (lpos!=STOP) {
		    poscurs = val;
		    delete_sym(s1,1);
		    l--;
		}
	    } while (lpos!=STOP && lpos!=OPENBRACE);
	    poscurs = l;
	    possel = poscurs;
	    break;
	default:
	    break;
	}
    reconnect(verscurs, formatcurs);
    /*
    ** i == number of lines removed from this version.
    */
    return i;
}

static void clear_window(void)
{
    new_editnotation();
    adjust_scrollbar();
    XClearArea(display, notadefwin,0,lines[1]+1,0,0,MP_False);
    draw_all();
}

static void clear_version(void)
{
    int i,j;

    if (verscurs<0) return;
    i = verscurs;
    move_cursor_to(-1, 0, Ustrlen(notname));
    free(heights[i][0]);
    j=i+1;
    move_version(j, lines[j+1]-lines[j+2]);
    for (; j<=editnr; j++) {
	int k;
	lines[j+2] = lines[j+3];
	for (k=0; k<MAXFORMAT; k++) {
	    nr_lines[j-1][k] = nr_lines[j][k];
	    heights[j-1][k] = heights[j][k];
	}
    }
    remove_version(&editversion, i, &editnr);
    adjust_scrollbar();
}

static void auto_format(void)
{
    int i,j;
    if (itemlist[KINDPOS].selection != 0) j=LMATHMODE; else j=LTEXTMODE;
    if (verscurs<0) {
	for (i=0; i<editnr; i++) {
	    if (editversion[i].format[LATEXFORMAT]) {
		free(editversion[i].format[LATEXFORMAT]);
		editversion[i].format[LATEXFORMAT]=NULL;
		editversion[i].max[LATEXFORMAT]=0;
		nr_lines[i][LATEXFORMAT]=nr_lines[i][0];
	    }
	    editversion[i].latexmode = j;
	}
    } else
	if (formatcurs!=SCREENFORMAT) {
	    if (editversion[verscurs].format[formatcurs]) {
		free(editversion[verscurs].format[formatcurs]);
		editversion[verscurs].format[formatcurs]=NULL;
		editversion[verscurs].max[formatcurs]=0;
		if (formatcurs!=NAMEFORMAT)
		    nr_lines[verscurs][formatcurs]=nr_lines[verscurs][0];
		else
		    nr_lines[verscurs][formatcurs]=1;
	    }
	    if (formatcurs==LATEXFORMAT)
		editversion[verscurs].latexmode=j;
	} else {
	    for (i=0; i<MAXFORMAT; i++)
		if (i!=SCREENFORMAT && i!=NAMEFORMAT)
		    if (editversion[verscurs].format[i]) {
			free(editversion[verscurs].format[i]);
			editversion[verscurs].format[i]=NULL;
			editversion[verscurs].max[i]=0;
			nr_lines[verscurs][i]=nr_lines[verscurs][0];
		    }
	    editversion[verscurs].latexmode=j;
	}
    move_cursor_to(verscurs, formatcurs, poscurs);
    reset_lines();
    draw_all();
}

static void transpose_char(void)
{
    INPUT_OK(poscurs>0) {
	if (verscurs<0) {
	    Char *h = (formatcurs? nothelpname : notname);
	    if (h[poscurs]) {
		char c=h[poscurs];
		h[poscurs]=h[poscurs-1];
		h[poscurs-1]=c;
		poscurs++;
		draw_cursor(verscurs, formatcurs);
	    }
	} else {
	    Char *s1=NULL;
	    int i,kl=0,kr=0,k,l=0;
	    
	    s1 = disconnect(verscurs, formatcurs, 0);
	    if (!s1 || !s1[poscurs]) return;
	    kl = char_kind(s1[poscurs-1]);
	    kr = char_kind(s1[poscurs]);
	    if (!kr || !kl) return;
	    if (kl==OTHER || kl==NEWL || kr==OTHER || kr==NEWL)
		l=1;
	    else if (kr==OPENBRACE) {
		i=poscurs;
		do {
		    i = close_match(s1, i+1, &k);
		} while (k!=STOP && k!=CLOSEBRACE);
		l = i-poscurs+1;
	    } else if (kl==CLOSEBRACE) {
		i=poscurs-1;
		do {
		    i=open_match(s1, i, &k);
		} while (k!=OPENBRACE && i);
		l = i-poscurs;
	    }
	    if (!l) {
		poscurs++;
	    } else if (l>0) {
		Char c = s1[poscurs-1];
		for (i=0; i<l; i++) s1[poscurs+i-1]=s1[poscurs+i];
		s1[poscurs+l-1]=c;
		poscurs+=l;
	    } else {
		Char c = s1[poscurs];
		for (i=0; i>l; i--) s1[poscurs+i]=s1[poscurs+i-1];
		s1[poscurs+l]=c;
		poscurs++;
	    }
	    possel=poscurs;
	    draw_cursor(verscurs, formatcurs);
	}
    }
}

static void insert_char(int keycode, Index arg)
{
    INPUT_OK(MP_True) {
	if (verscurs<0) {
	    Char *h = (formatcurs ? nothelpname : notname);
	    insert_char_in_name(h, (Char) keycode, arg);
	} else
	    (void) insert_in_version( (Char) keycode, arg);
	draw_cursor(verscurs, formatcurs);
    }
}

static void insert_return(Index arg)
{
    INPUT_OK(verscurs>=0 && formatcurs!=NAMEFORMAT) {
	insert_in_version(Newline,arg);
	draw_cursor(verscurs, formatcurs);
    }
}

static void insert_soft_return(Index arg)
{
    INPUT_OK(verscurs>=0 && formatcurs!=NAMEFORMAT) {
	insert_in_version(SoftNewline,arg);
	draw_cursor(verscurs, formatcurs);
    }
}

static void insert_tab(Index arg)
{
    INPUT_OK(verscurs>=0 && formatcurs!=NAMEFORMAT) {
	(void) insert_in_version( Rtab, arg);
	draw_version(verscurs);
    }
}

static void insert_backtab(Index arg)
{
    INPUT_OK(verscurs>=0 && formatcurs!=NAMEFORMAT) {
	(void) insert_in_version( Ltab, arg);
	draw_version(verscurs);
    }
}

static void insert_settab(Index arg)
{
    INPUT_OK(verscurs>=0 && formatcurs!=NAMEFORMAT) {
	(void) insert_in_version( Settab, arg);
	draw_version(verscurs);
    }
}

static void delete_char(Index arg)
{
    INPUT_OK(MP_True) {
	if (verscurs < 0)
	    delete_char_in_name((formatcurs? nothelpname:notname),arg);
	else {
	    if (possel!=poscurs) {
		arg=poscurs-possel;
		poscurs=possel;
	    }
	    delete_in_version(arg);
	}
	draw_cursor(verscurs, formatcurs);
    }
}

static void backspace_char(Index arg)
{
    INPUT_OK(poscurs>0) {
	if (verscurs>=0 && possel!=poscurs) arg = poscurs-possel;
	if (poscurs < (int)arg) arg = poscurs;
	poscurs -=arg;
	possel=poscurs;
	delete_char(arg);
    }
}

static void kill_line(void)
{
    int i;

    INPUT_OK(MP_True) {
	if (verscurs<0) {
	    Char *h= (formatcurs ? nothelpname : notname);
	    delete_char_in_name(h, (h?Ustrlen(h)-poscurs:0));
	} else {
	    Char *c;
	    c = editversion[verscurs].format[formatcurs];
	    if (!c && formatcurs==LATEXFORMAT)
		c=editversion[verscurs].format[SCREENFORMAT];
	    if (c) {
		i = kill_length(c,poscurs);
		delete_in_version(i);
	    }
	}
	draw_cursor(verscurs, formatcurs);
    }
}

/* go to begin of line, using newlines */
static void go_home(void)
{
  INPUT_OK(poscurs>0) {
    Char *c;
    int npos;
    if (verscurs<0) {
      npos=0;
    } else {
      c = editversion[verscurs].format[formatcurs];
      if (!c && formatcurs==LATEXFORMAT)
	c = editversion[verscurs].format[SCREENFORMAT];
      if (c) {
	npos=poscurs;
	while (npos && !IsNewline(c[npos-1])) npos--;
      }
    }
    move_cursor_to(verscurs, formatcurs, npos);
  }
}

/* go character left */
static void go_left(void)
{
    INPUT_OK(poscurs>0)
	move_cursor_to(verscurs, formatcurs,poscurs-1);
}

/* go character right */
static void go_right(void)
{
    INPUT_OK(MP_True)
	move_cursor_to(verscurs,formatcurs, poscurs+1);
}

/* go to end of line, using newlines */
static void go_end(void)
{
  INPUT_OK(MP_True) {
    Char *c;
    int npos;
    if (verscurs<0) {
      npos=0xffff;
    } else {
      c = editversion[verscurs].format[formatcurs];
      if (!c && formatcurs==LATEXFORMAT)
	c = editversion[verscurs].format[SCREENFORMAT];
      if (c) {
	npos=poscurs;
	while (c[npos] && !IsNewline(c[npos])) npos++;
      }
    }
    move_cursor_to(verscurs, formatcurs, npos);
  }
}

static int old_x_pos=0, old_y_pos=0;

static int get_current_line(int vnr, int fmnr, int ypos)
{
  int i,y;
  push_fontgroup(EDITFONT);
  y=lines[vnr+2]+line_height()/2;
  for (i=0;i<fmnr; i++) y+= heights[vnr][i][0];
  i=0;
  while (i<nr_lines[vnr][fmnr] && ypos>y) {
    i++;
    y+= heights[vnr][fmnr][i];
  }
  return i-1;
}

/* go one line up, using current cursor position */
static void go_up(void)
{
    INPUT_OK(verscurs>=0 || formatcurs>0) {
      if (verscurs<0) {
	move_cursor_to(-1,0,poscurs);
      } else {
	int lnr,vnr, fmnr;
	if (old_x_pos!=cursor_x || old_y_pos!=cursor_y || !cursor_target)
	  cursor_target=cursor_x;
	vnr=verscurs;
	fmnr=formatcurs;
	swap_cursor_off();
	lnr = get_current_line(vnr, fmnr, cursor_y+1);
	if (lnr==0) {
	  if (fmnr>0) {
	    fmnr--;
	    lnr = nr_lines[vnr][fmnr]-1;
	  } else {
	    if (vnr>0) {
	      vnr--;
	      fmnr=MAXFORMAT-1;
	      lnr=nr_lines[vnr][fmnr]-1;
	    } else {
	      vnr=-1;
	      fmnr=1;
	      lnr=0;
	    }
	  }
	} else {
	  lnr--;
	}
	if (vnr>=0) {
	  detect_line=lnr;
	  detect_pos=cursor_target;
	  formatcurs=0;
	  verscurs=-1;
	  invis=MP_True;
	  poscurs=possel=0;posstart=-1;
	  draw_format_args(vnr,fmnr);
	  poscurs=detect_pos;
	}
	move_cursor_to(vnr,fmnr,poscurs);
	old_x_pos=cursor_x;
	old_y_pos=cursor_y;
      }
    }
}

/* go one line down, using current cursor position */
static void go_down(void)
{
    INPUT_OK((verscurs<editnr-1 || formatcurs != MAXFORMAT-1)) {
      if (verscurs==-1) {
	if (!formatcurs)
	  move_cursor_to(-1,1,poscurs);
	else if (editnr)
	  move_cursor_to(0,0,0);
      } else {
	int lnr,vnr, fmnr;
	if (old_x_pos!=cursor_x || old_y_pos!=cursor_y || !cursor_target)
	  cursor_target=cursor_x;
	vnr=verscurs;
	fmnr=formatcurs;
	swap_cursor_off();
	lnr = get_current_line(vnr, fmnr, cursor_y+1);
	if (lnr==nr_lines[vnr][fmnr]-1) {
	  if (fmnr<MAXFORMAT-1) {
	    fmnr++;
	    lnr = 0;
	  } else {
	    vnr++;
	    fmnr=0;
	    lnr=0;
	  }
	} else {
	  lnr++;
	}
	if (vnr>=0) {
	  detect_line=lnr;
	  detect_pos=cursor_target;
	  formatcurs=0;
	  verscurs=-1;
	  invis=MP_True;
	  poscurs=possel=0;posstart=-1;
	  draw_format_args(vnr,fmnr);
	  poscurs=detect_pos;
	}
	move_cursor_to(vnr,fmnr,poscurs);
	old_x_pos=cursor_x;
	old_y_pos=cursor_y;
      }
    }
}

static void insert_special_symbol(Char sym)
{
    INPUT_OK(verscurs>=0) {
	(void) insert_in_version(sym,1);
	draw_cursor(verscurs, formatcurs);
    }
}


static void switch_versions(int side)
{
    int i = verscurs;

    INPUT_OK(i>=0 && side && i+side>=0 && i+side<editnr) {
	int j, h, l;
	int nl[MAXFORMAT];
	VERSION vs;

	for (l=0; l<MAXFORMAT; l++) nl[l] = nr_lines[i][l];
	vs = editversion[i];
	h = (side<0 ? -1 : 1);
	j = side;
	while (j) {
	    for (l=0; l<MAXFORMAT; l++)
		nr_lines[i][l] = nr_lines[(i+h)][l];
	    editversion[i] = editversion[i+h];
	    if (h>0)
		lines[3+i] = lines[4+i]+lines[2+i]-lines[3+i];
	    else
		lines[2+i] = lines[3+i]+lines[1+i]-lines[2+i];
	    i += h;
	    j -= h;
	}
	for (l=0; l<MAXFORMAT; l++) nr_lines[i][l] = nl[l];
	editversion[i] = vs;
	verscurs += side;
	for (i=0; i<editnr; i++) draw_version(i);
	move_cursor_to(verscurs, formatcurs,poscurs);
    }
}

static void move_version_down(Index arg)
{
    INPUT_OK(MP_True)
	switch_versions(arg);
}

static void move_version_up(Index arg)
{
    INPUT_OK(MP_True)
	switch_versions(-arg);
}

static void move_version_first(void)
{
    INPUT_OK(verscurs>0)
	switch_versions(-verscurs);
}

static void move_version_last(void)
{
    INPUT_OK(verscurs>0)
	switch_versions(editnr - verscurs -1);
}

static void notadef_insert_string(void)
{
    INPUT_OK(verscurs>=0 && wmselection) {
	int i,oh=0,nh=0;

	insert_string_in_version(wmselection);
	for (i=0; i<MAXFORMAT; i++) oh+= heights[verscurs][i][0];
	draw_version(verscurs);
	for (i=0; i<MAXFORMAT; i++) nh+= heights[verscurs][i][0];
	if (oh!=nh) {
	    move_version(verscurs+1,nh-oh);
	    adjust_scrollbar();
	}
    }
}

static void ask_selection(void)
{
    get_wm_selection();
}

static void set_precedence(int value)
{
  itemlist[PRECPOS].selection=value;
  draw_itemlist();
}

static void set_kind(int value)
{
  itemlist[KINDPOS].selection=value;
  draw_itemlist();
}

static void set_spacing(int value)
{
  itemlist[SPACEPOS].selection=value;
  draw_itemlist();
}

static void notadef_keysymbol(void)
{
    Char selchar;
    selchar = symbol_last();
    INPUT_OK(selchar) {
      if (verscurs>=0) {
	insert_in_version(selchar, 1);
      } else {
	Char *h= (formatcurs? nothelpname : notname);
	insert_char_in_name(h, selchar, 1);
      }
      draw_cursor(verscurs, formatcurs);
    }
}

static int insert_formatstring(Bool preph, Char *str, Bool postph)
{
    int deltaln = 0;

    if (preph) deltaln += insert_in_version(MP_Expr,1);
    insert_String_in_version(str);
    if (postph) deltaln += insert_in_version(MP_Expr,1);
    return deltaln;
}

static void insert_notation(NOTATION *nota, int vnr)
{
    VERSION temp;
    Bool preph, postph;
    int i=0,j,n,oh=0,nh=0;

    INPUT_OK(nota) {
	if (verscurs<0) {
	    int fl=0,nl=0,f;
	    new_editnotation();
	    editnotation = nota->innr;
	    afternotation = -1;
	    get_nota(nota);
	    for (i=0; i<editnr; i++) {
		if (i==vnr) fl = nl;
		nl++;
		for (f=0; f<MAXFORMAT; f++) {
		    Char *h = editversion[i].format[f];
		    n=1;
		    if (f==LATEXFORMAT && !h) n=nr_lines[i][SCREENFORMAT];
		    else if (h) {
			while (*h) {
			    n+= IsNewline(*h);
			    h++;
			}
		    }
		    nr_lines[i][f] = n;
		    nl+=n;
		}
		heights[i][0] = (int *) malloc(EDIT_SIZE*MAXFORMAT*
						      sizeof(int));
		for (f=1; f<MAXFORMAT; f++)
		    heights[i][f]=heights[i][f-1]+EDIT_SIZE;
	    }
	    invis = MP_True;
	    draw_all();
	    invis = MP_False;
	    scrollbar_set(scrollver, fl,nl);
	    reset_lines();
	    XClearArea(display, notadefwin, 0, lines[1]+1, 0, 0, MP_False);
	    draw_all();
	} else if (formatcurs==NAMEFORMAT) {
	    temp = nota->vers[vnr];
	    i = verscurs;
	    for (j=0; j<MAXFORMAT; j++) {
		Char *h;
		if (aig(h=temp.format[j])) {
		    make_size_version(editversion+i, j, (h?Ustrlen(h):0));
		    Ustrcpy(editversion[i].format[j],h);
		    nr_lines[i][j] = 1;
		    while (*h) {
			nr_lines[i][j] += IsNewline(*h);
			h++;
		    }
		} else {
		    if (editversion[i].format[j]) {
			free(editversion[i].format[j]);
			editversion[i].format[j] = NULL;
			editversion[i].max[j]=0;
		    }
		    if (j==NAMEFORMAT)
			nr_lines[i][j] = 1;
		    else
			nr_lines[i][j] = nr_lines[i][SCREENFORMAT];
		}
	    }
	    for (j=0; j<MAXFORMAT; j++) oh+=heights[i][j][0];
	    invis= MP_True;
	    draw_version(verscurs);
	    invis=MP_False;
	    for (j=0; j<MAXFORMAT; j++) nh+=heights[i][j][0];
	    if (oh!=nh) {
		move_version(verscurs+1, nh-oh);
		adjust_scrollbar();
	    }
	    draw_version(verscurs);
	} else {
	    temp = nota->vers[vnr];
	    i=verscurs;
	    preph = (nota->kind != 0 && nota->kind != 1);
	    postph = (nota->kind != 0 && nota->kind != 2);
	    if (formatcurs==SCREENFORMAT) {
		int f;
		int op=poscurs;
		for (f=0; f<MAXFORMAT; f++)
		   if (f!=SCREENFORMAT && f!=NAMEFORMAT &&
		       !editversion[i].format[f] && temp.format[f]) {
		       formatcurs = f;
		       insert_formatstring(preph, temp.format[f], postph);
		       poscurs = op;
		   }
		formatcurs = SCREENFORMAT;
	    }
	    if (!temp.format[formatcurs])
		insert_formatstring(preph, temp.format[SCREENFORMAT], postph);
	    else
		insert_formatstring(preph, temp.format[formatcurs], postph);
	    for (j=0; j<MAXFORMAT; j++) oh+=heights[i][j][0];
	    invis= MP_True;
	    draw_version(verscurs);
	    invis=MP_False;
	    for (j=0; j<MAXFORMAT; j++) nh+=heights[i][j][0];
	    if (oh!=nh) {
		move_version(verscurs+1, nh-oh);
		adjust_scrollbar();
	    }
	    draw_version(verscurs);
	}
    }
}

static void notadef_notation(void)
{
    NOTATION *tempnota = which_notation(notation_last());

    insert_notation(tempnota, notation_version());
}

static void notadef_selected_notation(void)
{
    int vnr=0;
    int nr = ss_notation(&vnr);
    if (nr>=0) {
	verscurs=-1;
	formatcurs=0;
	poscurs=possel=0;
	insert_notation(which_notation(nnr_vnr2innr(nr,vnr)), vnr);
    }
}
	
static void switch_tabmode(void)
{
    showtabs = !showtabs;
    invis=MP_True;
    draw_all();
    invis=MP_False;
    reset_lines();
    draw_all();
    clear_bottom();
}

static void move_notation_begin(void)
{
    notation_move_begin();
}

static void move_notation_end(void)
{
    notation_move_end();
}

static void move_notation_left(void)
{
    notation_move_left();
}

static void move_notation_right(void)
{
    notation_move_right();
}

static void insert_expr(Index arg)
{
    if (arg>15) arg=0;
    insert_special_symbol(PhNum2Char(MP_Expr, arg));
}

static void insert_op(Index arg)
{
     insert_special_symbol(PhNum2Char(MP_Op, arg));
}

static void insert_text(Index arg)
{
    if (arg>15) arg=0;
    insert_special_symbol(PhNum2Char(MP_Text, arg));
}

static void insert_var(Index arg)
{
    if (arg>15) arg=0;
    insert_special_symbol(PhNum2Char(MP_Var, arg));
}

static void insert_id(Index arg)
{
    if (arg>15) arg=0;
    insert_special_symbol(PhNum2Char(MP_Id, arg));
}

static void insert_gluespace(void)
{
    insert_special_symbol(GlueSpace);
}

static void insert_glueline(void)
{
    insert_special_symbol(GlueLine);
}

static void insert_gluestipple(void)
{
    insert_special_symbol(GlueStipple);
}

static void insert_size(int sizenr)
{
    INPUT_OK(verscurs>=0 && sizenr> -63 && sizenr < 63) {
	insert_in_version(Font2Char(SizeFont, 192+sizenr),1);
	insert_in_version(PopSize,1);
	poscurs--;
	possel=poscurs;
	draw_cursor(verscurs, formatcurs);
    }
}

static void insert_relative_size(int sizenr)
{
    INPUT_OK(verscurs>=0 && sizenr> -63 && sizenr < 63) {
	insert_in_version(Font2Char(SizeFont, 64+sizenr),1);
	insert_in_version(PopSize,1);
	poscurs--;
	possel=poscurs;
	draw_cursor(verscurs, formatcurs);
    }
}

static void insert_font(int fontnr)
{
    INPUT_OK(verscurs>=0 && fontnr>=0 && fontnr<255) {
	insert_in_version(Font2Char(FontFont, fontnr),1);
	insert_in_version(Font2Char(PopFont, fontnr),1);
	draw_cursor(verscurs, formatcurs);
    }
}

static void notadef_insert_markup(Char *pre, Char *post)
{
  INPUT_OK(verscurs>=0) {
	int i,k,ps;
	i = 0;
	k = poscurs-possel;
	ps=possel;
	while (post[i]) {
	    insert_in_version(post[i],1);
	    i++;
	}
	poscurs = ps;
	i=0;
	while (pre[i]) {
	    insert_in_version(pre[i],1);
	    i++;
	}
	possel=poscurs;
	poscurs += k;
	draw_cursor(verscurs, formatcurs);
    }
}

static void notadef_symbol_short(Index arg)
{
    if (arg && char_width(arg))
	insert_special_symbol(arg);
}

static void notadef_notation_short(Char *name)
{
    int usenota;
    int i=0,l;

    if (!name) return;
    l = Ustrlen(name);
    if (l>2 && name[l-2]==':' && name[l-1]>='1' && name[l-1]<='9') {
	i = name[l-1]-'0';
	name[l-2] = '\0';
    }
    usenota = notation_with_name(name);
    if (usenota>=0) {
	if (!i || i<=which_notation(usenota)->versions)
	    insert_notation(which_notation(usenota), (i?i-1:0));
    }
    if (i) name[l-2]=':';
}

static int notadef_last_pos(int *x, int *y, int *w, int *h)
{
    *x = win_xpos;
    *y = win_ypos;
    *w = win_width;
    *h = win_height;
    return MP_False;
}

static void notadef_set_last_pos(int x, int y, int w, int h)
{
    win_xpos = x;
    win_ypos = y;
    win_width = w;
    win_height = h;
}

FUNCTIONS notadeffuncs = {
    notadef_bad_end, notadef_draw, notadef_resize, notadef_press,
    notadef_release, notadef_motion, notadef_iconize, notadef_deiconize,
    NULL, NULL, notadef_layout_change, NULL, NULL, NULL, NULL, NULL,
    notadef_last_pos, notadef_set_last_pos };

void notadef_init(void)
{
    int i,n, k, ytemp,wh,ww;
    char namebuf[500];
    MENU *mp;

    /* initialiseer window-attributen */
    notadef_mask =
        (CWBackPixel | CWBorderPixel | CWBitGravity |
	 CWColormap | CWEventMask);
    notadef_attr.background_pixel = white_pixel;
    notadef_attr.border_pixel = black_pixel;
    notadef_attr.colormap = colormap;
    notadef_attr.bit_gravity = NorthWestGravity;
    notadef_attr.event_mask = (ExposureMask | ButtonPressMask
			       | ButtonReleaseMask | ButtonMotionMask
			       | PointerMotionHintMask | KeyPressMask
			       | FocusChangeMask
			       | StructureNotifyMask | VisibilityChangeMask);
    /* initialiseer items */
    n = 0;
    k = 0;
    mp = (MENU*) malloc(sizeof(MENU)*MAX_ITEMLIST);
    memset(mp, 0, sizeof(MENU)*MAX_ITEMLIST);
    /* mlp = (MENULINE*) malloc(sizeof(MENULINE)*ni); */
    strcpy(namebuf, "DefinePopup");
    for (i=0; i<MAX_ITEMLIST; i++) {
        strcpy(namebuf+11, itemlist[i].description);
	itemlist[i].menu = mp;
	itemlist[i].menu->menu = popup_define(translate(namebuf));
	itemlist[i].menu->x= -1;
	itemlist[i].menu->y= -1;
	mp++;
    }
    unique_number = new_number();
    xpos=INTERSPACE*2;
    lines[0]=button_height+BINTERSPACE;
    ypos=lines[0]*2;
    verscurs=-1;
    formatcurs=0;
    poscurs = 0;
    notname[0] = '\0';
    nothelpname[0]='\0';
    notadef_layout_change(NULL);
    ytemp = BINTERSPACE*2+button_width(translate(itemlist[MAX_ITEMLIST-1].description));
    ww = 0;
    for (i=0; i<NR_BUTTON; i++)
	ww += button_width(translate(notadefbutton[i]))+BINTERSPACE;
    wh= 8*ww / 5;
    if (ww<ytemp) ww = ytemp;
    if (!win_width) {
	win_width = ww;
	win_height = wh;
	win_xpos = (display_width-ww)/2;
	win_ypos = (display_height-wh)/2;
    }
    if (!XStringListToTextProperty(&icon_name, 1, &iconname))
	message(MP_EXIT -1, translate("Can't make Define-icon"));
}

static void create_popup(void *data, int n __attribute__((unused)))
{
    if (mouse_button == Button3) {
	popup_make((MENU*) data);
    } else {
        popup_call_default((MENU*) data);
    }
}

void notadef_open(void)
{
    int x = BINTERSPACE/2;
    int y = BINTERSPACE/2;
    int i,j;
    XSizeHints size_hints;

    if (!win_xpos) {
	win_xpos = (display_width - win_width)/2;
	win_ypos = (display_height - win_height)/2;
    }
    free(notadef_name);
    notadef_name = concat(translate(NOTADEFNAME), get_notation_filename(edit_fnr));
    {
      char *tn;
      tn=(char*)UstrtoLocale(notadef_name);
      if (!XStringListToTextProperty(&tn, 1, &notadefname))
	message(MP_MESSAGE,translate("Can't make Define-name"));
    }
    notadefwin = XCreateWindow(display, root_window,
			       win_xpos, win_ypos, win_width, win_height,
			       BORDERWIDTH, CopyFromParent, InputOutput,
			       visual,
			       notadef_mask, &notadef_attr);
    size_hints.flags = PPosition | PSize | PMinSize;
    size_hints.min_width =
	size_hints.min_height = button_height*3;
    
    XSetWMProperties(display, notadefwin, &notadefname, &iconname,
		     NULL, 0, &size_hints, &wm_hints, &class_hints);
    set_protocols(notadefwin);
    i=0;
    if (add_window(notadefwin, NOTATIONDEFWINDOW, root_window,
		   NULL, translate(helpname[DEFINEHELP]))) {
	while (i<NR_BUTTON &&
	       button_make(i, notadefwin, translate(notadefbutton[i]), &x, y, 1, NULL,
			   helpname[notahelp[i]],
			   NULL,NULL,NULL,notadef_handle_button,NULL,NULL))
	    i++,x+=BINTERSPACE;
	j=0;
	x=xpos;y=lines[0]+INTERSPACE;
	while (j<MAX_ITEMLIST && (x=itemlist[j].x) &&
	       (itemlist[j].buttondata = 
		button_make(itemlist[j].def,notadefwin,translate(itemlist[j].description),
			   &x, y, 0, itemlist[j].menu,
			   helpname[itemlist[j].helpnr],
			   NULL, NULL, create_popup,
			   create_popup, create_popup, NULL))) {
	    itemlist[j].menu->parentwin=notadefwin;
	    j++;
	}
	push_fontgroup(EDITFONT);
	scrollver = scrollbar_make(VERTICAL, notadefwin, INTERSPACE, lines[1],
				   win_width-lines[1]-INTERSPACE,line_height(),
				   notadef_scrollto, NULL);
	pop_fontgroup();
	if (i<NR_BUTTON || !scrollver) {
	    XDestroyWindow(display, notadefwin);
	    destroy_window(notadefwin);
	} else {
	    notadef_is_open = MP_True;
	    notadef_rename();
	    unique_number = new_number();
	    XMapSubwindows(display, notadefwin);
	    XMapWindow(display, notadefwin);
	}
    }
}

void notadef_close(void)
{
    XDestroyWindow(display, notadefwin);
    notadef_bad_end(NULL);
}

void notadef_rename(void)
{
    char *tn;
    if (!notadef_is_open) return;
    free(notadef_name);
    notadef_name = concat(translate(NOTADEFNAME), get_notation_filename(edit_fnr));
    tn = (char*)UstrtoLocale(notadef_name);
    if (!XStringListToTextProperty(&tn, 1, &notadefname))
	message(MP_MESSAGE, translate("Can't make Define-name"));
    else
	XSetWMName(display, notadefwin, &notadefname);
}

void notadef_raise(void)
{
    if (!notadef_is_open) return;
    if (notadef_iconized) XMapWindow(display, notadefwin);
    XRaiseWindow(display, notadefwin);
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

static int call_strarg(int (*func)(), void **argl)
{
  return (*func)(*((Char**)argl[0]));
}

static int call_strstrarg(int (*func)(), void **argl)
{
  return (*func)(*((Char**)argl[0]),*((Char**)argl[1]));
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
#define PROSTRARG 3
#define PROSTRSTRARG 4

static PROTOLIST protolist[] = 
{
  { {0}, 0, call_noarg, 0 },
  { {IntType}, 1, call_intarg, 0 },
  { {IntType,IntType}, 2, call_intintarg, 0},
  { {StringType}, 1, call_strarg, 0},
  { {StringType,StringType}, 2, call_strstrarg, 0},
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
    { notadef_insert_string,
      "D_insert_selection",
      "Insert the selection from the window system.",
      PRONOARG },
    /* { construct_argument,
      "universal_argument",
      "",
      PRONOARG }, */
    { transpose_char,
      "D_transpose_char",
      "Exchange the characters left and right of the cursor and move the cursor forward. "
      "Due to the nesting of the special mark up, the function might behave "
      "strange near such mark up.",
      PRONOARG },
    { go_left,
      "D_backward_char",
      "Move the cursor backwards (N times).",
      PROINTARG },
    { go_right,
      "D_forward_char",
      "Move the cursor forward (N times).",
      PROINTARG },
    { insert_return,
      "D_newline",
      "Insert an end of line (N times).",
      PROINTARG },
    { insert_soft_return,
      "D_soft_newline",
      "Insert a soft end of line (N times).",
      PROINTARG },
    { backspace_char,
      "D_backward_delete_char",
      "Remove the character before the cursor (N times).",
      PROINTARG },
    { delete_char,
      "D_delete_char",
      "Remove the character after the cursor (N times).",
      PROINTARG },
    { kill_line,
      "D_kill_line",
      "Remove all the character up to the next end of line or special mark up.",
      PRONOARG },
    { insert_tab,
      "D_insert_tab",
      "Insert a tab character (N times).",
      PROINTARG },
    { insert_backtab,
      "D_insert_backtab",
      "Insert the markup to tab backwards (N times).",
      PRONOARG },
    { insert_settab,
      "D_insert_settab",
      "Insert the markup to set a tab (N times).",
      PROINTARG },
    { insert_expr,
      "D_insert_expression",
      "Insert an expression place holder with index N.",
      PROINTARG },
    { insert_op,
      "D_insert_operator",
      "Insert an operator place holder with index N.",
      PROINTARG },
    { insert_id,
      "D_insert_identifier",
      "Insert an identifier place holder with index N.",
      PROINTARG },
    { insert_text,
      "D_insert_text",
      "Insert an text place holder with index N.",
      PROINTARG },
    { insert_var,
      "D_insert_variable",
      "Insert an variable place holder with index N.",
      PROINTARG },
    { go_home,
      "D_beginning_of_line",
      "Go to the beginning of the line.",
      PRONOARG },
    { go_down,
      "D_next_line",
      "Go to the next formatting string.",
      PROINTARG },
    { go_up,
      "D_previous_line",
      "Go to the previous formatting string.",
      PROINTARG },
    { go_end,
      "D_end_of_line",
      "Go to the end of the line.",
      PRONOARG },
    { move_version_down,
      "D_move_version_down",
      "Move the current version down.",
      PRONOARG },
    { move_version_up,
      "D_move_version_up",
      "Move the current version up.",
      PRONOARG },
    { move_version_first,
      "D_move_version_first",
      "Move the current version to the first position.",
      PRONOARG },
    { move_version_last,
      "D_move_version_last",
      "Move the current version to the last position",
      PRONOARG },
    { move_notation_left,
      "D_move_template_left",
      "Move the selected template left or up (in the stencil window).",
      PRONOARG },
    { move_notation_right,
      "D_move_template_right",
      "Move the selected template right or down (in the stencil window).",
      PRONOARG },
    { move_notation_begin,
      "D_move_template_begin",
      "Move the selected template to the first position (in the stencil window).",
      PRONOARG },
    { move_notation_end,
      "D_move_template_end",
      "Move the selected template to the last position (in the stencil window).",
      PRONOARG },
    { switch_tabmode,
      "D_switch_tabmode",
      "Toggle the display mode: visible mark up codes versus interpreted mark up codes.",
      PRONOARG },
    { ask_selection,
      "insert_selection",
      "Insert the selection from the window system.",
      PRONOARG },
    { set_precedence,
      "D_set_precedence",
      "Set the precedence for the current template.",
      PROINTARG },
    { set_spacing,
      "D_set_spacing",
      "Set the default spacing for the current template.",
      PROINTARG },
    { set_kind,
      "D_set_kind",
      "Set the kind of operator for the current template.",
      PROINTARG },
    { notadef_keysymbol,
      "D_symbol_click",
      "Insert the symbol selected from the symbol window.",
      PRONOARG },
    { notadef_symbol_short,
      "D_insert_symbol",
      "Insert the symbol with a given code (N times).",
      PROINTINTARG },
    { notadef_notation_short,
      "D_insert_template",
      "Insert a version from a template with a given name.",
      PROSTRARG },
    { notadef_selected_notation,
      "D_selected_template",
      "Insert the template selected from a document.",
      PRONOARG },
    { notadef_notation,
      "D_template_click",
      "Insert the template selected form a stencil window.",
      PRONOARG },
    { insert_char,
      "D_self_insert",
      "Insert the given character (N times).",
      PROINTINTARG },
    { insert_gluespace,
      "D_insert_glue_space",
      "Insert the mark up for a stretching space (N times).",
      PROINTARG },
    { insert_glueline,
      "D_insert_glue_line",
      "Insert the mark up for a stretching horizontal line (N times).",
      PROINTARG },
    { insert_gluestipple,
      "D_insert_glue_stipple",
      "Insert the mark up for a stretching stipple line (N times).",
      PROINTARG },
    { insert_font,
      "D_insert_font",
      "Insert the mark up for changing the font.",
      PROINTARG },
    { insert_size,
      "D_insert_size",
      "Insert the mark up for changing to an absolute fontsize.",
      PROINTARG },
    { insert_relative_size,
      "D_insert_relative_size",
      "Insert the mark up for changing the fontsize relatively.",
      PROINTARG },
    { notadef_insert_markup,
      "D_insert_markup",
      "Insert two markup strings, one before the cursor, one after.",
      PROSTRSTRARG },
    { clear_window,
      "D_clear_window",
      "Clear the define window.",
      PRONOARG },
    { clear_version,
      "D_delete_version",
      "Remove the current version.",
      PRONOARG },
    { toggle_latexmode,
      "D_toggle_latexmode",
      "Change the LaTeX mode to be used.",
      PRONOARG },
    { auto_format,
      "D_automatic_output",
      "Restore the automatically generated output for the current format.",
      PRONOARG },
    { 0,0,0,0}
};

void notadef_keyboard(void)
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

