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
**  File  : menu.c
**  Datum : 9-4-92
**  Doel  : plaatsen van group-window en buttons maken voor de verschillende
**          functie en windows
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
/* #include "keyboard.h" */
#include "message.h"
#include "button.h"
#include "symbol.h"
#include "edit.h"
#include "buffer.h"
#include "default.h"
#include "notatype.h"
#include "notation.h"
#include "output.h"
#include "latexout.h"
#include "find.h"
#include "editor.h"
#include "menu.h"
#include "popup.h"
#include "fileselc.h"
#include "fileread.h"
#include "helpfile.h"
#include <ctype.h>

#include "unitype.h"
#include "parse.h"

#define MENUNAME  "MathSpad 0.80"
#define ICONNAME  "MathSpad"
#define POPUPSIZE 1000
#define MAXLINES 50

enum button { WINDOWBUTTON, EDITOPBUTTON, STRUCTUREBUTTON, SELECTIONBUTTON,
              MISCBUTTON, VERSIONBUTTON, QUITBUTTON, NR_BUTTON };

#define NR_ROWS    1

static
char *menubutton[NR_BUTTON] =
       { "Window", "EditOp", "Structure", "Selection",
         "Misc.",  "Version","Quit"
       };
static
int menuhelp[NR_BUTTON] =
     { CONSOLEWINDOWHELP, CONSOLEEDITOPHELP, CONSOLESTRUCTUREHELP,
       CONSOLESELECTIONHELP, CONSOLEMISCHELP, CONSOLEVERSIONHELP,
       CONSOLEQUITHELP };


static int rows[NR_ROWS] = { NR_BUTTON };
static MENU mainmenu[NR_BUTTON+1];
static char *mainmenuname[NR_BUTTON+1] = {
  "WindowMenu", "EditOpMenu", "StructureMenu", "SelectionMenu", "MiscMenu",
  "VersionMenu", "ExitMenu" };

static void popup_quit(void *data, int n);
static void create_popup(void *data, int n);

typedef struct ITEMDESC { 
    char *descr;
    int nr;
    int submenu;
    void *data;
    BTFUNC func;
} ITEMDESC;

static Window menuwin, miniwin;
static void *mini_info;
static int win_xpos=0, win_ypos=0;
static unsigned int win_width=0, win_height=0, window_width;
static unsigned int miniwin_width, miniwin_height;
static int line_y;
static Bool searching = MP_False;
static Char popuptext[POPUPSIZE];

#define sub_width(A)   (A) - INTERSPACE*2
#define sub_height(A)  line_height()
#define window_height  NR_ROWS*(button_height+BINTERSPACE) + line_height() \
                       + INTERSPACE*2

int quit_sequence = MP_False;

static void set_name(Char *projectname)
{
    XTextProperty prop_name;
    char name[1500];
    char *tname;
    Char *filename;
    int i,l;

    if (projectname) {
        filename = Ustrrchr(projectname,DIRSEPCHAR);
        if (!filename) filename=projectname; else filename++;
        sprintf(name, "%s : %s",
		UstrtoLocale(translate(MENUNAME)),
		UstrtoLocale(filename));
        i=strlen(name);
	l=strlen((char*)UstrtoLocale(translate(".mpj")));
	if (i>l && !strcmp(name+i-l,(char*)UstrtoLocale(translate(".mpj"))))
	  name[i-l]='\0';
        if (i>4 && !strcmp(name+i-4,".mpj")) name[i-4]='\0';
    } else
        strcpy(name, (char*)UstrtoLocale(translate(MENUNAME)));
    tname=name;
    if (!XStringListToTextProperty(&tname, 1, &prop_name)) return;
    XSetWMName(display, menuwin, &prop_name);
    filename=translate(ICONNAME);
    {
      char *fn = (char*)UstrtoLocale(filename);
      if (!XStringListToTextProperty(&fn, 1, &prop_name)) return;
      XSetWMIconName(display, menuwin, &prop_name);
    }
}

static void notation_make_backups(void *data __attribute__((unused)), int dumps)
{
    int i=0;
    Char *c;
    Bool opened;

    while ((i = get_next_filename(i, &c, &opened))>=0)
        auto_save_window(i, dumps);
}

static void draw_search(void);

static void menu_draw(void *data)
{
    Window win = *((Window*)data);
    if (win == menuwin)
        XDrawLine(display, menuwin, get_GC(Normal,0,0),
                  0, line_y, (int)win_width, line_y);
    else
        if (searching)
            draw_search();
        else
            draw_message();
}

static void menu_layout_change(void *data __attribute__((unused)))
{
    XSizeHints hints;
    clear_tab_positions();
    push_fontgroup(POPUPFONT);
    win_height = window_height;
    miniwin_height = sub_height(window_height);
    XResizeWindow(display, menuwin, win_width, win_height);
    hints.flags = (PMinSize | PMaxSize);
    hints.min_height = hints.max_height = win_height;
    hints.min_width = window_width;
    hints.max_width = display_width;
    XSetWMNormalHints(display, menuwin, &hints);
    XResizeWindow(display, miniwin, miniwin_width, miniwin_height);
    XClearArea(display, miniwin, 0, 0, 0, 0, MP_True);
    resize_window(mini_info, miniwin_width, miniwin_height);
    if (searching) draw_search();
    pop_fontgroup();
}

