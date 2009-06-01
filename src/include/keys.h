
#ifndef KEYS_H
#define KEYS_H

#define EmptyKeyMode  0xffff

#define ModShift 0x01
#define ModCtrl  0x04
#define ModMeta  0x08
#define ModAlt   0x10
#define ModSuper 0x20
#define ModHyper 0x40

#define ModeAddNeed(M,K)   (((M)|((K)<<16))&(~(K)))
#define ModeAddIgnore(M,K) (((M)&(~((K)<<16)))&(~(K)))
#define ModeAdd(M,K)       ((M)|(((K)<<16)|(K)))

typedef unsigned int KeyNum;
typedef unsigned int KeyMode;

extern int pressedkey;

/* parses a string containing a description of a key */

extern int parse_key(char *keystr, KeyNum *keyres, KeyMode *moderes);

/* prints a key and mode in a string */

extern int print_key(char *keystr, int len, KeyNum key, KeyMode mode);

/* convert an event and return 1 if the event is processed by the
** keyboard handler
*/
extern int convert_event(void *event, KeyNum *keyres, KeyMode *moderes);

/* is a key a modifier? */
extern int is_key_modifier(KeyNum key);

#endif
