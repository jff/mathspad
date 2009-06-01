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
**   File : edit.c
**   Datum: 11-4-92
**   Doel : Het open en verwerken van gegevens voor een file dat
**          is gekoppeld aan een edit-window. Er kunnen meerdere
**          edit-windows open zijn, met samen 1 primaire en 1
**          secundaire selectie.
*/
#include <stdlib.h>
#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
/* #include "keymap.h" */
#include "message.h"
#include "button.h"
#include "scrollbar.h"
#include "remark.h"
#include "output.h"
#include "latexout.h"
#include "fileread.h"
#include "notatype.h"
#include "edit.h"
#include "editor.h"
#include "menu.h"
#include "fileselc.h"
#include "helpfile.h"
#include "popup.h"
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>


#define EDITNAME  "MathSfile : "
#define EMPTYFILE "noname"
#define LENEMPTYFILE 6
#define CHANGED   " (modified)"
#define VIEWCOM   " (view)"
#define DONECOM   " (done)"
#define RUNCOM    " (running)"
#define EXTENSION ".mpd"

enum button { LOADBUTTON,  SAVEBUTTON,    RENAMEBUTTON,
	      OUTPUTBUTTON, INCLUDEBUTTON, DONEBUTTON, NR_BUTTON  };
#define ASCIIBUTTON NR_BUTTON
static
int perm[NR_BUTTON+5] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

static
char *editbutton[NR_BUTTON+1] = { "Load",  "Save", "Rename",
					"Output", "Include", "Done",
					"Output" };
static
int edithelp[NR_BUTTON+1] =
{ EDITLOADHELP, EDITSAVEHELP, EDITRENAMEHELP, EDITOUTPUTHELP,
  EDITINCLUDEHELP, EDITDONEHELP, EDITOUTPUTHELP };

static char *donesave8[] = { " Yes ", " No ", " Cancel ", 0 };
static Char *donesave[] = {0,0,0,0};

static
struct { char *lines;
	 char *extension;
     } textremark[NR_BUTTON+5] = {
	 {"Load document:", "*.mpd"},
	 {"Save document:", "*.mpd"},
	 {"Rename document:", "*.mpd"},
	 {"Make output file:", "*.tex"},
	 {"Include document:", "*.mpd"},
	 {"File not saved!\nSave it?", " Yes \n No \n Cancel "},
	 {"Make output file:", "*"},
	 {"Make LaTeX output file:", "*.tex"},
	 {"Make pe output file:", "*.tex"},
	 {"Make plain LaTeX output file:", "*.tex"},
	 {"Make Ascii output file:", "*"}};

/*
static void handle_output_popup(void*, int);
static Char oname1[6] = { 'L', 'a', 'T','e', 'X', 0 };
static Char oname2[6] = { 'P', 'l', 'a','i', 'n', 0 };
static Char oname3[6] = { 'A', 's', 'c','i', 'i', 0 };
static
MENULINE outputlines[3] =
{ { oname1 ,5,0,0,handle_output_popup,NULL,MPTEX, NULL},
  { oname2 ,5,0,0,handle_output_popup,NULL,PLAINTEX, NULL},
  { oname3 ,5,0,0,handle_output_popup,NULL,ASCII, NULL}};
*/
static MENU outputmenu;

typedef struct { Window win_id, drawwin_id;
                 /* tekstboom, windowtekst, aantal regels, ... */
                 Bool saved, iconized, auto_saved, view_mode, empty,
		 shell,fini,strt;
		 pid_t pid;
		 int xpos, ypos;
                 unsigned int width, height, buflen;
		 int (*callback)(unsigned char*,unsigned int*);
                 void *info;
                 Char *headername, *filename, *pathname, *outputname;
		 unsigned char *prcsbuf;
                 void *scrollver, *scrollhor;    /* gegevens van scrollbars */
               } EDITINFO;

/*
** newname wordt gebruikt om de string van remark terug te krijgen.
** newname is alleen niet NULL als functie edit_handle_remark
** wordt aangeroepen
*/

static unsigned long edit_mask;
static XSetWindowAttributes edit_attr;
static Char *newname = NULL;
static int number_open=0;
static int number_icon=0;
static int is_opened=MP_False;
static int last_xpos =0, last_ypos = 0, last_width = 0, last_height = 0;
static Bool state_open = MP_False, as_icon = MP_False;
static EDITINFO *state_window = NULL;
static Bool change_check = MP_True;
static Atom inputat, outputat, intestat, outtestat, textat, procesidat;

#define sub_width(A)   (A) - INTERSPACE*3 -SCROLLBARSIZE
#define sub_height(A)  (A) - INTERSPACE*4 -SCROLLBARSIZE - (int) button_height
#define pos_x_with     INTERSPACE*2 +SCROLLBARSIZE
#define pos_x_without  INTERSPACE
#define pos_y_with     INTERSPACE*3 +SCROLLBARSIZE + (int) button_height
#define pos_y_without  INTERSPACE*2 + (int) button_height

static Char buffer[2000];

static void remove_auto_save(EDITINFO *einf)
{
    buffer[0] = '\0';
    Ustrcat(buffer, userdir);
    Ustrcat(buffer, translate("/#"));
    Ustrcat(buffer, strip_name(einf->filename));
    Ustrcat(buffer, translate("#" EXTENSION));
    remove_file(buffer);
}

static void edit_auto_save(void *data, int dump)
{
    EDITINFO *einf = (EDITINFO *) data;
    FILE *f;
    /* Using dump%s to keep a possible #%s# as extra backup */

    if (!einf->auto_saved && !einf->empty) {
      if (dump) {
	buffer[0]=0;
	Ustrcat(buffer, translate("dump"));
	Ustrcat(buffer, strip_name(einf->filename));
	Ustrcat(buffer, translate(EXTENSION));
      } else {
	buffer[0]=0;
	Ustrcat(buffer, translate("#"));
	Ustrcat(buffer, strip_name(einf->filename));
	Ustrcat(buffer, translate("#" EXTENSION));
      }
	f = open_dirfile(userdir, buffer, "wb");
	if (f) {
	    set_file(f);
	    put_filecode(DOCUMENTFILE);
	    save_editwindow(einf->info);
	    unset_file();
	    cleanup_stencilstack();
	    fclose(f);
	    einf->auto_saved = MP_True;
	} else failure=MP_True;
    }
    /* 
    ** In case of a failure, the document should be saved
    ** in another file, to make sure nothing gets lost.
    **
    */
}