static void menu_state(void *data __attribute__((unused)), int *posx, int *posy, int *width,
                       int *height, int *as_icon, int *sbpos, Char **string)
{
    *posx = win_xpos;
    *posy = win_ypos;
    *height = win_height;
    *width = win_width;
    *as_icon = menu_iconized;
    *sbpos = 0;
    *string = NULL;
}

static void menu_resize(void *data, XConfigureEvent *event)
{
    if (event->window == menuwin) {
        int x,y;
        window_manager_added(menuwin, &x,&y);
        win_xpos = event->x-x;
        win_ypos = event->y-y;
        win_width = event->width;
        push_fontgroup(POPUPFONT);
        miniwin_width = sub_width(win_width);
        win_height = event->height;
        XResizeWindow(display, miniwin, miniwin_width, miniwin_height);
        resize_window(mini_info, miniwin_width, miniwin_height);
        menu_draw(data);
        pop_fontgroup();
    }
}

static void handle_new_version(void *data __attribute__((unused)),  int ivnr)
{
    if (ivnr>=0) new_version(ivnr);
}


static void handle_new_id_font(int nfnr)
{
    new_id_font(nfnr);
}

static MENU idfont;
static Bool make_id_popup(void)
{
    int nr;

    nr = ps_id_font();
    if (nr<0) return MP_False;
    idfont.selline=idfont.x=idfont.y=-1;
    idfont.sticky=MP_False;
    idfont.freesub=MP_False;
    idfont.parentwin=menuwin;
    idfont.menu = popup_define(translate("IdentifierSelect"));
    return (popup_make(&idfont)!=NULL);
}

static void next_template_version(void)
{
  new_version(-1);
}

static Bool make_version_popup(void)
{
  int nr;
  unsigned int vnr=0;
  nr = ps_notation(&vnr);
  if (nr>=0) {
    if (!make_notation_popup(nr, vnr, handle_new_version,
			     translate("Version"), True))
      new_version(-1);
  } else {
    if (!make_id_popup())
      message(MP_MESSAGE, translate("No template or identifier selected."));
  }
}

static void latex_select(Index arg);
static void ask_selection(void);
static void ask_latex_line(void);
static void switch_textdots(void);

static void save_project_wrap(Char *name)
{
  Char *h = translate(".mpj");
  int i = Ustrlen(h);
  if (!Ustrcmp(h, name+Ustrlen(name)-i)) name[Ustrlen(name)-i]='\0';
  save_project(name);
  set_name(project_name);
}

static void save_project_name(void *data __attribute__((unused)), Char *c)
{
  Char *h = translate(".mpj");
  int i = Ustrlen(h), j = Ustrlen(c);
  if (!Ustrcmp(h, c+j-i)) c[j-i]='\0';
  save_project(c);
  set_name(project_name);
}

static void menu_quick_exit(void)
{
  menu_bad_end(NULL);
}

static void popup_quit(void *data __attribute__((unused)), int n)
{
    Char *c;
    switch (n) {
    case 0: menu_close(); break;
    case 1: menu_bad_end(NULL); break;
    case 2:
        message(MP_CLICKREMARK, translate("Sorry, not yet implemented."));
        break;
    case 3:
        c = concat(homedir, translate("mathspad/"));
        fileselc_open(save_project_name, NULL,
                      translate("Save the current state in a project file.\n"
                      "To be able to find it when you start again,\n"
                      "it will be placed in your mathspad directory."),
                      c, translate("*.mpj"), project_name, menuwin);
        break;
    default: break;
    }
}

static Char *selected_file=NULL;

extern void calculate_lazy_expression();

static void run_callbackfunc(void *data, Char *name)
{
  selected_file=name;
  calculate_lazy_expression(data);
}

static void open_fileselector(Char *comment, Char *dir, Char *mask,
			      Char *file, void *callbackfunc)
{
  fileselc_open(run_callbackfunc, callbackfunc,
		comment, dir, mask, file, menuwin);
}

static void open_dirselector(Char *comment, Char *dir, Char *mask,
			      Char *file, void *callbackfunc)
{
  dirselc_open(run_callbackfunc, callbackfunc,
	       comment, dir, mask, file, menuwin);
}

static void create_popup(void *data __attribute__((unused)), int n)
{
    if (mouse_button == Button3) {
      mainmenu[n].menu = popup_define(translate(mainmenuname[n]));
      (void) popup_make(mainmenu+n);
    } else {
      mainmenu[n].menu = popup_define(translate(mainmenuname[n]));
      popup_call_default(mainmenu+n);
    }
}


static void menu_press(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
  /* get_motion_hints(miniwin, -1);
    other_window(mini_info);
    */
}

static void menu_release(void *data __attribute__((unused)), XButtonEvent *event __attribute__((unused)))
{
  /* stop_motion_hints(); */
    /* handle_key(XK_Escape, 0); */
    /* set_window_keymap(get_map("Global")); */
}

static void menu_motion(void *data __attribute__((unused)), int x __attribute__((unused)), int y __attribute__((unused)))
{
}

static void menu_iconize(void *data __attribute__((unused)))
{
    menu_iconized = MP_True;
}

static void menu_deiconize(void *data __attribute__((unused)))
{
    menu_iconized = MP_False;
}

