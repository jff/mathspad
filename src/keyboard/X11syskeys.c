#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <string.h>

#include "unicode.h"
#include "unimap.h"
#include "unistring.h"
#include "X11syskeys.h"

/* from keyboard.c */
extern void set_input_string(Uchar *string, int len);
extern int multi_press(unsigned long t, int button);
extern void set_window_position(unsigned long window, int x, int y,
				unsigned long time);
extern int multi_release(unsigned long t);
extern int multi_drag(unsigned long t);

/* range not used for X11 keysyms */
/* subitems Down1,...,Down5 etc. are calculated but not defined. */
#define KeyButtonDown         0x7F00
#define KeyButtonDblDown      0x7F10
#define KeyButtonTripleDown   0x7F20
#define KeyButtonUp           0x7F40
#define KeyButtonDblUp        0x7F50
#define KeyButtonTripleUp     0x7F60
#define KeyDrag               0x7F80
#define KeyDblDrag            0x7F90
#define KeyTripleDrag         0x7FA0
#define KeyMotion             0x7FC0

#define IsDouble(A)  (((A)&0xFF30)==0x7F10)
#define IsTriple(A)  (((A)&0xFF30)==0x7F20)
#define IsDown(A)    (((A)&0xFFC0)==0x7F00)
#define IsUp(A)      (((A)&0xFFC0)==0x7F40)
#define IsDrag(A)    (((A)&0xFFC0)==0x7F80)
#define IsMotion(A)  (((A)&0xFFC0)==0x7FC0)
#define IsNum(A)     (((A)&0xFFC0)==0x7F00)
#define IsMouseKey(A) (((A)&0xFF00)==0x7F00)
#define MouseNum(A)   ((A)&0xf)

extern KeyMode default_ignore;

KeyNum StringToKey(char *s)
{
  KeySym ks;
  KeyNum k;
  if (!s) return 0;
  if (s[0] && !s[1]) return s[0]; /* locale specific */
  ks=XStringToKeysym(s);
  if (ks!=NoSymbol) return ks;
  /* name not found. Use own algorithm */
  k=KeyButtonUp;
  if (!strncmp(s,"double_", 7)) {
    k=k+(KeyButtonDblUp-KeyButtonUp);
    s=s+7;
  } else if (!strncmp(s,"triple_", 7)) {
    k=k+(KeyButtonTripleUp-KeyButtonUp);
    s=s+7;
  }
  if (!strncmp(s,"down_", 5)) {
    k=k+(KeyButtonDown-KeyButtonUp);
    s=s+5;
  } else if (!strncmp(s,"drag_",5)) {
    k=k+(KeyButtonUp-KeyDrag);
    s=s+5;
  }
  if (!strncmp(s,"mouse_", 6) && ('1'<=s[6] && s[6]<='5') && !s[7]) {
    k=k+s[6]-'0';
    return k;
  }
  if (k!=KeyButtonUp) {
    /* k changed and no correct key is found */
    return 0;
  }
  if (!strncmp(s,"mouse_movement", 14)) {
    k=KeyMotion;
    s=s+14;
    if (!*s) return k;
    if (*s=='_' && ('1'<=s[1] && s[1]<='5') && !s[2]) {
      k=k+s[1]-'0';
      return k;
    }
  }
  return 0;
}

static char keystringbuf[500];

char *KeyToString(KeyNum key)
{
    char *s;
    s = XKeysymToString(key);
    if (s) return s;
    s=keystringbuf;
    if (IsMouseKey(key)) {
      if (IsDouble(key)) { strcpy(s,"double_"); s=s+7; }
      if (IsTriple(key)) { strcpy(s,"triple_"); s=s+7; }
      if (IsDown(key)) { strcpy(s,"down_"); s=s+5; }
      if (IsDrag(key)) { strcpy(s,"drag_"); s=s+5; }
      strcpy(s,"mouse"); s=s+5;
      if (IsMotion(key)) { strcpy(s,"_movement"); s=s+9; }
      if (MouseNum(key)) { *s++='_'; *s++='0'+MouseNum(key); *s='\0'; }
      return keystringbuf;
    }
    return 0;
}

/* name of the map to convert a KeyNum to a unicode position */
/* Note: KeyNum will be truncated to 2-bytes to be able to use the map */
static char *mapkeysymname = "X11keysym";

/* mode numbers are chosen such that mapping X mask is easy. */

#define ALLMODS (ModShift|ModCtrl|ModMeta|ModAlt|ModSuper|ModHyper)

static const unsigned int modmasklist[] = {
    ShiftMask, ControlMask, Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask,0 };

static KeyMode statetomode(unsigned int state)
{
    return ((state&ALLMODS)|((state&ALLMODS)<<16));
}

static MapUchar keynummap=0;