static int set_name(void *data, Char *pathname)
{
    EDITINFO *einf = (EDITINFO *) data;
    XTextProperty prop_name;
    Char *name;
    int namesize;
    Char *stripname, *nname;

    if (pathname == NULL) {
	EDITINFO *tinf;
	FlexArray istck;
	int i=0,j;
	int_init(istck);
	while (aig(tinf=(EDITINFO*)next_data_with_type(MAINEDITWINDOW, &i))) {
	  if (!Ustrncmp(translate(EMPTYFILE),tinf->filename, Ustrlen(translate(EMPTYFILE)))) {
	    j = Ustrtol(tinf->filename+Ustrlen(translate(EMPTYFILE)), NULL, 10);
	    int_add(istck, j);
	  }
	  i++;
	}
	i=1;
	while (int_contains(istck,i)) i++;
	int_clear(istck);
	nname = (Char *) malloc(sizeof(Char)*(Ustrlen(userdir) +
					      Ustrlen(translate("/" EMPTYFILE))
					      + 5 +Ustrlen(translate(EXTENSION))));
	concat_in(nname, userdir, translate("/" EMPTYFILE));
	stripname = nname + Ustrlen(nname);
	{ Char sb[40];
	  Char *s;
	  sb[39]=0;
	  s=Ultostr(i,sb+39);
	  Ustrcat(stripname,s);
	}
	Ustrcat(stripname, translate(EXTENSION));
    } else
        nname = pathname;
    stripname = concat(strip_name(nname),NULL);
    if (!Ustrcmp(stripname+Ustrlen(stripname)-Ustrlen(translate(EXTENSION)),
		 translate(EXTENSION)))
	stripname[Ustrlen(stripname)-Ustrlen(translate(EXTENSION))] = 0;
    namesize = Ustrlen(translate(EDITNAME)) + Ustrlen(stripname) + 1 +
	(einf->saved ? 0 : Ustrlen(translate(CHANGED))) +
	(einf->view_mode && !einf->shell ? Ustrlen(translate(VIEWCOM)) : 0) +
	(einf->shell && !einf->fini ? Ustrlen(translate(RUNCOM)) : 0) +
	(einf->shell && einf->fini ? Ustrlen(translate(DONECOM)) : 0);
    name = (Char *) malloc((size_t) namesize*sizeof(Char) );
    if (name) {
	name[0]= '\0';
	Ustrcat(name, translate(EDITNAME));
	Ustrcat(name, stripname);
	if (!einf->saved && !einf->shell) Ustrcat(name, translate(CHANGED));
	if (einf->view_mode && !einf->shell) Ustrcat(name, translate(VIEWCOM));
	if (einf->shell && !einf->fini) Ustrcat(name, translate(RUNCOM));
	if (einf->shell && einf->fini) Ustrcat(name, translate(DONECOM));
    }
    {
      char *n;
      n= (char*)UstrtoLocale(name);
      if (!name || !XStringListToTextProperty(&n, 1, &prop_name)) {
	message(MP_ERROR, translate("No location for editname."));
	return 0;
      }
    }
    XSetWMName(display, einf->win_id, &prop_name);
    free(einf->headername);
    if (einf->pathname!=nname) free(einf->pathname);
    free(einf->filename);
    einf->headername = name;
    einf->filename = stripname;
    einf->pathname = nname;
    {
      char *icn;
      icn = (char*)UstrtoLocale(stripname);
      if (!XStringListToTextProperty(&icn, 1, &prop_name)) {
	message(MP_ERROR, translate("No location for editicon."));
	return 0;
      }
    }
    XSetWMIconName(display, einf->win_id, &prop_name);
    return 1;
}

static void set_output_name(EDITINFO *einf)
{
    int i=0,l;
    if (einf->outputname) free(einf->outputname);
    i= Ustrlen(latexdir);
    l = i+Ustrlen(einf->filename)+15;
    einf->outputname = (Char*) malloc(sizeof(Char)*l);
    if (latexdir[i-1]=='/')
      concat_in(einf->outputname,latexdir,einf->filename);
    else {
      Ustrcpy(einf->outputname,latexdir);
      einf->outputname[i]='/';
      einf->outputname[i+1]=0;
      Ustrcat(einf->outputname, einf->filename);
    }
    if (output_mode==ASCII)
      Ustrncat(einf->outputname, translate(".asc"),l);
    else
      Ustrncat(einf->outputname, translate(".tex"),l);
}

#define BINDOC 1
#define OLDDOC 2
#define NEWDOC 3

static int test_file(FILE *f)
{
    int i;
    i = fgetc(f);
    if (i=='B') {
	if (missing_font(f)==MP_EXIT) {
	    rewind(f);
	    return BINDOC;
	} else
	    return OLDDOC;
    } else {
	rewind(f);
	return (i==27 ? NEWDOC: BINDOC);
    }
}

static int check_name(EDITINFO *einf, Char *name)
{
    int i=0,found=0;
    EDITINFO *tinf;
    if (!einf || !name) return 0;
    do {
      tinf=(EDITINFO*) next_data_with_type(MAINEDITWINDOW, &i);
      found = (tinf && tinf!=einf && !tinf->view_mode && 
	       !Ustrcmp(name, tinf->pathname));
      i++;
    } while (!found && tinf);
    if (found) {
	if (tinf->iconized)
	    XMapWindow(display, tinf->win_id);
	XRaiseWindow(display, tinf->win_id);
    }
    return found;
}

