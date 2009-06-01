/*
** File:     keyboard.h
** Purpose:  Handle keyboard mappings and convert keys.
**           Keymaps are stored in a size-dependent way.
*/
#ifndef MP_KEYBOARD_H
#define MP_KEYBOARD_H

#ifndef MP_SEQUENCE_H

#define Sequence void
extern int eval_sequence(Sequence *seq);
extern void calculate_lazy_expression(void *expr);
#endif

#include "keys.h"
#include "unicode.h"

typedef struct keymap KeyMap;

typedef struct {
    KeyNum keysym;   /* number of the key */
    KeyMode mode;    /* mode for modifiers (alt|control|shift|meta|...) */
    KeyMap *prefix;  /* sub map */
    Sequence *func;  /* function to call when this key is pressed */
} Key;

/* keymap uses a table, hashtable or tree to store the keys */

struct keymap {
    char storage_type;  /* how is this map stored */
    int numdef;         /* home many keys are defined */
    void *map;          /* the map itself  */
    Uchar *name;        /* the name of the map (if supplied) */
    Uchar *comment;     /* comment if help is needed  */
};

/* Define a (key,mode) combination in a map to perform a certain function */
extern void define_key(KeyMap *map,  KeyNum key, KeyMode mode, Sequence *func);

/* Get a map with a certain name */
extern KeyMap *get_map(Uchar *name);

/* Add some help information to the keyboard map */
extern void set_keymap_help(KeyMap *map, Uchar *help);

extern void clear_keymap(KeyMap *map);

/* Set a certain function for a range of keys. The function could refer to
** the variable currentkey if it has to be key-specific
*/
extern void define_keyrange(KeyMap *map, KeyNum keystart, KeyNum keyend,
			    KeyMode mode, Sequence *func);

/* Define a sequence of keys to perform some action.  The list
** of keys is available in the variable currentkeylist.
*/
extern void define_keysequence(KeyMap *map, int len, KeyNum *keylist,
			       KeyMode *modelist, Sequence *func);

/* stack operations */
extern void push_keymap(KeyMap *map);
extern void push_temporary_keymap(KeyMap *map, void *beforelazyfunc,
				  void *afterlazyfunc);
extern KeyMap *pop_keymap(void);

extern void set_window_keymap(KeyMap *map);

extern void handle_key(KeyNum key, KeyMode mode);

extern void keyboard_init(void);

/* construct a stack of keyboard mappings */
extern void* get_keyboard_stack(Uchar *stackpath);

/* set the current keyboard stack */
extern void  set_keyboard_stack(void* stack);

/* copy a keyboard stack */
extern void *copy_keyboard_stack(void* stack);

/* destroy a keyboard stack */
extern void destroy_keyboard_stack(void *stack);


#endif