static int menu_last_pos(int *x, int *y, int *w, int *h)
{
    *x = win_xpos;
    *y = win_ypos;
    *h = win_height;
    *w = win_width;
    return MP_False;
}

static void menu_set_last_pos(int x, int y, int w, int h)
{
    win_xpos = x;
    win_ypos = y;
    win_height = h;
    win_width = w;
}    

FUNCTIONS menufuncs = {
    menu_bad_end, menu_draw, menu_resize, menu_press, menu_release,
    menu_motion, menu_iconize, menu_deiconize, NULL, NULL, menu_layout_change,
    notation_make_backups, menu_open, menu_state, NULL, NULL, menu_last_pos,
    menu_set_last_pos };

void menu_set_command(void)
{
    char *newarg[3];
    newarg[0] = arguments[0];
    newarg[1] = "-project";
    newarg[2] = UstrtoFilename(project_name);
    XSetCommand(display, menuwin, newarg, 3);
}

void menu_init()
{
}

void menu_open(int x, int y, int w, int h, int icon, int s __attribute__((unused)), Char *str)
{
    XSetWindowAttributes menu_attr;
    unsigned long menu_mask;
    int miniwin_x, miniwin_y;
    int i,j,mnleft,b_x, b_y;
    unsigned int row_width;
    XSizeHints size_hints;

    j=0;
    i=0;
    mnleft=0;
    free(str);
    for (i=0; i<NR_BUTTON; i++) {
        mainmenu[i].x=mainmenu[i].y= -1;
	mainmenu[i].sticky = mainmenu[i].freesub=0;
    }
    j=0;
    window_width = 0;
    set_change_function(update_selections);
    set_change_function(refresh_all);
    push_fontgroup(POPUPFONT);
    for (i=0; i<NR_ROWS; i++) {
        row_width = 0;
        while (j<rows[i]) {
            row_width += button_width(translate(menubutton[j]))+BINTERSPACE;
            j++;
        }
        if (window_width<row_width) window_width=row_width;
    }
    if (win_width<window_width) win_width = window_width;
    if (win_height<window_height) win_height = window_height;
    size_hints.min_height = size_hints.max_height = window_height;
    size_hints.min_width  = window_width;
    size_hints.max_width  = display_width;
    if (w>(int)win_width) win_width = w;
    if (h>(int)win_height) win_height = h;
    if (w) {
        win_xpos = x;
        win_ypos = y;
    } else if (!win_xpos && !win_ypos) {
        win_xpos = (display_width - win_width)/2;
        win_ypos = (display_height - win_height)/2;
    }
    line_y = NR_ROWS * (button_height+BINTERSPACE);
    miniwin_width = sub_width(win_width);
    miniwin_height = sub_height(win_height);
    miniwin_x = INTERSPACE;
    miniwin_y = line_y+ INTERSPACE;
    menu_mask = (CWBackPixel | CWBorderPixel | CWBitGravity | CWEventMask |
                 CWColormap);
    menu_attr.background_pixel = white_pixel;
    menu_attr.colormap=colormap;
    menu_attr.border_pixel = black_pixel;
    menu_attr.bit_gravity = NorthWestGravity;
    menu_attr.event_mask = (ExposureMask | ButtonPressMask |
                            ButtonReleaseMask | KeyPressMask |
			    FocusChangeMask |
                            PropertyChangeMask | StructureNotifyMask |
                            VisibilityChangeMask);
    menuwin = XCreateWindow(display, root_window,
                            win_xpos, win_ypos, win_width, win_height,
                            BORDERWIDTH, CopyFromParent, InputOutput,
                            visual,
                            menu_mask, &menu_attr);
    miniwin = XCreateWindow(display, menuwin, miniwin_x, miniwin_y,
                            miniwin_width, miniwin_height,
                            0, CopyFromParent, InputOutput,
                            visual,
                            menu_mask, &menu_attr);
    if (w)
        size_hints.flags = USPosition | USSize | PMinSize | PMaxSize;
    else
        size_hints.flags = PPosition | PSize | PMinSize | PMaxSize;
    wm_hints.initial_state = ((iconic || icon) ? IconicState : NormalState);
    wm_hints.input = MP_True;
    wm_hints.icon_pixmap = icon_pixmap;
    wm_hints.window_group = menuwin;
    wm_hints.flags = StateHint | IconPixmapHint | InputHint | WindowGroupHint;

    class_hints.res_name = (char*)UstrtoLocale(progname);
    class_hints.res_class = "MathEdit";

    XSetWMProperties(display, menuwin, NULL, NULL,
                     arguments, number_of_arguments,
                     &size_hints, &wm_hints, &class_hints);
    set_name(project_name);
    XSetWMProtocols(display, menuwin, protocol, 2);

    for (j=0; j<NR_BUTTON; j++) mainmenu[j].parentwin=menuwin;
    i=0;
    j=0;
    b_x = b_y = BINTERSPACE/2;
    if (add_window(menuwin, MENUWINDOW, root_window,
                   NULL, translate(helpname[CONSOLEHELP]))) {
        while (j<NR_BUTTON &&
               button_make(j,menuwin,translate(menubutton[j]),
                           &b_x,b_y,1,NULL, helpname[menuhelp[j]], NULL,NULL,
                           create_popup,
                           create_popup,
                           create_popup, NULL)) {
            j++;
            b_x+=BINTERSPACE;
            if (j==rows[i]) {
                b_x = BINTERSPACE/2;
                b_y += button_height+BINTERSPACE;
                i++;
            }
        }
        if (add_window(miniwin, MENUWINDOW, menuwin, NULL,
                       translate(helpname[CONSOLEHELP]))) j++;
    }
    if (j<NR_BUTTON+1) {
        XDestroyWindow(display, menuwin);
        XDestroyWindow(display, miniwin);
        message(MP_EXIT -1, translate("Unable to open menu window."));
    }
    set_selection_window(menuwin);
    mini_info = open_miniwindow( &miniwin, miniwin_width, miniwin_height);
    menu_is_open = MP_True;
    set_message_window(&miniwin);
    XMapSubwindows(display, menuwin);
    XMapWindow(display, menuwin);
    pop_fontgroup();
}