static void handle_filename(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (name) {
	FILE *f;
	int i;
	int check_found = check_name(einf, name);

	if (!(f = fopen((char*)UstrtoFilename(name),"rb"))) {
	    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
	    free(name);
	    failure=MP_True;
	    return;
	}
	i = test_file(f);
	set_wait_cursor(einf->win_id);
	switch (i) {
	case BINDOC:
	    message(MP_MESSAGE, translate("Loading ascii file."));
	    read_file(f,BINARYFILE);
	    unset_file();
	    load_editwindow(einf->info);
	    break;
	case OLDDOC:
	    i = edit_fnr;
	    edit_fnr = 0;
	    load_notation_filenames(f);
	    old_load_editwindow(einf->info,f);
	    edit_fnr = i;
	    if (!state_open) clear_file_ref();
	    break;
	case NEWDOC:
	    i = edit_fnr;
	    edit_fnr = 0;
	    read_file(f,DOCUMENTFILE);
	    unset_file();
	    load_editwindow(einf->info);
	    edit_fnr = i;
	    break;
	default: break;
	}
	fclose(f);
	cleanup_nodestack();
	cleanup_filestack();
	cleanup_stencilstack();
	einf->saved = MP_True;
	einf->auto_saved = MP_True;
	einf->view_mode = check_found;
	set_name(einf, name);
	set_output_name(einf);
	name = NULL;
	remove_wait_cursor();
	if (check_found)
	    message(MP_CLICKREMARK, translate("The document is already loaded.\n"
		    "This copy has been loaded in view mode\n"
		    "in order to ensure that only one\n"
		    "backup is made."));
	return;
    }
    free(name);
}

static void handle_view_filename(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (name) {
	FILE *f;
	int i;

	if (!(f = fopen((char*)UstrtoFilename(name),"rb"))) {
	    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
	    free(name);
	    failure=MP_True;
	    return;
	}
	i=test_file(f);
	set_wait_cursor(einf->win_id);
	switch (i) {
	case BINDOC:
	    message(MP_MESSAGE, translate("Viewing ascii file."));
	    read_file(f,BINARYFILE);
	    unset_file();
	    load_editwindow(einf->info);
	    break;
	case OLDDOC:
	    i = edit_fnr;
	    edit_fnr = 0;
	    view_notation_filenames(f);
	    old_load_editwindow(einf->info,f);
	    edit_fnr = i;
	    break;
	case NEWDOC:
	    i = edit_fnr;
	    edit_fnr = 0;
	    read_file(f,DOCUMENTFILE);
	    unset_file();
	    load_editwindow(einf->info);
	    edit_fnr = i;
	    break;
	default: break;
	}
	fclose(f);
	cleanup_filestack();
	cleanup_stencilstack();
	cleanup_nodestack();
	einf->saved = MP_True;
	einf->auto_saved = MP_True;
	einf->view_mode = MP_True;
	set_name(einf, name);
	set_output_name(einf);
	remove_wait_cursor();
	return;
    }
    free(name);
}

static void handle_include_filename(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;
    FILE *f;
    int i;

    if (!name) return;
    if (!(f = fopen((char*)UstrtoFilename(name),"rb"))) {
	message2(MP_CLICKREMARK, translate("Unable to open file "), name);
	free(name);
	failure=MP_True;
	return;
    }
    i=test_file(f);
    set_wait_cursor(einf->win_id);
    switch (i) {
    case BINDOC:
	message(MP_MESSAGE, translate("Including ascii file."));
	read_file(f,BINARYFILE);
	unset_file();
	include_editwindow(einf->info);
	break;
    case OLDDOC:
	i = edit_fnr;
	edit_fnr = 0;
	load_notation_filenames(f);
	old_include_editwindow(einf->info,f);
	edit_fnr = i;
	if (!state_open) clear_file_ref();
	break;
    case NEWDOC:
	i = edit_fnr;
	edit_fnr = 0;
	read_file(f, DOCUMENTFILE);
	unset_file();
	include_editwindow(einf->info);
	edit_fnr = i;
	break;
    default: break;
    }
    fclose(f);
    cleanup_filestack();
    cleanup_stencilstack();
    cleanup_nodestack();
    einf->saved = MP_False;
    einf->auto_saved = MP_False;
    remove_wait_cursor();
    free(name);
}

static void edit_handle_fileselc_save(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (name) {
	FILE *f;
	int check_found = check_name(einf, name);

	f = fopen((char*)UstrtoFilename(name),"wb");
	if (f) {
	    set_wait_cursor(einf->win_id);
	    set_file(f);
	    put_filecode(DOCUMENTFILE);
	    save_editwindow(einf->info);
	    unset_file();
	    cleanup_stencilstack();
	    fclose(f);
	    remove_auto_save(einf);
	    einf->auto_saved = MP_True;
	    einf->saved = MP_True;
	    if (check_found) {
		einf->view_mode = MP_True;
		message(MP_CLICKREMARK, translate("You have saved the document under a name\n"
			"which is already used by a different window.\n"
			"To make sure that your backups are\n"
			"correct, this copy will be in view mode."));
	    }
	    set_name(einf,name);
	    message(MP_MESSAGE,translate("File saved."));
	    remove_wait_cursor();
	} else {
	    message(MP_ERROR, translate("Can't save file! "));
	    free(newname);
	    failure=MP_True;
	}
    } else
	free(newname);
    kind_of_remark = NO_REMARK;
}

static void edit_handle_fileselc_rename(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;
    int check_found = check_name(einf, name);
    if (check_found) {
	einf->view_mode=MP_True;
	message(MP_CLICKREMARK, translate("There is already a document loaded with\n"
		"the same name in a different window.\n"
		"To make sure that your backups are\n"
		"correct, this copy will be in view mode."));
    }
    set_name(einf, name);
}

static int texmode=MPTEX;
static char envbuf[3000];

static void edit_handle_fileselc_output(void *data, Char *name)
{
    EDITINFO *einf = (EDITINFO *) data;
    FILE *f;

    if (aig(f=fopen((char*)UstrtoFilename(name), "w"))) {
	tex_set_file(f);
	tex_mode(texmode);
	tex_placeholders(ON);
	latex_editwindow(einf->info);
	tex_unset();
	fclose(f);
	free(einf->outputname);
	einf->outputname = name;
	sprintf(envbuf,"OUTPUTFILE=%s", UstrtoFilename(einf->outputname));
	if (putenv(envbuf)!=0) printf("No outputfile set!!!\n");
	message(MP_MESSAGE, translate("Document converted."));
    } else {
	message2(MP_CLICKREMARK, translate("No output made. Unable to open file "),
		 name);
	failure=MP_True;
    }
}

