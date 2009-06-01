#ifndef MP_POPUPMENU_H
#define MP_POPUPMENU_H

#include "unicode.h"

typedef struct PopupMenu PopupMenu;
typedef struct PopupLine PopupLine;

struct PopupLine {
  Uchar *string;
  int enabled;
  int width,height;
  int defaultline;
  int linetype;
  union {
    Sequence *func;
    PopupMenu *menu;
    int thickness;
  } line;
  PopupLine *next, *prev;
};

struct PopupMenu {
  Uchar *title;
  Uchar *help;
  Uchar *name;
  int rightleft;
  int pinable;
  int width,height;
  PopupLine *firstline, *lastline;
};

extern PopupMenu *popup_define(Uchar *name);
extern void popup_store(PopupMenu *menu);
extern void popup_add_separator(PopupMenu *menu);
extern void popup_set_title(PopupMenu *menu, Uchar *title);
extern void popup_add_line(PopupMenu *menu, Uchar *line, Sequence *func);
extern void popup_add_submenu(PopupMenu *menu, Uchar *line, Uchar *subname);
extern void popup_make_default(PopupMenu *menu);
extern void popup_direction(PopupMenu *menu, int rtol);
extern void popup_pinable(PopupMenu *menu);
extern void popup_enable(Uchar *menuID, Uchar *label);
extern void popup_disable(Uchar *menuID, Uchar *label);


#endif
