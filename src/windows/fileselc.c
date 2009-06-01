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
#include "button.h"
#include "getstring.h"
#include "scrollbar.h"
/* #include "keymap.h" */
#include "message.h"
#include "output.h"
#include "fstate.h"
#include "fileselc.h"
#include "helpfile.h"

#define FILESELNAME "File Selector"
#define ICONNAME    "FileSel"
#define DIRECTORY   "Filename"
#define FILEMASK    "Mask"
#define DIRECTORIES "Directories"
#define FILES       "Files"
#define BORDERMARGIN 10

enum button { OKBUTTON, RESCANBUTTON, CANCELBUTTON, NR_BUTTON };
static char *filebutton[NR_BUTTON] = { " OK ", " Rescan ", " Cancel " };
static int filehelp[NR_BUTTON] =
{ FILESELCOKHELP, FILESELCRESCANHELP, FILESELCCANCELHELP };

typedef
struct {
    Char *mask;
    FSTATE *statem;
    Window checkwindow;
    void (*ret_func)(void *,Char*);
    void *func_arg;
    Char *description;
    Char *directory;
    Char *dirblock;
    Char **dirlist;
    Char *fileblock;
    Char **filelist;
    int x,y,w,h;
    int ygd,yfm,yd,xf,sh; 
    int dirnr, dirsel;
    int filenr, filesel;
    int visnr;
    Bool dir_select;
    Bool open;
    Bool iconized;
    void *getfile;
    void *getmask;
    void *scrolldir;
    void *scrollfile;
    Window mainwin, dirwin, filewin;
} FILESELECTOR;


static FILESELECTOR fsel;
static XTextProperty fileselc_name, fileselc_icon;
static char *fileselcname=FILESELNAME, *iconname=ICONNAME;

static void fileselc_bad_end(void *data)
{
    Window win = *((Window*)data);
    if (win==fsel.mainwin) {
	free(fsel.description);
	free(fsel.dirlist);
	free(fsel.dirblock);
	free(fsel.filelist);
	free(fsel.fileblock);
	free(fsel.directory);
	free(fsel.mask);
	string_destroy(fsel.getfile);
	string_destroy(fsel.getmask);
	destroy_window(fsel.mainwin);
	fsel.open=MP_False;
	fsel.dir_select=MP_False;
	fsel.mainwin=0;
    }
}

static void draw_nameline(Char *s, int i, int start, int end, int sel)
{
    if (i<start || i>end) return;
    set_x_y(0, (i-start)*line_height());
    thinspace(2);
    if (i==sel) set_text_mode(Reverse);
    out_string(s);
    if (i==sel) set_text_mode(Normal);
    out_char(Newline);
}

static void fileselc_draw(void *data)
{
    Window win = *((Window*)data);
    if (win==fsel.mainwin) {
	Char *c=fsel.description;
	set_output_window(&fsel.mainwin);
	set_margin(BORDERMARGIN);
	set_y(BORDERMARGIN);
	do {
	    out_string(c);
	    out_char(Newline);
	    c = c+Ustrlen(c);
	} while (*c++);
	set_y(fsel.ygd); out_string(translate(DIRECTORY));out_char(Newline);
	set_y(fsel.yfm); out_string(translate(FILEMASK));out_char(Newline);
	set_y(fsel.yd); thinspace(SCROLLBARSIZE+2);out_string(translate(DIRECTORIES));
	thinspace(fsel.xf+SCROLLBARSIZE+2-where_x());
	out_string(translate(FILES));out_char(Newline);
	unset_output_window();
    } else {
	Char **c;
	int i,bl,sl,nl;
	if (win==fsel.dirwin) {
	    c = fsel.dirlist;
	    bl = scrollbar_line(fsel.scrolldir,0);
	    sl = fsel.dirsel;
	    nl = fsel.dirnr;
	} else {
	    c = fsel.filelist;
	    bl = scrollbar_line(fsel.scrollfile,0);
	    sl = fsel.filesel;
	    nl = fsel.filenr;
	}
	set_output_window(&win);
	for (i=0; i<nl; i++) {
	    draw_nameline(c[i], i, bl, bl+fsel.visnr, sl);
	}
	out_char(Newline);
	unset_output_window();
    }
}