static void edit_handle_remark_done(void *data, int bnr)
{
    EDITINFO *einf = (EDITINFO *) data;
    FILE *f;
    int close_next = quit_sequence;

    if (!bnr) {
	f = fopen((char*)UstrtoFilename(newname),"wb");
	if (f) {
	    set_file(f);
	    put_filecode(DOCUMENTFILE);
	    save_editwindow(einf->info);
	    unset_file();
	    cleanup_stencilstack();
	    fclose(f);
	    remove_auto_save(einf);
	    einf->saved = MP_True;
	    edit_close(data);
	} else {
	    message(MP_ERROR, translate("Can't save file."));
	    failure=MP_True;
	}
    } else if (bnr==1) {
	edit_auto_save(data, 0);
	einf->saved = MP_True;
	edit_close(data);
    } else if (bnr==2) {
	close_next = MP_False;
	quit_sequence = MP_False;
    }
    free(newname);
    kind_of_remark = NO_REMARK;
    newname = NULL;
    if (close_next) menu_close();
}

static void (*handle_fileselc_func[NR_BUTTON+5])(void*,Char*) = {
    handle_filename,     edit_handle_fileselc_save,
    edit_handle_fileselc_rename,   edit_handle_fileselc_output,
    handle_include_filename, NULL, edit_handle_fileselc_output,
    edit_handle_fileselc_output, edit_handle_fileselc_output,
    edit_handle_fileselc_output, edit_handle_fileselc_output
};


static void set_fileselc(void *data, int nr)
{
    EDITINFO *einf = (EDITINFO *) data;
    Char *c=NULL, *h, *s;

    nr = perm[nr];
    if ((nr==OUTPUTBUTTON || nr>=NR_BUTTON) && einf->outputname) {
	h = einf->outputname;
	s = strip_name(einf->outputname);
	if (s!=h) {
	    c=s-1;
	    *c='\0';
	} else h=NULL;
    } else {
	h = einf->pathname;
	s = strip_name(einf->pathname);
	if (s!=h) {
	    c=s-1;
	    *c='\0';
	} else h=NULL;
    }
    fileselc_open(handle_fileselc_func[nr], data, translate(textremark[nr].lines),
		  h, translate(textremark[nr].extension), s, einf->win_id);
    if (c) *c='/';
}

static void edit_draw(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    redraw_window(einf->info);
}

static void edit_layout_change(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (!data) return;
    if (!einf->iconized)
	XClearArea(display, einf->drawwin_id, 0, 0, 0, 0, MP_True);
    scrollbar_linesize(einf->scrollver, line_height());
}

/*
static void handle_output_popup(void *data, int nr)
{
    EDITINFO *einf = (EDITINFO *) data;
    if (!einf->outputname) set_output_name(einf);
    texmode = nr;
    set_fileselc(data, NR_BUTTON+nr);
}
*/

static void edit_handle_button(void *data, int b_num)
{
    EDITINFO *einf = (EDITINFO *) data;

    switch (b_num) {
    case OUTPUTBUTTON:
	if (mouse_button==3) {
	    outputmenu.parentwin=einf->win_id;
	    outputmenu.x=-1;
	    outputmenu.y=-1;
	    outputmenu.menu = popup_define(translate("OutputFormat"));
	    popup_make(&outputmenu);
	} else {
	  outputmenu.menu = popup_define(translate("OutputFormat"));
	  popup_call_default(&outputmenu);
	  set_fileselc(data,b_num);
	}
	break;
    case LOADBUTTON:
    case INCLUDEBUTTON:
    case SAVEBUTTON:
    case RENAMEBUTTON:
	set_fileselc( data, b_num);
	break;
    case DONEBUTTON:
	if (can_close_edit) edit_close(data);
	break;
    }
    newname = NULL;
}

static int edit_margin(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    einf->empty = window_empty(einf->info);
    if (!einf->view_mode && !einf->shell) {
	if (!einf->auto_saved)
	    einf->auto_saved = einf->empty;
	else if (change_check)
	    einf->auto_saved = !window_changed(einf->info) || einf->empty;
	if ((!einf->auto_saved && einf->saved) || 
	    (!einf->saved && einf->empty)) {
	    einf->saved = !einf->saved;
	    set_name(einf, einf->pathname);
	}
    }
    return -scrollbar_line(einf->scrollhor, 0) * font_width() + 3;
}

static void edit_press(void *data, XButtonEvent *event)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (event->window == einf->drawwin_id) {
	change_check = MP_False;
	mouse_down(einf->info, event->x-edit_margin(data),
		   event->y, mouse_button);
	get_motion_hints(einf->drawwin_id, -1);
    }
}

static void double_click_func(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    change_check = MP_False;
    dbl_click();
    get_motion_hints(einf->drawwin_id, 0);
}

static void edit_release(void *data, XButtonEvent *event)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (event->window == einf->drawwin_id) {
	stop_motion_hints();
	mouse_up(event->x -edit_margin(data), event->y);
	change_check = MP_True;
    }
}

static void edit_motion(void *data, int x, int y)
{
    mouse_move(x-edit_margin(data),y);
}

static void edit_resize(void *data, XConfigureEvent *event)
{
    EDITINFO *einf = (EDITINFO *) data;
    unsigned int new_width, new_height;

    einf->width = last_width = event->width;
    einf->height = last_height = event->height;
    einf->xpos = last_xpos = event->x;
    einf->ypos = last_ypos = event->y;
    new_width  = sub_width( event->width );
    new_height = sub_height(event->height );
    XResizeWindow(display, einf->drawwin_id, new_width-2, new_height-2);
    resize_window(einf->info, new_width-2, new_height-2);
    scrollbar_resize(einf->scrollver, new_height);
    scrollbar_resize(einf->scrollhor, new_width);
}