void menu_close(void)
{
    void *data, *unsaved;
    int i;

    i=0;
    unsaved = NULL;
    quit_sequence = MP_True;
    while (aig(data = next_data_with_type(MAINEDITWINDOW, &i)))
        if (edit_saved(data))
            edit_close(data);
        else {
            unsaved = data;
            i++;
        }
    if (unsaved==NULL) {
        /* there are no unsave documents. */
        if (aig(i=notation_not_saved(0)))
            notation_confirm_backup(i);
        else
            server_close();
    } else
        edit_close(unsaved);
}

void menu_bad_end(void *data __attribute__((unused)))
{
    void *dt;
    int i;

    i=0;
    menu_is_open = MP_False;
    while (aig(dt = next_data_with_type(MAINEDITWINDOW, &i)))
        edit_bad_end(dt);
    notation_make_backups(NULL,0);
    server_close();
}

static void menu_keysymbol(void)
{
    Char selchar;

    selchar = symbol_last();
    if (selchar) {
        insert_symbol(selchar,1);
    }
}

static void menu_number_shortcut(int number)
{
    int usenota;
    unsigned long uvnr;
    uvnr = (long) number;
    usenota = notation_with_number(uvnr);
    if (usenota >= 0) insert_notation((unsigned)usenota);
}

static void menu_number_shortcut_string(Char *number)
{
    int usenota;
    unsigned long uvnr;
    uvnr = Ustrtol(number,0,0);
    usenota = notation_with_number(uvnr);
    if (usenota >= 0) insert_notation((unsigned)usenota);
}

static void menu_notation_shortcut(Char *name)
{
    int usenota;
    int i=0,k=1,l;

    if (!name) return;
    l= Ustrlen(name)-1;
    while (l && Uisdigit(name[l])) {
        i=Utovalue(name[l])*k+i;
        k=k*10;
        l=l-1;
    }
    if (name[l]==':')
        name[l]='\0';
    else
        i=0;
    usenota = notation_with_name(name);
    if (usenota>=0) {
        usenota = nnr_vnr2innr(usenota, 0);
        if (i>0 && i<= which_notation(usenota)->versions)
            usenota = which_notation(usenota)->vers[i-1].ivnr;
        insert_notation((unsigned)usenota);
    }
    if (i) name[l]=':';
}

static void menu_notation(void)
{
    int usenota = notation_last();

    if (usenota >=0) insert_notation((unsigned)usenota);
}

static void menu_selected_notation(void)
{
  int vnr=0;
  int nr=ss_notation(&vnr);
  if (nr>=0) {
    insert_notation(nnr_vnr2innr(nr,vnr));
  }
}

static void ask_selection(void)
{
    get_wm_selection();
}

#define SEARCH 0
#define NOT_FOUND 1
#define WRAP_SEARCH 2
#define WRAP_NOT_FOUND 3

#define MAXLEN 500

static Char searchstr[MAXLEN], replacestr[MAXLEN], oldsearchstr[MAXLEN];
static int search_status = SEARCH;
static int stackkind[256];
static int stackd=0;
static Bool find_notation = MP_False;
static Bool backward_search = MP_False;
static Bool in_search = MP_True;
static Bool is_search = MP_True;
static Bool y_n_option = MP_False;
static unsigned int searchlen = 0, replacelen = 0, oldsearchlen = 0;

#define stackcode(A) ((search_status<<11) + (backward_search<<10)+(A))
#define statusfromstack(A) (stackkind[A]>>11)
#define backfromstack(A) ((stackkind[A]>>10)&0x1)
#define numberfromstack(A) (stackkind[A]&0x3FF)

