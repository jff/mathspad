#include <stdlib.h>
#include <stdio.h>

#include "keyboard.h"
#include "keys.h"

#include "unistring.h"

/* just place all requested keymaps in a linked list */
/* replacing it with a hash structure or tree should not be difficult */

typedef struct KeyMapList KeyMapList;
struct KeyMapList {
    KeyMap *keymap;
    KeyMapList *next;
};

#define LISTSTORE  0
#define ARRAYSTORE 1
#define HASHSTORE  2
#define TREESTORE  3

typedef struct KeyList KeyList;
struct KeyList {
    Key key;
    KeyList *next;
};

static KeyMapList *allkeymaps=0;

static KeyMap *new_keymap(void)
{
    KeyMap *km=malloc(sizeof(KeyMap));
    km->storage_type=LISTSTORE;
    km->map=0;
    km->name=0;
    km->comment=0;
    return km;
}

/* Get a map with a certain name */
KeyMap *get_map(Uchar *name)
{
    KeyMapList **kml=&allkeymaps;
    KeyMapList *km;

    while (*kml && (Ustrcmp((*kml)->keymap->name,name))) kml=&(*kml)->next;
    if (*kml) return (*kml)->keymap;
    *kml=km=malloc(sizeof(KeyMapList));
    km->next=0;
    km->keymap=new_keymap();
    km->keymap->name=malloc(sizeof(Uchar*)*(Ustrlen(name)+1));
    Ustrcpy(km->keymap->name, name);
    return km->keymap;
}

void clear_keymap(KeyMap *map)
{
}

static int key_match_exact(Key *k1, KeyNum num, KeyMode mode)
{
    return k1->keysym==num && k1->mode==mode;
}

static int key_match_pressed(Key *k1, KeyNum num, KeyMode mode)
{
    KeyMode tr;
    if (k1->keysym!=num) return 0;
    tr=k1->mode&mode;
    return (!(tr&0xffff) && ((k1->mode&0xffff0000)==tr));
}

static Key *resolve_key(KeyMap *map, KeyNum key, KeyMode mode,
			int (*cmpfunc)(Key*,KeyNum,KeyMode))
{
    switch (map->storage_type) {
    case LISTSTORE: {
	KeyList *kl;
	kl=map->map;
	while (kl && !(*cmpfunc)(&(kl->key),key,mode)) kl=kl->next;
	if (kl) return &kl->key;
	break;
    };
    default:
	fprintf(stderr, "Don't know how to resolve keys.\n");
	break;
    }
    return 0;
}

/* Define a (key,mode) combination in a map to perform a certain function */
void define_key(KeyMap *map,  KeyNum key, KeyMode mode, Sequence *func)
{
    Key *k;
    k=resolve_key(map,key,mode, key_match_exact);
    if (k) {
	/* (key,mode) combination is already defined, redefine */
	k->func=func;
	return;
    }
    /* add a new key to the list */
    switch (map->storage_type) {
    case LISTSTORE: {
	/* just add it in front, no check on double definitions */
	KeyList *kl;
	kl=malloc(sizeof(KeyList));
	kl->key.keysym=key;
	kl->key.mode=mode;
	kl->key.prefix=0;
	kl->key.func=func;
	kl->next=map->map;
	map->map=kl;
	break;
    }
    default:
	fprintf(stderr, "Don't know how to handle keymap storage.\n");
	break;
    }
}

/* Add some help information to the keyboard map */
void set_keymap_help(KeyMap *map, Uchar *help)
{
    map->comment=malloc((Ustrlen(help)+1)*sizeof(Uchar));
    Ustrcpy(map->comment,help);
}

/* Set a certain function for a range of keys. The function could refer to
** the variable currentkey if it has to be key-specific
*/
void define_keyrange(KeyMap *map, KeyNum keystart, KeyNum keyend,
		     KeyMode mode, Sequence *func)
{
    KeyNum k;
    for (k=keystart; k<=keyend; k++) {
	define_key(map, k, mode, func);
    }
}

/* Define a sequence of keys to perform some action.  The list
** of keys is available in the variable currentkeylist.
*/
void define_keysequence(KeyMap *map, int len, KeyNum *keylist,
			KeyMode *modelist, Sequence *func)
{
    int i;
    KeyMap *m=map;
    Key *k;
    for (i=0; i<len-1; i++) {
	/* define function for key to be empty */
	define_key(m, keylist[i], modelist[i], 0);
	/* key is defined, get the definition */
	k=resolve_key(m,keylist[i],modelist[i], key_match_exact);
	/* add a prefix map if available */
	if (!k->prefix) k->prefix=new_keymap();
	m=k->prefix;
    }
    define_key(m,keylist[i],modelist[i],func);
}