static void edit_scrollto(void *data, int kind)
{
    EDITINFO *einf = (EDITINFO*) data;
    /*
    ** handle scrollbar up/down/left/right
    ** redraw drawwin
    */
    if (!kind) {
	redraw_window(einf->info);
    } else {
	/* vertical scroll */
	int line_nr = scrollbar_line(einf->scrollver,0);
	editwindow_line(einf->info, line_nr);
    }
}

static void edit_iconize(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (!einf->iconized) {
	einf->iconized = MP_True;
	number_icon++;
	edit_iconized =  (number_open==number_icon);
    }
    /*
    **  sluit invoer op einf->info af
    */
}

static void edit_deiconize(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (einf->iconized) {
	einf->iconized = MP_False;
	number_icon--;
	edit_iconized = MP_False;
    }
    /*
    **  maak invoer op einf->info mogelijk
    */
}

static void edit_state(void *data, int *x, int*y, int *w, int *h,
		       int *i, int *s, Char **str)
{
    EDITINFO *einf = (EDITINFO *) data;
    int xm,ym;
    window_manager_added(einf->win_id, &xm, &ym);
    *x = einf->xpos-xm;
    *y = einf->ypos-ym;
    *w = einf->width;
    *h = einf->height;
    *i = (einf->iconized ? 0x1 : 0x0);
    *i += (einf->view_mode ? 0x2 : 0x0);
    *s = scrollbar_line(einf->scrollver, 0);
    *str = einf->pathname;
}

static void edit_use_state(int x, int y, int w, int h,
			   int i, int s, Char *str)
{
    as_icon = i&0x1;
    state_open = MP_True;
    state_window=NULL;
    if (w>0 && h>0) {
	last_xpos = x;
	last_ypos = y;
	last_width = w;
	last_height = h;
    }
    if (i&0x4) {
	open_helpfile(str, 0);
    } else {
	edit_open();
	if (state_window) {
	    if (i&0x2)
		handle_view_filename( (void *) state_window, str);
	    else
		handle_filename( (void *) state_window, str);
	}
    }
    if (state_window)
	editwindow_line(state_window->info, s);
    state_open = MP_False;
    as_icon = MP_False;
}

static int edit_last_pos(int *x, int *y, int *w, int *h)
{
    *x = last_xpos;
    *y = last_ypos;
    *w = last_width;
    *h = last_height;
    return is_opened;
}

static void edit_set_last_pos(int x, int y, int w, int h)
{
    last_xpos = x;
    last_ypos = y;
    last_width = w;
    last_height = h;
}

void edit_set_number_of_lines(void *window, int numlin)
{
    EDITINFO *einf;
    void *pdata;
    Window pwin;

    (void) get_window_type(*(Window*) window, &pwin, &pdata);
    (void) get_window_type(pwin, &pwin, &pdata);
    einf = (EDITINFO*) pdata;
    scrollbar_set(einf->scrollver, line_number(einf->info), numlin);
}

Bool edit_saved(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    return (einf->saved);
}

void edit_bad_end(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;
    /*
    **  save belangrijke informatie in backup-file  #?#  ?~ ?.BAK
    */
    edit_auto_save(data, 0);
    einf->saved = MP_True;
    close_editwindow(einf->info);
    free(einf->headername);
    free(einf->pathname);
    free(einf->filename);
    free(einf->outputname);
    free(einf->prcsbuf);
    if (einf->iconized) number_icon--;
    edit_is_open = (--number_open >0);
    edit_iconized = (number_icon==number_open);
    popup_remove(einf->win_id);
    destroy_window(einf->win_id);
}

#define PROPBUFSIZE 8192

static void edit_property_handle(void *data, XPropertyEvent *event)
{
    EDITINFO *einfo = (EDITINFO*) data;
    if (event->window != einfo->win_id) return;
    if (event->atom==inputat && event->state==PropertyNewValue) {
	long n=0;
	long len=PROPBUFSIZE;
	Atom actt;
	int actf;
	unsigned long nit,baf=1;
	unsigned int redu;
	unsigned char *prp;
	unsigned char *totalbuf=malloc(1);
	int totsize=0;

	while (baf) {
	    unsigned char *h;
	    XGetWindowProperty(display, einfo->win_id, inputat, n/4, len, MP_True,
			       textat, &actt, &actf, &nit, &baf, &prp);
	    redu=nit;
	    h=realloc(totalbuf, totsize+nit+1);
	    if (h) {
	      memcpy(h+totsize, prp, nit);
	      totalbuf=h;
	      totsize=totsize+nit;
	    } else {
	      fprintf(stderr, "Out of memory on processing input.\n");
	    }
	    XFree(prp);
	    n=n+nit;
	}
	totalbuf[totsize]=0;
	if (!einfo->callback || !(*einfo->callback)(totalbuf,&totsize)) {
	  Char *conv;
	  conv = LocaletoUstr(totalbuf);
	  append_editwindow(einfo->info, conv, Ustrlen(conv));
	}
	free(totalbuf);
	if (!n) {
	    einfo->fini=MP_True;
	    einfo->pid=0;
	    einfo->strt=MP_False;
	    set_name(einfo, einfo->pathname);
	}
    } else if (event->atom==outtestat && event->state==PropertyNewValue) {
	einfo->strt=MP_True;
	if (einfo->buflen) {
	    XChangeProperty(display, einfo->win_id, outputat, textat, 8,
			    PropModeAppend, einfo->prcsbuf, (int)einfo->buflen);
	    XFlush(display);
	    free(einfo->prcsbuf);
	    einfo->prcsbuf=NULL;
	    einfo->buflen=0;
	}
    } else if (event->atom==outtestat && event->state==PropertyDelete) {
	einfo->strt=MP_False;
	einfo->fini=MP_True;
	einfo->pid=0;
	set_name(einfo, einfo->pathname);
    } else if (event->atom==procesidat && event->state==PropertyNewValue) {
	long n=0;
	long len=4096;
	Atom actt;
	int actf;
	unsigned long nit,baf=1;
	unsigned char *prp;
	while (baf) {
	    XGetWindowProperty(display, einfo->win_id, procesidat, n/4, len, MP_True,
			       textat, &actt, &actf, &nit, &baf, &prp);
	    if (prp) {
		einfo->pid=atoi((char*)prp);
		XFree(prp);
	    }
	    n=n+nit;
	}
    }
}