static void fileselc_layout_change(void *data __attribute__((unused)))
{
    if (fsel.open) {
	/* fsel.{ygd,yfm,yd,xf,sh}
	** move/resize
	*/
	if (!fsel.iconized) {
	    XClearArea(display, fsel.dirwin, 0,0,0,0,MP_True);
	    XClearArea(display, fsel.filewin, 0,0,0,0,MP_True);
	    XClearArea(display, fsel.mainwin, 0,0,0,0,MP_True);
	}
	fsel.visnr = (fsel.sh+line_height()/2)/line_height();
	scrollbar_linesize(fsel.scrolldir, line_height());
	scrollbar_linesize(fsel.scrollfile, line_height());
    }
}

typedef int (*CHECKTYPE)(Char*, Char*);

static void fileselc_rescan(int completion)
{
  Char *c,*h, *d;
  Char **dl, **fl;
  int l, nd, nf;
  c = standard_dir(string_text(fsel.getfile));
  h = d = string_text(fsel.getmask);
  if (Ustrcmp(d,fsel.mask)) {
    free_fstate(fsel.statem);
    fsel.statem = make_fstate(d,MP_True);
    h = fsel.mask;
    fsel.mask=d;
  }
  free(h);h=c;
  if (Ustrcmp(c,fsel.directory)) {
    if (completion || !is_directory(c)) {
      Char *g = Ustrrchr(c,DIRSEPCHAR);
      if (g) *g='\0';
    }
    if (is_directory(c)) {
      h = fsel.directory;
      fsel.directory=c;
    }
  }
  free(h);
  set_wait_cursor(fsel.mainwin);
  l = read_dir_contents(fsel.directory, MP_False, (CHECKTYPE) fstate_check,
			(Char*) fsel.statem,
			&dl, &nd, &fl, &nf);
  remove_wait_cursor();
  if (l) {
    free(fsel.dirblock);
    free(fsel.dirlist);
    free(fsel.fileblock);
    free(fsel.filelist);
    fsel.dirlist= (nd?dl:NULL);
    if (fsel.dirlist) fsel.dirblock=fsel.dirlist[0]; else fsel.dirblock=0;
    fsel.dirnr=nd;
    fsel.filelist=(nf?fl:NULL);
    if (fsel.filelist) fsel.fileblock=fsel.filelist[0]; else fsel.fileblock=0;
    fsel.filenr=nf;
  }
}

void fileselc_update_window(void)
{
  scrollbar_set(fsel.scrolldir, 0, fsel.dirnr);
  scrollbar_set(fsel.scrollfile, 0, fsel.filenr);	
  XClearWindow(display, fsel.dirwin);
  XClearWindow(display, fsel.filewin);
  fsel.dirsel=-1;
  fileselc_draw(&fsel.dirwin);
  fileselc_draw(&fsel.filewin);
}


static void fileselc_handle_button(void *data __attribute__((unused)), int b_num)
{
    Char *c,*d,*h;
    Char **dl, **fl;
    int l,nd,nf;
    switch (b_num) {
    case OKBUTTON:
	c = standard_dir(string_text(fsel.getfile));
	if (is_directory(c) && !fsel.dir_select) {
	    d=string_text(fsel.getmask);
	    if (Ustrcmp(c,fsel.directory) || Ustrcmp(d,fsel.mask))
		fileselc_handle_button(NULL, RESCANBUTTON);
	    free(c);
	    free(d);
	} else {
	    set_wait_cursor(fsel.mainwin);
	    failure=MP_False;
	    if (exist_window(fsel.checkwindow))
		fsel.ret_func(fsel.func_arg, c);
	    if (!failure) {
		XDestroyWindow(display, fsel.mainwin);
		fileselc_bad_end(&fsel.mainwin);
	    } else remove_wait_cursor();
	}
	break;
    case RESCANBUTTON:
      fileselc_rescan(0);
      string_refresh(fsel.getfile, fsel.directory);
      fileselc_update_window();
      break;
    case CANCELBUTTON:
	XDestroyWindow(display, fsel.mainwin);
	fileselc_bad_end(&fsel.mainwin);
	break;
    default:
	break;
    }
}