static void draw_search(void)
{
    int i=0;

    push_fontgroup(POPUPFONT);
    set_output_window(&miniwin);
    out_clear();
    switch (search_status) {
    case NOT_FOUND:
        out_string(translate("Failing "));
        break;
    case WRAP_SEARCH:
        out_string(translate("Wrapped "));
        break;
    case WRAP_NOT_FOUND:
        out_string(translate("Failing wrapped "));
        break;
    default:
        break;
    }
    if (is_search) {
        Char *c;
        if (backward_search)
            out_string(translate("I-search backward :"));
        else
            out_string(translate("I-search :"));
        if (find_notation)
            c = stencil_screen(searchstr[0]);
        else {
            c = searchstr;
            searchstr[searchlen]=0;
        }
        while (c[i]) {
	  if  (!IsNewline(searchstr[i]))
	    out_char(searchstr[i]);
	  else
	    out_string(translate("^J"));
	  i++;
	}
	out_cursor(CURSOR);
    } else {
        Char *c;
        out_string(translate("Query replace"));
        if (in_search) out_char(((Char)(':')));
        out_char(((Char)' '));
        if (find_notation)
            c = stencil_screen(searchstr[0]);
        else {
            c = searchstr;
            searchstr[searchlen]=0;
        }
        while (c[i]) {
            if  (!IsNewline(c[i]))
                out_char(c[i]);
            else
                out_string(translate("^J"));
            i++;
        }
        if (in_search)
            out_cursor(CURSOR);
        else {
            out_string(translate(" with"));
            if (!y_n_option) out_char(':');
            out_char(' ');
            if (find_notation)
                c = stencil_screen(replacestr[0]);
            else {
                c = replacestr;
                replacestr[replacelen]=0;
            }
            i=0;
            while (c[i]) {
                if  (!IsNewline(c[i]))
                    out_char(c[i]);
                else
                    out_string(translate("^J"));
                i++;
            }
            if (!y_n_option)
                out_cursor(CURSOR);
            else {
                out_string(translate(": y,n,q or ! "));
                out_cursor(CURSOR);
            }
        }
    }
    unset_output_window();
    pop_fontgroup();
}

static void end_search(void)
{
    unsigned int i=0;

    clear_message(MP_True);
    /* pop_keymap(); */
    searching = MP_False;
    in_search = is_search = MP_True;
    oldsearchlen = 0;
    while (i<searchlen)
        oldsearchstr[oldsearchlen++] = searchstr[i++];
    oldsearchstr[i] = 0;
    oldsearchstr[i+1] = find_notation;
    find_notation = y_n_option = MP_False;
    search_status = SEARCH;
    backward_search = MP_False;
    searchstr[0] = searchlen = 0;
    replacestr[0] = replacelen = 0;
    clear_stack();
    stackd=0;
}

static void end_search_oldpos(void)
{
    clear_stack_and_use();
    end_search();
}

static void add_search_char(Char c, Index count)
{
    int i = count;

    if (!IsNewline(c) || is_search) {
        while (i && searchlen<MAXLEN-1) {
            searchstr[searchlen++] = c;
            i--;
        }
        searchstr[searchlen]=0;
        if (is_search) {
            stack_position();
            stackkind[stackd++] = stackcode(count-i);
            switch (search_status) {
            case SEARCH:
            case WRAP_SEARCH:
                if ((!backward_search && !find_string(searchstr)) ||
                    (backward_search && !find_backward_string(searchstr)))
                    search_status++;
                break;
            default:
                break;
            }
        }
    } else {
        in_search = MP_False;
        replacestr[0] = replacelen = 0;
    }
    draw_search();
}

static void add_replace_char(Char c, Index count)
{
    int i = count;

    if (!IsNewline(c)) {
        while (i && replacelen<MAXLEN-1) {
            replacestr[replacelen++] = c;
            i--;
        }
        replacestr[replacelen] = 0;
        draw_search();
    } else {
        if (!find_replace(searchstr)) {
            end_search();
            message(MP_MESSAGE, translate("Done."));
        } else {
            y_n_option = MP_True;
            /* push_temporary_keymap(get_map("answer"),0); */
            draw_search();
        }
    }
}

static void add_char(int code, Index count)
{
    if (!find_notation)
        if (in_search)
            add_search_char((Char)code, count);
        else
            add_replace_char((Char)code, count);
}

static void add_tab(Index count)
{
    if (!find_notation)
        if (in_search)
            add_search_char(Rtab, count);
        else
            add_replace_char(Rtab, count);
}

static void add_return(void)
{
    if (!find_notation)
        if (in_search)
            add_search_char(Newline, 1);
        else
            add_replace_char(Newline, 1);
}

static void add_sym(void)
{
    Char selchar;

    selchar = symbol_last();
    if (selchar && !find_notation)
        if (in_search)
            add_search_char( selchar, 1);
        else
            add_replace_char(selchar, 1);
}

static void add_search_notation(void)
{
    int usenota = notation_last();

    if (usenota>=0 && ((in_search && !searchlen) ||
                      (find_notation && !is_search && !replacelen))) {
        if (in_search) {
            searchstr[0] = usenota;
            searchlen = 1;
            find_notation = MP_True;
            replacestr[0] = replacelen = 0;
            in_search = is_search;
            if (is_search) {
                stack_position();
                stackkind[stackd++] = stackcode(1);
                switch (search_status) {
                case SEARCH:
                case WRAP_SEARCH:
                    if ((!backward_search && !find_stencil(searchstr[0])) ||
                        (backward_search && !find_backward_stencil(searchstr[0])))
                        search_status++;
                    break;
                default:
                    break;
                }
            }
            draw_search();
        } else {
            replacestr[0]=usenota;
            replacelen = 1;
            if (!find_replace_stencil(searchstr[0])) {
                end_search();
                message(MP_MESSAGE, translate("Done."));
            } else {
                y_n_option = MP_True;
                /* push_temporary_keymap(get_map("answer"),0); */
                draw_search();
            }
        }
    }
}