static void edit_send_to_proces(void *data, unsigned char *txt, int len)
{
    EDITINFO *einf= (EDITINFO*) data;
    if (!einf->shell || einf->fini) return;
    if (!einf->strt) {
	unsigned char *h;
	h=(unsigned char*) malloc(sizeof(unsigned char)*(einf->buflen+len));
	memcpy(h, einf->prcsbuf, (size_t)einf->buflen);
	memcpy(h+einf->buflen, txt, (size_t)len);
	if (einf->prcsbuf) free(einf->prcsbuf);
	einf->prcsbuf=h;
	einf->buflen=einf->buflen+len;
    } else {
	XChangeProperty(display, einf->win_id, outputat, textat, 8,
			PropModeAppend, txt, len);
	XFlush(display);
    }
}

FUNCTIONS maineditfuncs = {
    edit_bad_end, NULL, edit_resize, NULL, NULL, NULL, edit_iconize,
    edit_deiconize, NULL, NULL, edit_layout_change, edit_auto_save,
    edit_use_state, edit_state, NULL, NULL, edit_last_pos,
    edit_set_last_pos, NULL, edit_property_handle };

FUNCTIONS editfuncs = {
    NULL, edit_draw, NULL, edit_press, edit_release, edit_motion,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, edit_margin,
    edit_set_number_of_lines, NULL, NULL, double_click_func };

void edit_signal_to_proces(int signl, Char *shname)
{
    EDITINFO *edata;
    int i=0;
    while (aig(edata = (EDITINFO*) next_data_with_type(MAINEDITWINDOW,&i)) &&
	    Ustrcmp(edata->pathname,shname)) i++;
    if (!edata) {
	message2(MP_ERROR, shname, translate(" is not running."));
	return;
    }
    if (!edata->pid) {
	message2(MP_ERROR, translate("Unable to send signals to "),shname);
	return;
    }
    kill(edata->pid, signl);
}

void edit_string_to_proces(Char *txt, Char *shname)
{
    EDITINFO *edata;
    char *c,*h;
    Char *d;
    char *arg[4];
    Bool only_text=MP_False;
    int len=0,i=0;
    char lpref='\0';
    /*
    ** txt uses the following format sequences:
    **  %t  %1   target selection
    **  %s  %2   source selection
    **  %a  %3   argument selection
    **  %P?      ? is added at the beginning of each line (in a selection)
    **  %T       only text place holders are passed through
    **  %E       all place holders are passed through
    **  %?       ? character
    ** To add: some way to select the output mode.
    */
    while (aig(edata = (EDITINFO*) next_data_with_type(MAINEDITWINDOW, &i)) &&
	   Ustrcmp(edata->pathname,shname)) i++;
    if (!edata) {
	message2(MP_ERROR, shname,translate(" is not running."));
	return;
    }
    if (!edata->shell) {
	message2(MP_ERROR, shname,translate(" can not receive input."));
	return;
    }
    if (edata->fini) {
	message2(MP_ERROR, shname,translate(" is finished."));
	return;
    }
    if (!edata->strt) message2(MP_MESSAGE, shname, translate(" is still busy."));
    /* scan txt for arguments, to determine len */
    d=txt;
    for (i=0; i<4; arg[i++]=NULL);
    while (*d) {
	switch (*d) {
	case '%':
	    d++;
	    i=0;
	    switch (*d) {
	    case 't': case '1': i=1; break;
	    case 's': case '2': i=2; break;
	    case 'a': case '3': i=3; break;
	    case 'P': d++; lpref=*d; if (!*d) d--; break;
	    case 'T': only_text=MP_True; break;
	    case 'E': only_text=MP_False; break;
	    case '\0': c--; break;
	    default: len++; break;
	    }
	    if (i) {
		if (!arg[i]) {
		    tex_set_string(&arg[i]);
		    tex_placeholders(ON);
		    tex_mode(ASCII);
		    latex_text_only(only_text);
		    latex_selection(i);
		    latex_text_only(MP_False);
		    tex_unset();
		}
		if (arg[i]) {
		    len+=strlen(arg[i]);
		    if (lpref>32) {
			h=arg[i];
			while (*h) {
			    if (*h=='\n') len++;
			    h++;
			}
		    }
		} else {
		  arg[0]=malloc(sizeof(char));
		  arg[0][0]=0;
		}
	    }
	    break;
	default:
	    len++;
	    break;
	}
	d++;
    }
    if (arg[0]) {
	message(MP_ERROR, translate("Selections not set properly."));
	for (i=0;i<4;i++) if (arg[i]) free(arg[i]);
	return;
    }
    /* make string */
    c=h=(char*)malloc(sizeof(char)*(len+2));
    while (*txt) {
	switch (*txt) {
	case '%':
	    txt++;
	    i=0;
	    switch (*txt) {
	    case 't': case '1': i=1; break;
	    case 's': case '2': i=2; break;
	    case 'a': case '3': i=3; break;
	    case 'P': txt++; lpref=*txt; if (!*txt) txt--; break;
	    case 'T': break;
	    case '\0': txt--; break;
	    default: *h++=*txt; break;
	    }
	    if (i && arg[i]) {
		if (lpref>32) {
		    char *t=arg[i];
		    while (aig(*h=*t)) {
			if (*h=='\n') {
			    h++;
			    *h=lpref;
			}
			h++;
			t++;
		    }
		} else {
		    strcpy(h, arg[i]);
		    while (*h) h++;
		}
	    }
	    break;
	default:
	    *h++=*txt;
	    break;
	}
	txt++;
    }
    *h='\0';
    /* send string c */
    edit_send_to_proces((void*)edata, (unsigned char*) c, len);
    free(c);
    for (i=0; i<4; i++) if (arg[i]) free(arg[i]);
}

