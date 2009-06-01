
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#include "sequence.h"
#include "popupmenu.h"
#include "unistring.h"

typedef struct PopupList PopupList;
struct PopupList {
  PopupList *next;
  PopupMenu *menu;
};

static PopupList *popuplist=0;

static PopupLine *last_added=NULL;

#define PULineSeparator 0
#define PULineSubmenu 1
#define PULineFunc 2

PopupMenu *popup_define(Uchar *name)
{
  PopupMenu *menu;
  PopupList *pl;
  pl=popuplist;
  while (pl && Ustrcmp(pl->menu->name,name)) pl=pl->next;
  if (pl) return pl->menu;
  menu = (PopupMenu*) malloc(sizeof(PopupMenu));
  if (!menu) return 0;
  memset(menu, 0, sizeof(PopupMenu));
  menu->name = malloc(sizeof(Uchar)*(Ustrlen(name)+1));
  Ustrcpy(menu->name, name);
  return menu;
}

void popup_store(PopupMenu *menu)
{
  PopupList *list;
  if (!menu) { fprintf(stderr,"No menu."); return; }
  list = malloc(sizeof(PopupList));
  list->next=popuplist;
  list->menu=menu;
  popuplist=list;
}

void popup_add_separator(PopupMenu *menu)
{
  PopupLine *pl;
  if (!menu) { fprintf(stderr,"No menu."); return; }
  pl = malloc(sizeof(PopupLine));
  memset(pl,0,sizeof(PopupLine));
  pl->linetype=PULineSeparator;
  pl->line.thickness=1;
  if (menu->lastline) {
    menu->lastline->next = pl;
    pl->prev=menu->lastline;
    menu->lastline= pl;
  } else {
    menu->firstline = menu->lastline = pl;
  }
  last_added=pl;
}

static void popup_clear_items(PopupMenu *menu)
{
  if (menu->firstline) {
    PopupLine *pl, *pln;
    pl=menu->firstline;
    while (pl) {
      pln=pl->next;
      switch (pl->linetype) {
      case PULineSeparator:
	free(pl);
	break;
      case PULineFunc:
	free(pl->string);
	free_sequence(pl->line.func);
	break;
      case PULineSubmenu:
	free(pl->string);
	break;
      default:
	break;
      }
      if (pl==last_added) last_added=NULL;
      free(pl);
      pl=pln;
    }
  }
  menu->firstline=menu->lastline=NULL;
}

void popup_set_title(PopupMenu *menu, Uchar *title)
{
  menu->title = malloc(sizeof(Uchar)*(Ustrlen(title)+1));
  Ustrcpy(menu->title,title);
  popup_clear_items(menu);
}

#define IsOpCode(A)  (0xF000<=(A) && (A)<=0xF7FF)

static int popup_line_diff(Uchar *s, Uchar *t)
{
  if (!s || !t) return 1; 
  while (*s || *t) {
    if (IsOpCode(*s)) s++;
    else if (IsOpCode(*t)) t++;
    else if (*s == *t) {
      s++;
      t++;
    } else return 1;
  }
  return 0;
}

static PopupLine *popup_find_line(PopupMenu *menu, Uchar *text)
{
  PopupLine *pl;
  pl=menu->firstline;
  while (pl) {
    if (!popup_line_diff(pl->string, text)) return pl;
    pl=pl->next;
  }
  return NULL;
}

void popup_add_line(PopupMenu *menu, Uchar *line, Sequence *func)
{
  PopupLine *pl;
  if (!menu) { fprintf(stderr,"No menu."); return; }
  pl = popup_find_line(menu, line);
  if (pl) {
    if (pl->linetype==PULineFunc) free_sequence(pl->line.func);
    free(pl->string);
  } else {
    pl = malloc(sizeof(PopupLine));
    memset(pl,0,sizeof(PopupLine));
    if (menu->lastline) {
      menu->lastline->next=pl;
      pl->prev=menu->lastline;
      menu->lastline=pl;
    } else
      menu->firstline=menu->lastline=pl;
  }
  pl->linetype=PULineFunc;
  pl->line.func=func;
  pl->enabled=1;
  pl->string=line;
  last_added=pl;
}

void popup_add_submenu(PopupMenu *menu, Uchar *line, Uchar *subname)
{
  PopupLine *pl;
  if (!menu) { fprintf(stderr,"No menu."); return; }
  pl = popup_find_line(menu, line);
  if (pl) {
    if (pl->linetype==PULineFunc) free_sequence(pl->line.func);
    free(pl->string);
  } else {
    pl = malloc(sizeof(PopupLine));
    memset(pl,0,sizeof(PopupLine));
    if (menu->lastline) {
      menu->lastline->next=pl;
      pl->prev=menu->lastline;
      menu->lastline=pl;
    } else
      menu->firstline=menu->lastline=pl;
  }
  pl->linetype=PULineSubmenu;
  pl->line.menu=popup_define(subname);
  pl->enabled=1;
  pl->string=line;
  last_added=pl;
}

void popup_make_default(PopupMenu *menu)
{
  PopupLine *pl;
  if (!menu || !menu->lastline)  { fprintf(stderr,"No menu(line)."); return; }
  if (last_added) {
    last_added->defaultline=1;
    pl= menu->firstline;
    while (pl) {
      if (pl!=last_added) pl->defaultline=0;
      pl=pl->next;
    }
  }
}

void popup_direction(PopupMenu *menu, int rtol)
{
  if (!menu) { fprintf(stderr,"No menu."); return; }
  menu->rightleft = rtol;
}

void popup_pinable(PopupMenu *menu)
{
  if (!menu) { fprintf(stderr,"No menu."); return; }
  menu->pinable = 1;
}

void popup_disable(Uchar *menuID, Uchar *label)
{
  PopupMenu *menu;
  PopupLine *pl;
  menu = popup_define(menuID);
  if (menu) pl = popup_find_line(menu, label);
  if (pl) pl->enabled=0;
}

void popup_enable(Uchar *menuID, Uchar *label)
{
  PopupMenu *menu;
  PopupLine *pl;
  menu = popup_define(menuID);
  if (menu) pl = popup_find_line(menu, label);
  if (pl) pl->enabled=1;
}


