
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keys.h"


/* parse a string containing a description of a key
**
** To parse a string, use the system specific database from the
** window system.  There should be such a list on every system.
** Otherwise, it would not be too difficult to add our own
** database.
*/

int pressedkey=0;

/* syskeys.h contains macros or function definitions for system specific
** stringtokey and keytostring definitions.  It would be a symbolic
** link to a system specific file (X11syskeys.h, MACsymkeys.h, W95symkeys.h)
*/
#include "syskeys.h"

/* a keystring consists of modifier codes, followed by the name of the
** actual string.  The system specific code conversion is called for
** that string.  If it is converted correctly, that will be the result.
** Otherwise a built-in conversion routine is used.
*/

/* modifiers:
** C : control
** M : meta
** A : alt
** S : shift
** U : super
** H : hyper
** ~ : ignore following modifiers
*/

#define IGNOREMOD '~'
#define MODIFIERSEP '-'

static struct {
    KeyMode mode;
    char code;
} modifier[] = {
    {ModShift, 'S' },
    {ModCtrl, 'C' },
    {ModMeta, 'M' },
    {ModAlt, 'A' },
    {ModSuper, 'U'},
    {ModHyper, 'H'},
    {0,'\0'}};

int parse_key(char *keystr, KeyNum *keyres, KeyMode *moderes)
{
    /*
    ** Example CM~S-a  -> control-meta-a  (shift ignored)
    */
    int i;
    int ignore=0;
    KeyMode m;
    KeyNum k;
    char *c,*h;
    /* search for - */
    h=strchr(keystr, MODIFIERSEP);
    m=EmptyKeyMode;
    if (h && h!=keystr) {
	c=keystr;
	while (c<h) {
	    if (*c==IGNOREMOD) {
		ignore=1;
	    } else {
		i=0;
		while (modifier[i].code && modifier[i].code!=*c) i++;
		if (modifier[i].code) {
		    if (ignore) {
			m=ModeAddIgnore(m,modifier[i].mode);
		    } else {
			m=ModeAddNeed(m, modifier[i].mode);
		    }
		} else {
		    fprintf(stderr, "Unknown modifier code '%c'.\n", *c);
		}
	    }
	    c++;
	}
	h++;
    } else {
	h=keystr;
    }
    k=StringToKey(h);
    if (!k) {
      fprintf(stderr, "Unknown key: %s\n", h);
	/* default string handling */
	if (h[0] && !h[1]) {
	    k=*h;
	}
    }
    *keyres=k;
    *moderes=m;
    return (k>0);
}    

/* prints a key and mode in a string */

int print_key(char *keystr, int len, KeyNum key, KeyMode mode)
{
    int m,i,j;
    char *c;
    j=0;
    m=(mode>>16);
    i=0;
    while (modifier[i].code && j<len) {
	if (modifier[i].mode&m) keystr[j++]=modifier[i].code;
	i++;
    }
    m=(m^(mode&0xffff));
    if (m && j<len) {
	keystr[j++]=IGNOREMOD;
	i=0;
	while (modifier[i].code && j<len) {
	    if (modifier[i].mode&m) keystr[j++]=modifier[i].code;
	    i++;
	}
    }
    if (j && j<len) keystr[j++]=MODIFIERSEP;
    c=KeyToString(key);
    if (c) {
	while (*c && j<len) keystr[j++]=*c++;
    } else {
	if (0x20<=key && key<=0xff && j<len) keystr[j++]=key;
    }
    if (j<len) keystr[j]='\0';
    return j;
}

int convert_event(void *event, KeyNum *keyres, KeyMode *moderes)
{
  return EventToKeyAndMode(event, keyres, moderes);
}

int is_key_modifier(KeyNum key)
{
  return IsModifier(key);
}