void edit_init(void)
{
    if (output_mode==ASCII)
	perm[OUTPUTBUTTON] = NR_BUTTON;
    edit_mask =
        (CWBackPixel | CWBorderPixel | CWBitGravity |
	 CWEventMask | CWColormap);

    edit_attr.background_pixel = white_pixel;
    edit_attr.border_pixel = black_pixel;
    edit_attr.colormap = colormap;
    edit_attr.bit_gravity = NorthWestGravity;
    edit_attr.event_mask = (ExposureMask | ButtonPressMask | ButtonReleaseMask
			    | ButtonMotionMask | PointerMotionHintMask
			    | KeyPressMask | StructureNotifyMask
			    | FocusChangeMask
			    | VisibilityChangeMask | PropertyChangeMask);
    if (!last_width) {
	last_width = display_width / 2;
	last_height = display_height / 3;
	last_xpos = (display_width-last_width)/2;
	last_ypos = (display_width-last_width)/2;
    }
    inputat  =XInternAtom(display, "MPINPUT", MP_False);
    outputat =XInternAtom(display, "MPOUTPUT", MP_False);
    intestat =XInternAtom(display, "MPINTEST", MP_False);
    outtestat=XInternAtom(display, "MPOUTTEST", MP_False);
    textat   =XInternAtom(display, "TEXT", MP_False);
    procesidat=XInternAtom(display,"PROCESSID", MP_False);
}

void edit_open(void)
{
    int x = INTERSPACE;
    int y = INTERSPACE;
    unsigned int w=4,h=4;
    int i;
    XSizeHints size_hints;
    EDITINFO *einf;

    state_window = NULL;
    if ( (einf = (EDITINFO *) malloc( sizeof(EDITINFO) )) == NULL)
	message(MP_ERROR, translate("Out of memory in edit."));
    else {
	if (!state_open)
	    if (!last_xpos && !last_ypos) {
		last_xpos = (display_width - last_width)/2;
		last_ypos = (display_height - last_height)/2;
	    }
	einf->xpos = last_xpos;
	einf->ypos = last_ypos;
	einf->width = last_width;
	einf->height = last_height;
	einf->saved = MP_True;
	einf->auto_saved = MP_True;
	einf->view_mode = MP_False;
	einf->empty = MP_True;
	einf->iconized  = MP_True;
	einf->shell = MP_False;
	einf->fini = MP_False;
	einf->strt = MP_False;
	einf->buflen=0;
	einf->pid=0;
	einf->prcsbuf=NULL;
	einf->win_id = XCreateWindow(display, root_window, einf->xpos,
				     einf->ypos, einf->width, einf->height,
				     BORDERWIDTH, CopyFromParent, InputOutput,
				     visual,
				     edit_mask, &edit_attr);
	if (state_open)
	    size_hints.flags = USPosition | USSize | PMinSize;
	else
	    size_hints.flags = PPosition | PSize | PMinSize;
	wm_hints.initial_state =
	    ((iconic || as_icon) ? IconicState : NormalState);
	size_hints.min_width =
	    size_hints.min_height = pos_y_with + SCROLLBARSIZE*3;
	XSetWMProperties(display, einf->win_id, NULL, NULL,
			 NULL, 0, &size_hints, &wm_hints, &class_hints);
	wm_hints.initial_state = NormalState;
	set_protocols(einf->win_id);

	i=0;
	einf->headername = NULL;
	einf->filename = NULL;
	einf->pathname = NULL;
	einf->outputname = NULL;
	if (set_name(einf, NULL) &&
	    add_window(einf->win_id, MAINEDITWINDOW,
		       root_window, (void *) einf, translate(helpname[EDITHELP]))) {
	    while (i<NR_BUTTON &&
		   button_make(i, einf->win_id, translate(editbutton[perm[i]]), &x, y, 1,
			       (void*) einf, helpname[edithelp[i]],
			       NULL, NULL, edit_handle_button,
			       edit_handle_button, edit_handle_button, NULL))
		i++,x+=BINTERSPACE;
	    w = sub_width(last_width);
	    h = sub_height(last_height);
	    if (i==NR_BUTTON) {
		einf->drawwin_id =
		    XCreateWindow(display, einf->win_id,
				  pos_x_with, pos_y_with, w-2, h-2, 1,
				  CopyFromParent, InputOutput,
				  visual,
				  edit_mask, &edit_attr);
		if (add_window(einf->drawwin_id, EDITWINDOW,
			       einf->win_id, NULL, translate(helpname[EDITSUBHELP])))
		    i++;
	    }
	    if (i==NR_BUTTON +1 &&
		(einf->scrollhor =
		 scrollbar_make(HORIZONTAL, einf->win_id, pos_x_with,
				pos_y_without, w, font_width(),
				edit_scrollto, (void*) einf)))
		i++;
	    if (i==NR_BUTTON+2 &&
		(einf->scrollver =
		 scrollbar_make(VERTICAL, einf->win_id, pos_x_without,
				pos_y_with, h, line_height(),
				edit_scrollto, (void*) einf)))
		i++;
	    
	}
	if (i<NR_BUTTON+3) {
	    free(einf->headername);
	    free(einf->pathname);
	    free(einf->filename);
	    XDestroyWindow(display, einf->win_id);
	    destroy_window(einf->win_id);
	} else {
	    is_opened = MP_True;
	    scrollbar_set(einf->scrollver, 0, 1);
	    scrollbar_set(einf->scrollhor, 0, 80);
	    move_selection = !as_icon;
	    einf->info = open_editwindow(&einf->drawwin_id, w-2, h-2);
	    move_selection = MP_False;
	    (void) window_changed(einf->info);
	    number_icon++;
	    number_open++;
	    edit_is_open = MP_True;
	    edit_iconized = (number_icon==number_open);
	    state_window = einf;
	    XMapSubwindows(display, einf->win_id);
	    XMapWindow(display, einf->win_id);
	}
    }
}