static int reselect(Char **c, int oldsel, int newsel, int start, int end)
{
    Char *h=NULL,*g=NULL;
    int i=0;

    if (oldsel==newsel) return oldsel;
    if (!c || newsel<start || newsel>=end) newsel=-1;
    while (i<=oldsel || i<=newsel) {
	if (i==newsel) g=c[i];
	if (i==oldsel) h=c[i];
	i++;
    }
    if (h) draw_nameline(h, oldsel, start, end, newsel);
    if (g) draw_nameline(g, newsel, start, end, newsel);
    return newsel;
}

static void fileselc_motion(void *data, int x __attribute__((unused)), int y)
{
    Window win = *((Window*)data);
    if (fsel.dirsel>=0 && fsel.dirwin != win) {
	set_output_window(&fsel.dirwin);
	fsel.dirsel = reselect(fsel.dirlist, fsel.dirsel, -1,
			       scrollbar_line(fsel.scrolldir,0), fsel.dirnr);
	unset_output_window();
    }
    if (fsel.filesel>=0 && fsel.filewin !=win) {
	set_output_window(&fsel.filewin);
	fsel.filesel = reselect(fsel.filelist, fsel.filesel, -1,
				scrollbar_line(fsel.scrollfile,0),fsel.filenr);
	unset_output_window();
    }
    if (fsel.dirwin == win) {
	int j,k,n;
	set_output_window(&win);
	k = scrollbar_line(fsel.scrolldir,0);
	j = y/line_height()+k;
	n = k+fsel.visnr;
	if (n>fsel.dirnr) n=fsel.dirnr;
	fsel.dirsel = reselect(fsel.dirlist,fsel.dirsel,j,k,n);
	unset_output_window();
    } else if (fsel.filewin==win) {
	int j,k,n;
	set_output_window(&win);
	k = scrollbar_line(fsel.scrollfile,0);
	j = y/line_height()+k;
	n = k+fsel.visnr;
	if (n>fsel.filenr) n=fsel.filenr;
	fsel.filesel = reselect(fsel.filelist,fsel.filesel,j,k,n);
	unset_output_window();
    }
}

static void fileselc_double_click(void *data)
{
    Window win = *((Window*)data);
    if (win==fsel.filewin || win==fsel.dirwin)
	get_motion_hints(win,-1);
}

static void fileselc_press(void *data, XButtonEvent *event)
{
    Window win = *((Window*)data);
    fileselc_motion(data, event->x, event->y);
    if (win==fsel.filewin || win==fsel.dirwin)
	get_motion_hints(win,-1);
}

static void fileselc_release(void *data, XButtonEvent *event __attribute__((unused)))
{
    Window win = *((Window*)data);
    if (win==fsel.filewin || win==fsel.dirwin)
	stop_motion_hints();
    if (win==fsel.dirwin) {
	Char *c;
	Char *h=(Char*)malloc(1024 * sizeof(Char));
	int i=0;
	Ustrcpy(h,fsel.directory);
	i = Ustrlen(h);
	if (fsel.dirsel>=0) {
	    if (i && h[i-1]!='/') h[i++]='/';
	    Ustrcpy(h+i,fsel.dirlist[fsel.dirsel]);
	}
	c=standard_dir(h);
	string_refresh(fsel.getfile, c);
	if (double_click)
	    fileselc_handle_button(data, RESCANBUTTON);
    } else if (win==fsel.filewin) {
	Char h[1024];
	int i=0;
	Ustrcpy(h,fsel.directory);
	i = Ustrlen(h);
	if (fsel.filesel>=0) {
	    if (h[i-1]!='/') h[i++]='/';
	    Ustrcpy(h+i,fsel.filelist[fsel.filesel]);
	}
	string_refresh(fsel.getfile, h);
	if (double_click)
	    fileselc_handle_button(data, OKBUTTON);
    }
}

