#ifndef MP_SOURCES_H
#define MP_SOURCES_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/
 /*  File : sources.h
 **  Datum: 27-3-92
 **  Doel : initialiseren en eindigen van windowomgeving.
 **         registeren van windows en bepalen van windowtype.
 **         beschikbaar maken van fonts en gc's.
 **         initialiseren van algemene constanten (later mogelijk via file)
 */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

typedef enum {
    NOWINDOW, BUTTONWINDOW, SCROLLBARWINDOW, STRINGWINDOW, MENUWINDOW,
    MAINEDITWINDOW, EDITWINDOW, MAINBUFFERWINDOW, BUFFERWINDOW,
    MAINNOTATIONWINDOW, NOTATIONWINDOW, NOTATIONDEFWINDOW,
    SYMBOLWINDOW, DEFAULTWINDOW, REMARKWINDOW, FINDWINDOW, FILESELCWINDOW,
    POPUPWINDOW, CHECKBOXWINDOW, MAXWINDOWTYPE
} WINDOWTYPE;

#define MPTEX 1
#define PROOFTEX 2
#define PLAINTEX 3
#define ASCII 4

#include "fonts.h"
#include "unistring.h"

extern Char *translate(char *string);

typedef struct {
    void (*bad_end)(void*);
    void (*draw)(void*);
    void (*configure)(void*, XConfigureEvent*);
    void (*press)(void*, XButtonEvent*);
    void (*release)(void*, XButtonEvent*);
    void (*motion)(void*,int,int);
    void (*iconize)(void*);
    void (*deiconize)(void*);
    void (*leave)(void*);
    void (*enter)(void*);
    void (*layout_change)(void*);
    void (*auto_save)(void*,int);
    void (*use_state)(int,int,int,int,int,int,Char*);
    void (*state)(void*,int*,int*,int*,int*,int*,int*,Char**);
    int  (*margin)(void*);
    void (*set_lines)(void*,int);
    int  (*last_pos)(int *, int*,int*,int*); /* x,y,w,h */
    void (*set_last_pos)(int, int,int,int);
    void (*dbl_click)(void *);
    void (*property_change)(void *, XPropertyEvent*);
} FUNCTIONS;

extern FUNCTIONS *eventfunc[MAXWINDOWTYPE];
extern FUNCTIONS mainbufferfuncs, bufferfuncs, buttonfuncs, defaultfuncs,
                 maineditfuncs, editfuncs, findfuncs, stringfuncs, menufuncs,
		 notadeffuncs, mainnotationfuncs, notationfuncs, remarkfuncs,
		 scrollbarfuncs, symbolfuncs, fileselcfuncs, popupfuncs,
		 checkboxfuncs;

extern Display *display;
extern Visual *visual;
extern Colormap colormap;
extern Bool iconic,savestate,output_mode;
extern unsigned long save_time, message_time, last_time;
extern int save_minute;
extern char *wmselection;
extern char *latexselection;
extern unsigned long save_period;
extern int screen_num;
extern unsigned int display_height, display_width;
extern unsigned long white_pixel, black_pixel;
extern XClassHint class_hints;
extern Atom WM_SAVE_YOURSELF, WM_DELETE_WINDOW;
extern Atom protocol[2];
extern XWMHints wm_hints;
extern Pixmap icon_pixmap;
extern Window root_window;

#define set_protocols(A)  XSetWMProtocols(display, (A), protocol, 1)

extern Char *project_name;

extern Window motion_window;
extern Window when_motion_window;
extern Bool   motion_set, motion_wait, wait_for_time,
              is_click, double_click;
extern unsigned long press_time, release_time;
extern unsigned int  mouse_button;
extern unsigned int  press_state;
extern Bool can_auto_save;

extern unsigned long colorlist[8][2];
extern GC   get_GC(TextMode gcnr, int colortype, int colorpos);
extern GC   get_GCXor(int colortype);
extern int  get_color(Char *color);
extern int  must_underline(int colortype);
extern GC   get_GC_font(TextMode gcnr, int colortype, int colorpos,
			FontID fontid);
extern void undefined_font(TextMode gcnr);

extern WINDOWTYPE get_window_type(Window win, Window *pwin, void **data);
extern Bool exist_window(Window win);
extern int  add_window(Window win, WINDOWTYPE wtype, Window pwin, void *data,
		       Char* helpfile);
extern void window_def_keyboard(Char *windowtype, Char *list);
extern void* window_keyboard(Window win);
extern Char* window_find_help(void);
extern Char* window_help(Window win);
extern void change_visibility(Window win, int state);
extern void change_mapped(Window win, int mapped);
extern void refresh_all(void);
extern void *next_data_with_type(WINDOWTYPE wtype, int *i);
extern void *remove_window(Window win);
extern void destroy_window(Window win);     /* verwijdert data, geen window */
extern void set_selection_window(Window win);
extern void get_wm_selection(void);
extern Bool set_wm_selection(void);
extern void set_clipboard(void);
extern void send_selection(XSelectionRequestEvent *event);

extern void server_init(void);
extern void server_close(void);

extern Bool mouse_press(unsigned int state, unsigned int bnr);
extern Bool mouse_release(unsigned int state, unsigned int bnr);
extern void set_save_period(int minute);
extern Bool is_drag(unsigned long motion_time);
extern Bool motion_get_pos(int *x, int *y);
extern void get_motion_hints(Window win, int ms);
extern void stop_motion_hints(void);

extern void set_wait_cursor(Window win);
extern void remove_wait_cursor(void);

extern void create_needed_directories(int n, Bool made);

extern void make_defaults(int argc, char *argv[]);
extern void save_defaults(void);

extern void close_save_state_file(void);
extern Bool get_save_state_file(Char *name);
extern Bool get_save_entry(WINDOWTYPE *type, int *xpos, int *ypos,
			   int *width, int *height, Bool *as_icon, int *sbpos,
			   Char **string);

extern void load_project(Char *name);
extern void save_project(Char *name);
extern void call_layout_change(void);
extern void call_auto_save(unsigned long cur_time);

extern void window_manager_added(Window  win, int *x, int *y);
#endif