void edit_close(void *data)
{
    EDITINFO *einf = (EDITINFO *) data;

    if (einf->saved) {
	close_editwindow(einf->info);
	free(einf->headername);
	free(einf->pathname);
	free(einf->filename);
	free(einf->outputname);
	if (einf->iconized) number_icon--;
	edit_is_open = (--number_open >0);
	edit_iconized = (number_icon==number_open);
	XDestroyWindow(display, einf->win_id);
	popup_remove(einf->win_id);
	destroy_window(einf->win_id);
    } else {
	if (einf->iconized) {
	    XMapWindow(display, einf->win_id);
	    einf->iconized = MP_False;
	    XRaiseWindow(display, einf->win_id);
	}
	XFlush(display);
	newname = einf->pathname;
	{
	  int bnr;
	  for (bnr=0; donesave8[bnr]; bnr++) {
	    donesave[bnr] = translate(donesave8[bnr]);
	  }
	}	  
	remark_make(einf->win_id, data, edit_handle_remark_done,
		    REMARK_BUTTON, translate(textremark[DONEBUTTON].lines),
		    donesave, &newname, 300, NULL);
	remark_raise();
	newname = NULL;
    }
}

void open_program(Char *c, Char *title, int (*func)(unsigned char*, unsigned int*))
{
    /* c contains the commandline that should be executed.
     * It should contain a %i (or %x) at the location where the window-id
     * should be inserted
     * It has to be a shell script of the form:
     *      xpipeout -window wid | command | xpipein -window wid
     */
    EDITINFO *edata;
    char callbuf[1024];
    as_icon=1;
    edit_open();
    as_icon=0;
    if (state_window) {
	edata=state_window;
	edata->shell=1;
	edata->callback=func;
	sprintf(callbuf, (char*)UstrtoLocale(c), edata->win_id, edata->win_id);
	XChangeProperty(display, edata->win_id, intestat, textat, 8,
			PropModeReplace, (unsigned char*)"Yes", 3);
	XDeleteProperty(display, edata->win_id, outputat);
	XFlush(display);
	system(callbuf);
	/* wait until program is ready to read. */
	edata->view_mode=MP_True;
	set_name(edata, concat(title,NULL));
    }
}

static EDITINFO *message_window(Char *messagetitle)
{
    int i=0;
    EDITINFO *edata;
    if (!messagetitle) return NULL;
    while (aig(edata = (EDITINFO*) next_data_with_type(MAINEDITWINDOW, &i)) &&
	   Ustrcmp(edata->pathname,messagetitle)) {
	i++;
    }
    if (!edata) {
        int old_icon=as_icon;
	as_icon=MP_True;
	edit_open();
	as_icon=old_icon;
	if (state_window) {
	    edata=state_window;
	    edata->view_mode=MP_True;
	    set_name(edata, concat(messagetitle,NULL));
	}
    }
    if (edata && edata->iconized && 0) {
	XMapWindow(display, edata->win_id);
	XFlush(display);
    }
    return edata;
}

void open_message_window(Char *messagetitle)
{
    message_window(messagetitle);
}

void string_to_window(Char *messagegroup, Char *mess)
{
    EDITINFO *edata;
    edata=message_window(messagegroup);
    if (!edata) {
	fprintf(stderr, (char*)UstrtoLocale(translate("No window named '%s'.\n%s\n")),
		UstrtoLocale(messagegroup), UstrtoLocale(mess));
	return;
    }
    append_editwindow(edata->info, mess, Ustrlen(mess));
}

void structure_to_window(Char *messagegroup)
{
    EDITINFO *edata;
    edata=message_window(messagegroup);
    if (!edata) {
	fprintf(stderr, "No window named '%s'.\n", UstrtoLocale(messagegroup));
	include_selection();
	return;
    }
    append_structure(edata->info);
}

void open_temporary_file(char *editname, char *filename, int disp, int linenum)
{
  EDITINFO *edata;
  int i;
  Char *newname;

  newname=concat(LocaletoUstr(editname),0);
  i=0;
  while (aig(edata = (EDITINFO*) next_data_with_type(MAINEDITWINDOW, &i)) &&
	 Ustrcmp(edata->pathname,newname))
    i++;
  if (!edata) {
    as_icon=1;
    edit_open();
    as_icon=0;
    if (state_window) edata = state_window;
  }
  handle_view_filename((void *) edata, concat(LocaletoUstr(filename),NULL));
  editwindow_line(edata->info, linenum-1);
  set_name(edata,newname);
  if (edata->iconized) {
    XMapWindow(display, edata->win_id);
  }
  if (disp) XRaiseWindow(display, edata->win_id);
}  

/* header to be able to add this function to a menu */
void open_helpfile(void *data, int nr __attribute__((unused)))
{
    Char *c = (Char*) data;
    int i=0,hpos=0;
    Char *f;
    Char *name;
    Char *fullname;
    EDITINFO *edata;

    if (!c || !(f = (Char*) malloc(sizeof(Char)*(Ustrlen(c)+1)))) return;
    for (i=0; (f[i]=c[i]) ; i++)
	if (c[i]=='#') if (!i) hpos=-1; else if (hpos) hpos=0; else hpos=i;
    if (hpos>0) {
	name = c+hpos+1;
	f[hpos]='\0';
    } else name = c+i;
    f = standard_dir(f);
    fullname = search_through_dirs(help_dirs, nr_help_dirs, f);
    if (!fullname && f[0]=='/')
	fullname=f;
    else
	free(f);
    if (!fullname) {
	message(MP_ERROR, translate("Help document not found."));
	return;
    }
    i=0;
    while (aig(edata = (EDITINFO*) next_data_with_type(MAINEDITWINDOW, &i)) &&
	   Ustrcmp(edata->pathname,fullname))
	i++;
    if (!edata) {
	as_icon=1;
	edit_open();
	as_icon=0;
	if (state_window) {
	    edata = state_window;
	    handle_view_filename((void *) state_window, concat(fullname,NULL));
	    word_wrap_window(edata->info);
	}
    }
    if (!edata) {
	message2(MP_CLICKREMARK, translate("Unable to open an edit window for document "),
		 name);
	return;
    }
    if (name[0]) {
	int j=Ustrlen(name)+1;
	Char *cname = (Char*) malloc(j*sizeof(Char));
	if (cname) {
	    for (i=j-1;i>=0; i--) cname[i]=name[i];
	    editwindow_topto(edata->info, cname);
	    free(cname);
	}
    }
    if (edata->iconized) {
	XMapWindow(display, edata->win_id);
    }
    XRaiseWindow(display, edata->win_id);
}