static int keynumtounicode(KeyNum keynum)
{
  if (!keynummap) {
    MapUcharLoadFile(keynummap, mapkeysymname);
  }
  return MapValue(keynummap,keynum&0xffff);
}

static int keyandmodetochar(KeyNum keynum, KeyMode keymode)
{
  int i;
  i = keynumtounicode(keynum);
  if (i > 127 || i<32) return i;
  /* ignore shift */
  if (keymode & ModMeta) keynum=keynum+0x80;
  if (keymode & ModCtrl && (keynum&0x40)) {
    if (keynum & 0x20) keynum=keynum-32;
    keynum=keynum-64;
  }
  return keynum;
}
		   

int EventToKeyAndMode(void *event, KeyNum *keyres, KeyMode *moderes)
{
    XEvent *ev = (XEvent*) event;
    switch (ev->type) {
    case KeyPress:
	{
	    XKeyEvent *kev = &ev->xkey;
	    KeySym ks;
	    char buf[200];
	    int buflen=200;
	    int bres=0;
	    /* Using XLookupString. It seems like most applications use this
	    ** function and adjusting the keyboard using xmodmap remains
	    ** possible.
   	    ** The alternative is XKeycodeToKeysym, which needs
	    ** an index argument. The construction of that argument would
	    ** be system specific and lead to a difference in keyboard
	    ** handling between this module and other system, unless this
	    ** module can detect how XLookupString handles the modifiers
	    ** or variables are used to influence the handler.
	    */
	    bres=XLookupString(kev, buf, buflen, &ks,0);
	    /* input string might get processed by handler function */
	    set_input_string(LocaletoUstr(buf),bres);
	    /* remove those elements from state which are used by Lookup */
	    if (ks!=NoSymbol) {
		*moderes=statetomode(kev->state);
		if (default_ignore)
		    *moderes = (*moderes)& (~default_ignore);
		*keyres=ks;
		pressedkey=keyandmodetochar(*keyres, *moderes);
		return 1;
	    } else {
		return 0;
	    }
	}
	break;
    case KeyRelease:
	{
	    /* maybe handle modifier state here */
	    *keyres=NoSymbol;
	    *moderes=0;
	    return 1;
	}
	break;
    case ButtonPress:
	{ 
	    KeySym ks;
	    XButtonEvent *bev= &ev->xbutton;
	    int dbl;
	    dbl=multi_press(bev->time,bev->button);
            set_window_position(bev->window, bev->x, bev->y, bev->time);
	    switch (dbl) {
	    case -1: return 1;
	    case 0:  ks=KeyButtonDown+(bev->button);       break;
	    case 1:  ks=KeyButtonDblDown+(bev->button);    break;
	    default: ks=KeyButtonTripleDown+(bev->button); break;
	    }
	    *moderes=statetomode(bev->state);
	    *keyres=ks;
	    return 1;
	}
	break;
    case ButtonRelease:
	{
	    KeySym ks;
	    XButtonEvent *bev= &ev->xbutton;
	    int dbl;
	    dbl=multi_release(bev->time);
            set_window_position(bev->window, bev->x, bev->y, bev->time);
	    switch (dbl) {
	    case -1: return 1;
	    case 0:  ks=KeyButtonUp+(bev->button);       break;
	    case 1:  ks=KeyButtonDblUp+(bev->button);    break;
	    default: ks=KeyButtonTripleUp+(bev->button); break;
	    }
	    *moderes=statetomode(bev->state);
	    *keyres=ks;
	    return 1;
	}
	break;
    case MotionNotify:
	{
	    XMotionEvent *mev = &ev->xmotion;
	    int bdr;
	    /* convert motion to key and mode */
	    /* store position */
	    bdr=multi_drag(mev->time);
	    switch (bdr) {
	    case 1:
	    case 2:
	    case 3:
	    case 4:
	    case 5:
		*keyres=KeyDrag+bdr;
		*moderes=statetomode(mev->state);
		break;
	    case 0:
		*moderes=statetomode(mev->state);
		*keyres=KeyMotion;
                /* normal motion, useful for hot-zone detection */
		break;
	    default:
	        return 1;
	    }
	    /* get window position if needed */
	    if (mev->is_hint) {
		/* set field in event structure */
		XQueryPointer(mev->display, mev->window,
			      &mev->root, &mev->subwindow,
			      &mev->x_root, &mev->y_root,
			      &mev->x, &mev->y, &mev->state);
	    }
	    set_window_position(mev->window, mev->x, mev->y, mev->time);
	    return 1;
	}
	break;
    case MappingNotify:
	XRefreshKeyboardMapping(&ev->xmapping);
	return 1;
	break;
    default:
	break;
    }
    return 0;
}

int IsModifier(KeyNum key)
{
  return IsModifierKey(key);
}