static void fileselc_resize(void *data __attribute__((unused)), XConfigureEvent *event)
{
    if (event->window == fsel.mainwin) {
	int nsh,nxf,dx,dy,xg,i;
	window_manager_added(fsel.mainwin, &dx, &dy);
	xg = string_width(translate(DIRECTORY),-1);
	i = string_width(translate(FILEMASK), -1);
	if (i>xg) xg = i;
	xg +=2*BORDERMARGIN;
	fsel.x = event->x-dx;
	fsel.y = event->y-dy;
	fsel.w = event->width;
	fsel.h = event->height;
	xg = fsel.w - BORDERMARGIN-xg;
	if (xg<10) xg=10;
	string_resize(fsel.getfile, xg);
	string_resize(fsel.getmask, xg);
	nsh = event->height-fsel.yd-BORDERMARGIN*2-line_height()-button_height;
	nxf = (event->width - 3*BORDERMARGIN)/2+BORDERMARGIN;
	if (nsh!=fsel.sh || nxf!=fsel.xf) {
	    fsel.sh=nsh;
	    fsel.xf=nxf;
	    fsel.visnr = (nsh+line_height()/2)/line_height();
	    scrollbar_resize(fsel.scrolldir, fsel.sh);
	    scrollbar_resize(fsel.scrollfile, fsel.sh);
	    scrollbar_move(fsel.scrollfile, BORDERMARGIN+nxf,
			   fsel.yd+line_height());
	    XResizeWindow(display, fsel.dirwin,
			  nxf-SCROLLBARSIZE-2-BORDERMARGIN-2, nsh-2);
	    XResizeWindow(display, fsel.filewin,
			  nxf-SCROLLBARSIZE-2-BORDERMARGIN-2, nsh-2);
	    XMoveWindow(display, fsel.filewin, BORDERMARGIN+nxf+
			SCROLLBARSIZE+2,fsel.yd+line_height());
	}
    }
}

static void fileselc_scrollto(void *data, int kind __attribute__((unused)))
{
    fileselc_draw(data);
}

static void fileselc_iconize(void *data __attribute__((unused)))
{
    fsel.iconized=MP_True;
}

static void fileselc_deiconize(void *data __attribute__((unused)))
{
    fsel.iconized=MP_False;
}

static int fileselc_last_pos(int *x, int *y, int *w, int *h)
{
    *x=fsel.x;
    *y=fsel.y;
    *w=fsel.w;
    *h=fsel.h;
    return MP_False;
}

static void fileselc_set_last_pos(int x, int y, int w, int h)
{
    fsel.x=x;
    fsel.y=y;
    fsel.w=w;
    fsel.h=h;
}

static void fileselc_OK_action(void)
{
    fileselc_handle_button(NULL, OKBUTTON);
}

static void fileselc_rescan_action(void)
{
    fileselc_handle_button(NULL, RESCANBUTTON);
}