/* A stack of keymaps to handle keys.  As long as no special actions
** occur, the functions from the highest matching key combination are
** executed. If a special action occurs, like a focus change, the
** program has to update the stack of keymaps accordingly.
**
** A typical stack configuration is:
** global-map, window-map, mode-map, structure-maps
** (emacs)     (edit)      (latex)   (label)
**
** Using prefix keys from some map on the stack, will temporary change
** the stack to handle that prefix key.
**
** An example with the above stack and some key definition.
**                    situation
**             1          2           3 
** label      ---        function1   ---
** latex      prefix1    prefix1     prefix1
** edit       ---        ---         function2
** emacs      prefix2    prefix2     prefix2
**
** In situation 1, all maps on the stack enter the prefix mode.
** In situation 2, function1 is executed and the prefix functions are
**    not accessable while the label keymap is installed.
** In situation 3, all maps on the stack enter the prefix mode and
**    function2 is never executed for that key.
**
** Thus, the first encountered definition determines how a key is handled.
** If the definition is a function, that function is executed.  If the
** definition is a prefix map, all defined maps on the stack enter the
** prefix mode for that prefix key.
** It is not very wise to use keys as for both prefixes and functions,
** as it might confuse the user.
*/

typedef struct KeyStack KeyStack;
struct KeyStack {
    KeyStack *next;
    KeyMap *keymap;
    KeyMap *prefix;
    void *tempremovebefore; /* lazy expression, evaluated when the temporary
			    ** map is removed, before the key is processed
			    ** (For use with incremental search)
			    */
    void *tempremoveafter;  /* lazy expression, evaluated when the temporary
			    ** map is removed, after the key is processed
			    ** (For use with universal argument)
			    */
    int temporary; /* a temporary map is removed as soon as a
		   ** key can not be resolved within that map
		   ** Useful for argument building functions
		   ** like C-q and C-u in Emacs or for incremental
		   ** searches, where the first undefine key closed
		   ** the search and executes the selected action.
		   */
};

typedef struct KeyStackInfoRec {
    KeyStack *stack;
    int inprefix;
} KeyStackInfo;

static KeyStackInfo defaultkeystack = { 0,0};

static KeyStackInfo *currentstack=&defaultkeystack;

static void clear_state(void)
{
    KeyStack *k;
    k=currentstack->stack;
    while (k) {
	k->prefix=k->keymap;
	k=k->next;
    }
    currentstack->inprefix=0;
}

void push_keymap(KeyMap *map)
{
    KeyStack *k;
    clear_state();
    k=malloc(sizeof(KeyStack));
    k->next=currentstack->stack;
    k->prefix=k->keymap=map;
    k->tempremovebefore=0;
    k->tempremoveafter=0;
    k->temporary=0;
    currentstack->stack=k;
}

void push_temporary_keymap(KeyMap *map, void *beforelazyfunc,
			   void *afterlazyfunc)
{
    push_keymap(map);
    currentstack->stack->temporary=1;
    currentstack->stack->tempremovebefore=beforelazyfunc;
    currentstack->stack->tempremoveafter=afterlazyfunc;
}

KeyMap *pop_keymap(void)
{
    KeyStack *k;
    KeyMap *km;
    clear_state();
    k=currentstack->stack;
    if (k) {
	currentstack->stack=k->next;
	km=k->keymap;
	free(k);
    } else km=0;
    return km;
}

static void *afterlist[600];
static int afterlen=0;

void handle_key(KeyNum key, KeyMode mode)
{
    KeyStack *ks;
    Key *k;
    int found_prefix=0;
    int found_function=0;
    ks=currentstack->stack;
    /* If the shift modifier is used and the key is a valid input character,
    ** then the shift modifier should be ignored, as it is already used
    ** to select the input character.
    */
    if (mode&ModShift && pressedkey) mode=mode^ModShift;
    /* Perhaps some modifiers should be ignored by default.
    ** Under X11, the NumLock modifier is a locking modifier.
    ** On some systems, it will act as a Hyper (Mod4) modifier.
    */
    if (is_key_modifier(key)) {
      /* Modifier keys are handled specially. If a modifier key
      ** (like shift) is needed to get a different symbol (like *)
      ** handling the shift would complicate the definition of the
      ** keyboard mapping.
      ** Therefore, a modifier key will not change the current state
      ** of the keyboard and a modifier can not be used as a prefix.
      ** However, it is still possible to connect a function to
      ** modifier key (for example, to show the current state of
      ** the keyboard), so if such a function is defined, it
      ** should be called. Note that the called function can
      ** change the keyboard state, for example by selecting a
      ** different keymap.
      */
      while (ks) {
	if (ks->prefix) {
	  k=resolve_key(ks->prefix, key, mode, key_match_pressed);
	  if (k && k->func) {
	    found_function=1;
	    eval_sequence(k->func);
	    break;
	  }
	}
	ks=ks->next;
      }
    } else {  
      while (ks) {
	if (ks->prefix) {
	  k=resolve_key(ks->prefix, key,mode,key_match_pressed);
	  if (!k) ks->prefix=0;
	  else {
	    if (k->func && !found_prefix) {
	      /* found a function: call it */
	      found_function=1;
	      eval_sequence(k->func);
	      clear_state();
	      while (afterlen) {
		afterlen--;
		calculate_lazy_expression(afterlist[afterlen]);
	      }
	      break;
	    } else if (k->prefix) {
	      ks->prefix=k->prefix;
	      found_prefix=1;
	    } else {
	      ks->prefix=0;
	    }
	  }
	}
	if (ks->temporary && !found_prefix) {
	  /* remove everything up to this node */
	  KeyStack *cl;
	  void *ef=ks->tempremovebefore;
	  if (ks->tempremoveafter) {
	    afterlist[afterlen++]=ks->tempremoveafter;
	  }
	  cl=currentstack->stack;
	  currentstack->stack=ks->next;
	  while (cl!=currentstack->stack) {
	    ks=cl->next;
	    free(cl);
	    cl=ks;
	  }
	  /* execute the remove function if defined */
	  if (ef) calculate_lazy_expression(ef);
	  /* the remove function might have changed the keystack */
	  ks=currentstack->stack;
	} else {
	  ks=ks->next;
	}
      }
      if (!found_prefix && !found_function) {
	/* nothing found, reset stack */
	/* beep if wanted */
	clear_state();
      } else {
	currentstack->inprefix=found_prefix;
      }
    }
}

