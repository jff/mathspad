#ifndef MP_POPUP_H
#define MP_POPUP_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/

#include "language.h"

typedef struct MENU MENU;

struct MENU {
  Window parentwin;
  Window ownwin;
  int selline;
  int x,y,tw,th;
  int sticky;
  int freesub;
  void (*endfunc)(void*);
  void *enddata;
  int builded;
  MENU *submenu;
  MENU *mainmenu;
  PopupMenu *menu;
};

extern MENU *build_menu(Char *title);
extern void popup_set_termfunc(MENU *menu, void (*func)(void*), void *farg);
extern void add_item(MENU *menu,
		     Char *string, void (*func)(void*,int),
		     void *dataarg, int intarg);
extern MENU *popup_replace(MENU *menuold, MENU *menunew);

extern void popup_remove(Window win);
extern void popup_unmap(Window win);
extern void popup_map(Window win);
extern MENU *popup_make(MENU *menu);
extern void popup_init(void);
extern void popup_call_default(MENU *menu);
extern void update_popups(void);
#endif