static void remove_search_char(Index count)
{
    if (is_search) {
        use_stack();
        if (stackd) {
            stackd--;
            count = numberfromstack(stackd);
            search_status = statusfromstack(stackd);
            backward_search = backfromstack(stackd);
        }
    }
    if (in_search) {
        if (find_notation) {
            searchlen = 0;
            find_notation = MP_False;
        } else
            if (count>searchlen)
                searchlen = 0;
            else
                searchlen -= count;
        searchstr[searchlen]=0;
    } else {
        if (count>replacelen)
            replacelen = 0;
        else
            replacelen -= count;
        replacestr[replacelen] = 0;
    }
    draw_search();
}

static void positive_yn(void)
{
    if (find_notation)
        replace_notation(searchstr[0], replacestr[0]);
    else
        replace_string(searchstr, replacestr);
    if ((find_notation && !findnext_replace_stencil(searchstr[0])) ||
        (!find_notation && !findnext_replace(searchstr)))
        end_search();
    else
        draw_search();
}

static void negative_yn(void)
{
    if ((find_notation && !findnext_replace_stencil(searchstr[0])) ||
        (!find_notation && !findnext_replace(searchstr)))
        end_search();
    else
        draw_search();
}

static void replace_all_yn(void)
{
    if (find_notation)
        replace_all_notation(searchstr[0], replacestr[0]);
    else
        replace_all(searchstr, replacestr);
    end_search();
}

static void start_find(void)
{
    searching = MP_True;
    stack_position();
    draw_search();
    /* push_temporary_keymap(get_map("search"),0); */
}

static void start_replace(void)
{
    stack_position();
    is_search = MP_False;
    draw_search();
    /* push_temporary_keymap(get_map("search"),0); */
}

static void start_backward_find(void)
{
    searching = MP_True;
    backward_search = MP_True;
    stack_position();
    draw_search();
    /* push_temporary_keymap(get_map("search"),0); */
}

static void do_find(void)
{
    unsigned int i=0;

    if (is_search) {
        if (!searchlen) {
            while (i<oldsearchlen) {
                searchstr[searchlen++] = oldsearchstr[i++];
            }
            searchstr[i]=0;
            find_notation = oldsearchstr[i+1];
        }
        stack_position();
        stackkind[stackd++] = stackcode(i);
        if (backward_search) {
            search_status = SEARCH;
            backward_search=MP_False;
        }
        switch (search_status) {
        case SEARCH:
        case WRAP_SEARCH:
            if ((find_notation && !findnext_stencil(searchstr[0])) ||
                (!find_notation && !findnext_string(searchstr)))
                search_status++;
            break;
        case NOT_FOUND:
            if ((find_notation && !findwrap_stencil(searchstr[0])) ||
                (!find_notation && !findwrap_string(searchstr)))
                search_status = WRAP_NOT_FOUND;
            else
                search_status++;
            break;
        case WRAP_NOT_FOUND:
            if ((find_notation && findwrap_stencil(searchstr[0])) ||
                (!find_notation && findwrap_string(searchstr)))
                search_status--;
            break;
        default:
            break;
        }
        draw_search();
    }
}

static void do_find_backward(void)
{
    unsigned int i=0;

    if (is_search) {
        if (!searchlen) {
            while (i<oldsearchlen) {
                searchstr[searchlen++] = oldsearchstr[i++];
            }
            searchstr[i]=0;
            find_notation = oldsearchstr[i+1];
        }
        stack_position();
        stackkind[stackd++] = stackcode(i);
        if (!backward_search) {
            search_status = SEARCH;
            backward_search = MP_True;
        }
        switch (search_status) {
        case SEARCH:
        case WRAP_SEARCH:
            if ((find_notation && !findprev_stencil(searchstr[0])) ||
                (!find_notation && !findprev_string(searchstr)))
                search_status++;
            break;
        case NOT_FOUND:
            if ((find_notation && !findwrap_backward_stencil(searchstr[0])) ||
                (!find_notation && !findwrap_backward_string(searchstr)))
                search_status = WRAP_NOT_FOUND;
            else
                search_status++;
            break;
        case WRAP_NOT_FOUND:
            if ((find_notation && !findwrap_backward_stencil(searchstr[0])) ||
                (!find_notation && !findwrap_backward_string(searchstr)))
                search_status--;
            break;
        default:
            break;
        }
        draw_search();
    }
}

static void latex_select(Index arg)
{
    if (set_wm_selection()) {
        tex_set_string(&latexselection);
        tex_placeholders(ON);
        switch (arg) {
	case 6: latex_all_parens(MP_True);
        case 2: tex_mode(PLAINTEX); break;
	case 7: latex_all_parens(MP_True);
        case 3: tex_mode(ASCII);    break;
	case 5: latex_all_parens(MP_True);
        case 1: tex_mode(MPTEX);    break;
        default:break;
        }
        latex_selection(1);
        tex_unset();
	latex_all_parens(MP_False);
        set_clipboard();
        message(MP_MESSAGE, translate("Selection converted to LaTeX."));
    } else
        message(MP_ERROR, translate("Unable to obtain the selection."));
}

static void make_project(void)
{
    popup_quit(NULL, 3);
}