/* An argument building function is performed by using a small keyboard
** map, which is placed on top of the keyboard stack and marked as
** temporary.  As long as the next key event is defined in that small map,
** that small map is used. Otherwise, the temporary map is remove and
** a handle function is called before the key event is further processed.
** The handle function can simulate other events, set variables or clear
** pre-edit fields.
*/

KeyMode default_ignore=0;
static unsigned long doublediff=300;
static unsigned long dragdiff=300;
static seqcount=0;

static unsigned long lasttime;
static int buttonnum=0;
static int prdiff=0;

int multi_press(unsigned long t, int button)
{
    /* lasttime comes from multi_release */
    prdiff++;
    if (prdiff>1) return -1;
    if (t-lasttime<doublediff) {
	seqcount++;
    } else seqcount=0;
    buttonnum=button;
    lasttime=t;
    return seqcount;
}

int multi_release(unsigned long t)
{
    /* lasttime comes from multi_press */
    if (!prdiff) return -1;
    prdiff--;
    if (prdiff) return -1;
    lasttime=t;
    return seqcount;
}

int multi_drag(unsigned long t)
{
    /* lasttime comes from multi_press */
    if (!prdiff) {
	/* no button pressed. just movement */
	return 0;
    }
    if (t-lasttime>dragdiff) {
	return buttonnum;
    }
    return -1;
}

static Uchar isbuf[256];
static int buflen=0;

void set_input_string(Uchar *string, int len)
{
    int i;
    if (len<255) i=len; else i=255;
    Ustrncpy(isbuf,string, i);
    isbuf[i]='\0';
    buflen=i;
}

Uchar *get_input_string(int *len)
{
    *len=buflen;
    return isbuf;
}

static unsigned long cur_window, cur_time;
static int cur_x,cur_y;

void set_window_position(unsigned long window, int x, int y,
			 unsigned long time)
{
    cur_window=window;
    cur_x=x;
    cur_y=y;
    cur_time=time;
}

void keyboard_init(void) { }


void *get_keyboard_stack(Uchar *stackpath)
{
  KeyStackInfo *result;
  Uchar *c;
  Uchar buffer[500];
  int pos;
  result = malloc(sizeof(KeyStackInfo));
  result->stack=0;
  result->inprefix=0;
  while (stackpath && *stackpath) {
    KeyMap *km;
    pos=0; 
    while (pos<499 && *stackpath && *stackpath!=',') {
      buffer[pos]=*stackpath;
      stackpath++;
      pos++;
    }
    buffer[pos]=0;
    km = get_map(buffer);
    if (km) {
      KeyStack *stitem;
      stitem = malloc(sizeof(KeyStack));
      memset(stitem, 0, sizeof(KeyStack));
      stitem->keymap=km;
      stitem->prefix=km;
      stitem->next=result->stack;
      result->stack=stitem;
    }
    if (*stackpath==',') stackpath++;
  }
  return result;
}

void set_keyboard_stack(void *stack)
{
  if (stack) {
    if (currentstack) clear_state();
    currentstack=(KeyStackInfo*)stack;
  }
}

void *copy_keyboard_stack(void *stack)
{
  KeyStackInfo *result, *ast;
  KeyStack *orig, **dest;
  if (!stack) return 0;
  ast = (KeyStackInfo*) stack;
  result = malloc(sizeof(KeyStackInfo));
  *result = *ast;
  /* copy ast->stack */
  dest=&result->stack;
  orig=ast->stack;
  while (orig) {
    *dest = malloc(sizeof(KeyStack));
    **dest = *orig;
    dest = &((*dest)->next);
    orig=orig->next;
  }
  return result;
}

void destroy_keyboard_stack(void *stack)
{
  KeyStackInfo *ksi;
  KeyStack *ks;
  if (!stack) return;
  ksi = (KeyStackInfo*) stack;
  if (ksi==currentstack) {
    clear_state();
    currentstack = &defaultkeystack;
  }
  while (ksi->stack) {
    ks = ksi->stack;
    ksi->stack = ks->next;
    free(ks);
  }
  free(ksi);
}