static void fileselc_completion(void)
{
  Char *c, *longestmatch;
  int l,n,i,longestlen,clen;

  c = standard_dir(string_text(fsel.getfile));
  fileselc_rescan(1);
  l = Ustrlen(fsel.directory);
  /* handle incorrect completions, leave content in file edit field */
  if (Ustrncmp(fsel.directory, c,l)) {
    message(MP_ERROR,
	    translate("No file completion due to incorrect directory."));
    return;
  }
  if (c[l]==DIRSEPCHAR) l++;
  if (Ustrchr(c+l, DIRSEPCHAR)) {
    message(MP_ERROR,
	    translate("No file completion due to missing subdirectory."));
    return;
  }
  /* filter out all name not starting with the characters behond c[l] */
  longestmatch = NULL;
  longestlen=0;
  clen=Ustrlen(c+l);
  n=0;
  for (i=0; i<fsel.dirnr; i++) {
    if (!Ustrcmp(fsel.dirlist[i], translate(".."))) {
      /* ignore ".." */
      fsel.dirlist[n++] = fsel.dirlist[i];
    } else if (!Ustrncmp(c+l, fsel.dirlist[i], clen)) {
      fsel.dirlist[n++] = fsel.dirlist[i];
      if (!longestmatch) {
	longestmatch = fsel.dirlist[i];
	longestlen = Ustrlen(longestmatch);
      } else {
	int j;
	for (j=0;
	     j<longestlen && fsel.dirlist[i][j]==longestmatch[j];
	     j++);
	longestlen=j;
      }
    }
  }
  fsel.dirnr = n;
  n=0;
  for (i=0; i<fsel.filenr; i++) {
    if (!Ustrncmp(c+l, fsel.filelist[i], clen)) {
      fsel.filelist[n++] = fsel.filelist[i];
      if (!longestmatch) {
	longestmatch = fsel.filelist[i];
	longestlen = Ustrlen(longestmatch);
      } else {
	int j;
	for (j=clen;
	     j<longestlen && fsel.filelist[i][j]==longestmatch[j];
	     j++);
	longestlen=j;
      }
    }
  }
  fsel.filenr = n;
  if (longestlen>clen) {
    /* additional characters available, update string field */
    Char *result;
    int dirlen;
    dirlen=Ustrlen(fsel.directory);+longestlen+3;
    result = malloc(sizeof(Char)*(dirlen+longestlen+2));
    Ustrcpy(result, fsel.directory);
    if (result[dirlen-1]!=DIRSEPCHAR) {
      result[dirlen++]=DIRSEPCHAR;
    }
    Ustrncpy(result+dirlen, longestmatch, longestlen);
    if (fsel.filenr==0 && fsel.dirnr==2) {
      result[dirlen+longestlen]=DIRSEPCHAR;
      dirlen++;
    }
    result[dirlen+longestlen]=0;
    string_refresh(fsel.getfile, result);
  }
  if (fsel.filenr==0 && fsel.dirnr==2) {
    fileselc_handle_button(0, RESCANBUTTON);
  } else {
    fileselc_update_window();
  }
}

FUNCTIONS fileselcfuncs = {
    fileselc_bad_end, fileselc_draw, fileselc_resize, fileselc_press,
    fileselc_release, fileselc_motion, fileselc_iconize, fileselc_deiconize,
    NULL, NULL, fileselc_layout_change, NULL, NULL, NULL, NULL,
    NULL, fileselc_last_pos, fileselc_set_last_pos, fileselc_double_click };



#include "language.h"

static int call_noarg(int (*func)(), void **argl __attribute__((unused)))
{
  return (*func)();
}

void fileselc_init(void)
{
    void *pt;
    fsel.open = MP_False;
    fsel.w=display_width/3;
    fsel.h=display_height/3;
    fsel.x=0;fsel.y=0;
    if (!XStringListToTextProperty(&fileselcname, 1, &fileselc_name))
        message(MP_EXIT-1,translate("Can't set the name for the file selector."));
    if (!XStringListToTextProperty(&iconname, 1, &fileselc_icon))
        message(MP_EXIT-1, translate("Can't set iconname for the file selector."));
    pt = define_prototype(0,0,0,call_noarg);
    define_function("fileselc_OK_action",
		    "Perform the OK action from the file selector.",
		    pt, fileselc_OK_action);
    define_function("fileselc_rescan_action",
		    "Perform the rescan action from the file selector.",
		    pt, fileselc_rescan_action);
    define_function("fileselc_completion",
		    "Perform the default action from the file selector.",
		    pt, fileselc_completion);

}