static void filter_linenr(Char *name)
{
    int n;

    n=Ustrtol(name, 0,0);
    if (n) {
        goto_latex_line(n);
    }
    message(MP_MESSAGE, (Char*)"");
}

static void construct_string(Char *prompt, Char *defaultval,
			     void (*callback)(Char*))
{
  if (callback)
    message2(MP_MESSAGE, prompt, defaultval);
}

static void ask_latex_line(void)
{
    construct_string(translate("Go to LaTeX-line: "), (Char*)"", filter_linenr);
}

static void filter_savetime(Char *name)
{
    int n;

    n = Ustrtol(name, 0,0);
    if (n) {
        set_save_period(n);
    }
    message(MP_MESSAGE, (Char*)"");
}

static void ask_save_time(void)
{
  Char buffer[40];
  Char *s;
  buffer[39]=0;
  s= Ultostr(save_minute,buffer+39);
  Ustrcpy(popuptext,s);
  construct_string(translate("Minutes between automatic save: "),
		   popuptext, filter_savetime);
}

static void menu_insert_string(void)
{
    if (wmselection)
        insert_string(LocaletoUstr(wmselection));
}

static int argnr=0,bi=0,fi=0, cpos=-1;
static int bp[20] = {0};
static int bpd = 0;
static Char *argum[10] = {NULL};
static Char buffer[5000];
static Char *format=NULL;

static void use_argument(Char *name)
{
    int i=0;
    if (argnr) {
        argum[argnr] = concat(name,NULL);
        if (argum[argnr][0]) {
            while (name[i])
                buffer[bi++] = name[i++];
        } else if (bpd && bp[bpd-1]>=0) {
            Bool stop=MP_False;
            bi=bp[bpd-1];
            if (cpos>bi) cpos = bi;
            i = bpd;
            while (!stop) {
                while (format[fi] && format[fi]!='%') fi++;
                fi++;
                if (format[fi]==']') bpd--;
                if (format[fi]=='[') bpd++;
                stop = (!format[fi] || bpd <i);
                if (!stop && format[fi]>'0' && format[fi]<='9') {
                    i= format[fi]-'0';
                    if (!argum[i]) argum[i]= concat(NULL,NULL);
                }
            }
            if (format[fi]) fi++;
        }
    }
    while (format[fi]) {
        if (format[fi]=='%') {
            fi++;
            switch (format[fi]) {
            case 'c': cpos = bi; break;
            case '[': bp[bpd++] = bi; break;
            case ']': bpd--;          break;
            case 'n': buffer[bi++]='\n'; break;
            case 't': buffer[bi++]='\t'; break;
            case '%': buffer[bi++]='%';  break;
            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':
                i = format[fi]-'0';
                if (argum[i]) {
                    int j;
                    if (argum[i][0]) {
                        for (j=0; argum[i][j]; j++,bi++)
                            buffer[bi]=argum[i][j];
                    } else if (bpd && bp[bpd-1]>=0) {
                        Bool stop=MP_False;
                        bi=bp[bpd-1];
                        if (cpos>bi) cpos = bi;
                        j = bpd;
                        while (!stop) {
                            while (format[fi] && format[fi]!='%') fi++;
                            fi++;
                            if (format[fi]==']') bpd--;
                            if (format[fi]=='[') bpd++;
                            stop = (!format[fi] ||  bpd<j);
                            if (!stop && format[fi]>'0' && format[fi]<='9') {
                                j= format[fi]-'0';
                                if (!argum[j]) argum[j]= concat(NULL,NULL);
                            }
                        }
                        if (!format[fi]) fi--; 
                    }
                } else {
                    argnr = i;
                    fi++;
                    construct_string(translate("Argument: "), (Char*)"", use_argument);
                    return;
                }
                break;
            case '\0': fi--;
            default: return;
            }
            fi++;
        } else
            buffer[bi++] = format[fi++];
    }
    buffer[bi]='\0';
    insert_string(buffer);
    if (cpos>=0) {
        backward_char(1);
        forward_char(cpos);
    }
}

static void menu_permanent_message(Char *txt)
{
    if (txt) 
        out_permanent_message(txt);
    else
        out_permanent_message((Char*)"");
}

static void menu_filter_string(Char *txt)
{
    int i;
    for (i=0; i<10; i++)
        if (argum[i]) {
            free(argum[i]);
            argum[i]= NULL;
        }
    format = txt;
    buffer[0]='\0';
    bi = fi = 0;
    cpos = -1;
    bpd=0;
    argnr=0;
    if (!format) return;
    use_argument(translate(""));
}

static void switch_textdots(void)
{
    int i;
    void *data;

    textdots= !textdots;
    clear_tab_positions();
    i=0;
    while (aig(data = next_data_with_type(MAINEDITWINDOW, &i))) {
        i++;
        (*(eventfunc[EDITWINDOW]->draw))(data);
    }
    if (notadef_is_open && !notadef_iconized)
        (*(eventfunc[NOTATIONDEFWINDOW]->draw))(NULL);
    if (buffer_is_open && !buffer_iconized)
        (*(eventfunc[BUFFERWINDOW]->draw))(NULL);
    if (find_is_open && !find_iconized)
        (*(eventfunc[FINDWINDOW]->layout_change))(NULL);
}

