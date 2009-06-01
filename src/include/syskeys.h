#include "keys.h"

extern KeyNum StringToKey(char *name);
extern char *KeyToString(KeyNum key);
extern int EventToKeyAndMode(void *event, KeyNum *keyres, KeyMode *moderes);
extern int IsModifier(KeyNum key);