void fileselc_open(void (*func)(void*,Char*), void *arg, Char *descript,
		   Char *dir, Char *mask, Char *deffile,
		   Window checkw)
{
    int tw,th, dw,i,j,xg;
    XSetWindowAttributes attr;
    XSizeHints hints;
    Char *c;

    if (fsel.open) {
	XDestroyWindow(display, fsel.mainwin);
	fileselc_bad_end(&fsel.mainwin);
    }
    fsel.ret_func=func;
    fsel.func_arg=arg;
    fsel.checkwindow=checkw;
    fsel.description = concat(descript,NULL);
    fsel.directory = standard_dir(concat(dir,NULL));
    fsel.mask = concat(mask,NULL);
    fsel.dirlist = fsel.filelist = NULL;
    fsel.dirblock = fsel.fileblock = NULL;
    c = fsel.description;
    set_output_window(test_window());
    set_drawstyle(INVISIBLE);
    set_x_y(0,0);
    tw=th=0;
    while (*c) {
	if (*c=='\n') {
	    if (where_x()>tw) tw = where_x();
	    out_char(Newline);
	} else
	    out_char(*((unsigned char*)c));
	c++;
    }
    if (where_x()>tw) tw=where_x();
    out_char(Newline);
    unset_output_window();
    th = where_y();
    if (fsel.w<tw+2*BORDERMARGIN) fsel.w=tw+2*BORDERMARGIN;
    fsel.ygd = th+3*BORDERMARGIN/2;
    fsel.yfm = fsel.ygd+line_height()+BORDERMARGIN/2;
    fsel.yd = 2*fsel.yfm-fsel.ygd;
    j=BORDERMARGIN;
    for (i=0; i<NR_BUTTON; i++) j+=button_width(translate(filebutton[i]))+BORDERMARGIN;
    if (fsel.w<j) fsel.w=j;
    dw=string_width(translate(DIRECTORIES),-1);
    if ((i=string_width(translate(FILES),-1)) >dw) dw=i;
    i = dw*2+BORDERMARGIN*3+SCROLLBARSIZE*2;
    if (fsel.w< i) fsel.w=i;
    fsel.xf = (fsel.w-BORDERMARGIN)/2+BORDERMARGIN;
    i=fsel.h-fsel.yd-line_height()-BORDERMARGIN*2-button_height;
    j=line_height()*5;
    if (i<j) {
	fsel.h += j-i;
	fsel.sh = j;
    } else
	fsel.sh = i;
    xg=string_width(translate(DIRECTORY),-1);
    if ((i=string_width(translate(FILEMASK),-1)) >xg) xg=i;
    xg+=2*BORDERMARGIN;
    attr.background_pixel = white_pixel;
    attr.border_pixel = black_pixel;
    attr.colormap = colormap;
    attr.bit_gravity = NorthWestGravity;
    attr.event_mask = (ExposureMask | ButtonPressMask
		       | ButtonReleaseMask | ButtonMotionMask
		       | PointerMotionHintMask | KeyPressMask
		       | FocusChangeMask
		       | StructureNotifyMask | VisibilityChangeMask);
    fsel.mainwin = XCreateWindow(display, root_window,
				 fsel.x, fsel.y, fsel.w, fsel.h,
				 BORDERWIDTH, CopyFromParent, InputOutput,
				 visual,
				 (CWBackPixel | CWBorderPixel | CWColormap |
				  CWBitGravity | CWEventMask), &attr);
    hints.flags = PPosition | PSize;
    XSetWMProperties(display, fsel.mainwin, &fileselc_name, &fileselc_icon,
		     NULL, 0, &hints, &wm_hints, &class_hints);
    set_protocols(fsel.mainwin);
    if (add_window(fsel.mainwin, FILESELCWINDOW, root_window,
		   NULL, translate(helpname[FILESELCHELP]))) {
	int x,y,w;
	y=fsel.h-button_height-BORDERMARGIN;
	x=BORDERMARGIN;
	i=0;
	button_stick(SouthWestGravity);
	while (i<NR_BUTTON && button_make(i,fsel.mainwin, translate(filebutton[i]),&x,y,
					  (i?1:2), NULL, helpname[filehelp[i]],
					  NULL, NULL, NULL,
					  fileselc_handle_button, NULL, NULL))
	    i++,x+=BORDERMARGIN;
	button_stick(NorthWestGravity);
	x=BORDERMARGIN;
	y=fsel.yd+line_height();
	w=fsel.w-BORDERMARGIN-2-fsel.xf;
	if (i==NR_BUTTON &&
	    (fsel.scrolldir = scrollbar_make(VERTICAL_SHORT, fsel.mainwin,
					     x,y,fsel.sh, line_height(),
					     fileselc_scrollto,
					     (void*)(&fsel.dirwin))))
	    i++;
	x+=SCROLLBARSIZE+2;
	if (i==NR_BUTTON+1) {
	    fsel.dirwin = XCreateWindow(display, fsel.mainwin,
					x, y, w, fsel.sh-2, 1,
					CopyFromParent, InputOutput,
					visual,
					(CWBackPixel | CWBorderPixel |
					 CWBitGravity | CWEventMask), &attr);
	    if (add_window(fsel.dirwin, FILESELCWINDOW, fsel.mainwin,
			   NULL, translate(helpname[FILESELCHELP])))
		i++;
	}
	x=fsel.xf;
	if (i==NR_BUTTON+2 &&
	    (fsel.scrollfile = scrollbar_make(VERTICAL_SHORT, fsel.mainwin,
					      x,y,fsel.sh, line_height(),
					      fileselc_scrollto,
					      (void*)(&fsel.filewin))))
	    i++;
	x+=SCROLLBARSIZE+2;
	if (i==NR_BUTTON+3) {
	    fsel.filewin = XCreateWindow(display, fsel.mainwin,
					 x,y, w, fsel.sh-2, 1,
					 CopyFromParent, InputOutput,
					 visual,
					 (CWBackPixel | CWBorderPixel |
					  CWBitGravity | CWEventMask), &attr);
	    if (add_window(fsel.filewin, FILESELCWINDOW, fsel.mainwin,
			   NULL, translate(helpname[FILESELCHELP])))
		i++;
	}
	w=fsel.w-BORDERMARGIN-xg;
	if (deffile) {
	    Char *h=fsel.directory+Ustrlen(fsel.directory)-1;
	    if (*h!='/') {
		h=concat(fsel.directory,translate(DIRSEPSTR));
		c=concat(h,deffile);
		free(h);
	    } else {
		c=concat(fsel.directory, deffile);
	    }
	} else c=fsel.directory;
	if (i==NR_BUTTON+4 &&
	    (fsel.getfile = string_make(fsel.mainwin, c, 1024,
					w, helpname[FILESELCSELCHELP],
					xg, fsel.ygd, MP_False)))
	    i++;
	if (c!=fsel.directory) free(c);
	if (i==NR_BUTTON+5 &&
	    (fsel.getmask = string_make(fsel.mainwin, fsel.mask, 1024,
					w, helpname[FILESELCMASKHELP],
					xg, fsel.yfm, MP_False)))
	    i++;
    }
    if (i<NR_BUTTON+6) {
	XDestroyWindow(display, fsel.mainwin);
	destroy_window(fsel.mainwin);
    } else {
	string_relation(fsel.getfile, fsel.getmask, fsel.getmask);
	string_relation(fsel.getmask, fsel.getfile, fsel.getfile);
	string_get_input(fsel.getfile);
	string_map(fsel.getmask);
	string_map(fsel.getfile);
	fsel.iconized = MP_False;
	fsel.open = MP_True;
	fsel.statem=make_fstate(fsel.mask,MP_True);
	i=read_dir_contents(fsel.directory, MP_False, (CHECKTYPE) fstate_check,
			    (Char*) fsel.statem, &fsel.dirlist, &fsel.dirnr,
			    &fsel.filelist, &fsel.filenr);
	if (fsel.dirlist)
	  fsel.dirblock=fsel.dirlist[0];
	else
	  fsel.dirblock=0;
	if (fsel.filelist)
	  fsel.fileblock=fsel.filelist[0];
	else
	  fsel.fileblock=0;
	scrollbar_set(fsel.scrollfile, 0, fsel.filenr);
	scrollbar_set(fsel.scrolldir, 0, fsel.dirnr);
	fsel.dirsel=-1;
	fsel.filesel=-1;
	XMapSubwindows(display, fsel.mainwin);
	XMapWindow(display, fsel.mainwin);
    }
}

void dirselc_open(void (*func)(void*,Char*), void *arg, Char *descript,
		   Char *dir, Char *mask, Char *deffile,
		   Window checkw)
{
  fsel.dir_select=MP_True;
  fileselc_open(func, arg, descript, dir, mask,deffile, checkw);
}