static void print_selection_path(int full, int sub, Char **str)
{
  void *fsel, *ssel;
  char c;
  int l,i;
  int list[1024];
  fsel=get_selection(full);
  ssel=get_selection(sub);
  l=get_selection_path(fsel, ssel,list, 1024);
  *str=malloc(sizeof(Char)*(l+2));
  for (i=0; i<l; i++) {
    if (list[i]<10) (*str)[i]='0'+list[i];
    else if (list[i]<36) (*str)[i]='A'-10+list[i];
    else (*str)[i]='?';
  }
  (*str)[i]='\0';
}


#include "language.h"

static void use_map_string(Char *str)
{
  push_keymap(get_map(str));
}

static void use_temporary_map_string(Char *str, void *beforefunc, void *afterfunc)
{
  push_temporary_keymap(get_map(str),beforefunc, afterfunc);
}

static void reset_map(void)
{
  pop_keymap();
}

static void open_simple_program(Char *command, Char *title)
{
  open_program(command,title, NULL);
}

static void system_wrap(Char *command)
{
  system((char*)UstrtoLocale(command));
}

static void change_attribute(int *attributes, Char *attrname, Char *attrval)
{
  char *atname;
  char *localname;
  int i,j;
  i=0;
  localname=UstrtoLocale(attrname);
  while (aig(atname=font_get_name(i, -1)) && strcmp(atname,localname)) i++;
  if (!atname) return;
  j=0;
  localname=UstrtoLocale(attrval);
  while (aig(atname=font_get_name(i,j)) && strcmp(atname,localname)) j++;
  if (atname) {
    *attributes = font_change_attribute(*attributes, i,j);
  }
}

static void process_mpk_file(Char *str)
{
  if (lex_open_file(UstrtoFilename(str))) {
    parse_input();
    update_popups();
  } else {
    message2(MP_ERROR, translate("Unable to open mpk file "), str);
  }
}

static void context_sensitive_help(void)
{
  Char *c = window_find_help();
  if (c) open_helpfile(c,0);
  else message(MP_CLICKREMARK, translate("Sorry, no help available."));
}

static void find_load_items(Char *name)
{
  FILE *f;
  if (!name) {
    failure=MP_True;
    return;
  }
  if (!(f = fopen((char*)UstrtoFilename(name),"rb"))) {
    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
    failure=MP_True;
  } else {
    int i;
    i = edit_fnr;
    edit_fnr = 0;
    read_file(f,FINDREPFILE);
    unset_file();
    fclose(f);
    cleanup_nodestack();
    cleanup_filestack();
    cleanup_stencilstack();
  }
}

static void find_save_items(Char *name)
{
  FILE *f;
  if (!name) {
    failure=MP_True;
    return;
  }
  if (!(f=fopen((char*)UstrtoFilename(name), "wb"))) {
    message2(MP_CLICKREMARK, translate("Unable to open file "), name);
    failure=MP_True;
  } else {
    set_file(f);
    put_filecode(FINDREPFILE);
    put_findrep();
    unset_file();
    cleanup_stencilstack();
    fclose(f);
  }
}

static void popup_disable_item(Uchar *menuID, Uchar *label)
{
  popup_disable(menuID, label);
  update_popups();
}

static void popup_enable_item(Uchar *menuID, Uchar *label)
{
  popup_enable(menuID, label);
  update_popups();
}



#include "editlib.c"

static char *fontgroupname[NR_SIZE] =
  { "editfont", "stencilfont", "symbolfont", "popupfont" };

void menu_keyboard(void)
{
    int i;
    Type seltype;
    /*
    key_map = get_map("Global");
    print_map = get_map("print");
    search_map = get_map("search");
    y_n_map = get_map("answer");
    */
    seltype= define_type("Selection", sizeof(void*),construct_selection,
			 destruct_selection, copy_selection);
    for (i=0; protolist[i].callfunc; i++) {
      protolist[i].pt = define_prototype(protolist[i].tlist,
					 protolist[i].listlen,
					 protolist[i].rettype,
					 protolist[i].callfunc);
    }
    for (i=0; keysfuncs[i].func!=NULL; i++) {
      define_function(keysfuncs[i].namefunc, keysfuncs[i].description,
		      protolist[keysfuncs[i].protopos].pt,
		      keysfuncs[i].func);
    }
    define_program_variable(IntType, "pressedkey", &pressedkey);
    define_program_variable(StringType, "selected_file", &selected_file);
    for (i=0; i<NR_SIZE; i++) {
      define_program_variable(IntType, fontgroupname[i], &fontattributes[i]);
    }
    define_program_variable(IntType, "screen_line_space", &line_space);
    define_program_variable(IntType, "screen_tab_size", &screen_tab);
    define_program_variable(IntType, "screen_micro_space", &screen_space);
    define_program_variable(IntType, "left_margin", &latex_side);
    define_program_variable(IntType, "auto_save_time", &save_minute);

    define_program_variable(StringType,"document_dir", &userdir);
    define_program_variable(StringType,"stencil_dir", &notationdir);
    define_program_variable(StringType,"output_dir", &latexdir);

    define_program_variable(StringType,"latex_line_space", &latex_line_unit);
    define_program_variable(StringType,"latex_tab_size", &latex_tab_unit);
    define_program_variable(StringType,"latex_micro_space", &latex_space_unit);
}
