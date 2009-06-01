/*
**  File:    unitype.h
**  Purpose: provide simular functions for standard C functions to handle
**           Unicode. Most of the functions from ctype.h are available,
**           where the name is prefixed with "U" and the meaning remains
**           the same where possible  (is* -> Uis*)
**           Some other macros are added for other useful properties.
**           Conversion routines are also available.
**
**           All properties and conversions use dynamic tables, which
**           are loaded at initialisation according to environment
**           variables, built-in defaults and locale information.
**
**           Support for different tables at the same time should not
**           be difficult to add, if context switch function are added.
*/

#ifndef UNITYPE_H
#define UNITYPE_H

#include "unicode.h"

/* for character types */
#define IS_NONSPACE     (1<<0)
#define IS_COMBINING    (1<<1)
#define IS_COMBMOD      (IS_COMBINING|IS_NONSPACE)

#define IS_DIGIT        (1<<2)
#define IS_OTHER_NUM    (1<<3)
#define IS_NUM          (IS_DIGIT|IS_OTHER_NUM)

#define IS_SPACE        (1<<4)
#define IS_LINE         (1<<5)
#define IS_PARAGRAPH    (1<<6)
#define IS_BLANK        (IS_SPACE|IS_LINE|IS_PARAGRAPH)

#define IS_CONTROL      (1<<7)
#define IS_PRIVATE      (1<<8)
#define IS_NOT_ASSIGNED (1<<9)
#define IS_CONTCLASS    (IS_CONTROL|IS_PRIVATE|IS_NOT_ASSIGNED)

#define IS_UPPERCASE    (1<<10)
#define IS_LOWERCASE    (1<<11)
#define IS_TITLECASE    (1<<12)
#define IS_MODIFIER     (1<<13)
#define IS_OTHER_LETTER (1<<14)
#define IS_ALPHA        (IS_UPPERCASE|IS_LOWERCASE|IS_TITLECASE|\
			 IS_MODIFIER|IS_OTHER_LETTER)

#define IS_DASH         (1<<15)
#define IS_OPEN         (1<<16)
#define IS_CLOSE        (1<<17)
#define IS_OTHER_PUNCT  (1<<18)
#define IS_PUNCT        (IS_DASH|IS_OPEN|IS_CLOSE|IS_OTHER_PUNCT)

#define IS_MIRROR       (1<<19)

#define IS_MATH         (1<<20)
#define IS_CURRENCY     (1<<21)
#define IS_OTHER_SYMBOL (1<<22)
#define IS_SYMBOL       (IS_MATH|IS_CURRENCY|IS_OTHER_SYMBOL)

#define IS_GRAPH        (IS_COMBMOD|IS_NUM|IS_ALPHA|IS_PUNCT|IS_SYMBOL)
#define IS_PRINT        (IS_GRAPH|IS_BLANK)

/* for bidi algorithm */
#define IS_LEFT_RIGHT    (1<<23)
#define IS_RIGHT_LEFT    (1<<24)
#define IS_EURO_NUM      (1<<25)
#define IS_EURO_NUM_SEP  (1<<26)
#define IS_EURO_NUM_TERM (1<<27)
#define IS_ARAB_NUM      (1<<28)
#define IS_WHITESPACE    (1<<29)
#define IS_OTHER_NEUT    (1<<30)


/* decomposition types */
#define DEC_CANON 1
#define DEC_CIRCLE 2
#define DEC_COMPAT 3
#define DEC_FINAL 4
#define DEC_FONT 5
#define DEC_FRACTION 6
#define DEC_INITIAL 7
#define DEC_ISOLATED 8
#define DEC_MEDIAL 9
#define DEC_NARROW 10
#define DEC_NOBREAK 11
#define DEC_SMALL 12
#define DEC_SQUARE 13
#define DEC_SUB 14
#define DEC_SUPER 15
#define DEC_VERTICAL 16
#define DEC_WIDE 17

/* combining class, numbers from unicode data file  */
#define CCOverlay            1
#define CCTibSubjoin         6
#define CCNukta              7
#define CCHiraKataVoiced     8
#define CCVirama             9
#define CCBelowLeftAttached  200
#define CCBelowAttached      202
#define CCBelowRightAttached 204
#define CCLeftAttached       208 /* reordrant around single base character */
#define CCRightAttached      210
#define CCAboveLeftAttached  212
#define CCAboveAttached      214
#define CCAboveRightAttached 216
#define CCBelowLeft          218
#define CCBelow              220
#define CCBelowRight         222
#define CCLeft               224 /* reordrant around single base character */
#define CCRight              226
#define CCAboveLeft          228
#define CCAbove              230
#define CCAboveRight         232
#define CCDoubleAbove        234

/* declare a collection of unicode tables.  Most tables are rather small
** since their domain includes only small ranges
*/

extern MapInt  uctype;       /* character types/bidi behaviour */
extern MapUchar uclow;       /* lowercase */
extern MapUchar ucup;        /* upcase */
extern MapUchar uctitle;     /* title case */
extern MapChar uchexval;     /* hexvalue */
extern MapChar uccancom;     /* canonical composition class */
extern MapUstr ucdecomp;     /* decomposition string */
extern MapChar ucdetag;      /* decomposition information */
extern MapUchar ucinitial;   /* arabic initial form */
extern MapUchar ucmedial;    /* arabic medial form */
extern MapUchar ucfinal;     /* arabic final form */
extern MapUchar ucisolated;  /* arabic isolated form */
extern MapUchar ucvalue;     /* value of character */
extern MapUchar ucdenum;     /* denumerator value, usually 1 */
extern MapUchar ucspacevar;  /* spacing variant */
extern MapUchar ucmirror;    /* for bidi algorithm */
extern MapUchar ucnoaccent;  /* for getting base characters */

#define Uisalpha(c)  (MapValue(uctype,c)&(IS_ALPHA))
#define Uisupper(c)  (MapValue(uctype,c)&(IS_UPPERCASE))
#define Uislower(c)  (MapValue(uctype,c)&(IS_LOWERCASE))
#define Uistitle(c)  (MapValue(uctype,c)&(IS_TITLECASE))
#define Uismodifier(c) (MapValue(uctype,c)&(IS_MODIFIER))
#define Uisdigit(c)  (MapValue(uctype,c)&(IS_DIGIT))
#define Uisspace(c)  (MapValue(uctype,c)&(IS_BLANK))
#define Uispunct(c)  (MapValue(uctype,c)&(IS_PUNCT))
#define Uisalnum(c)  (MapValue(uctype,c)&(IS_ALPHA|IS_NUM))
#define Uissymbol(c) (MapValue(uctype,c)&(IS_SYMBOL))
#define Uisprint(c)  (MapValue(uctype,c)&(IS_PRINT))
#define Uisgraph(c)  (MapValue(uctype,c)&(IS_GRAPH))
#define Uiscntrl(c)  (MapValue(uctype,c)&(IS_CONTROL))

#define Uiscombining(c)  (MapValue(uctype,c)&(IS_COMBINING))
#define Uisnonspacing(c) (MapValue(uctype,c)&(IS_NONSPACE))

#define Uismath(c)   (MapValue(uctype,c)&(IS_MATH))
#define Uiscurrency(c)  (MapValue(uctype,c)&(IS_CURRENCY))
#define Uisopen(c)   (MapValue(uctype,c)&(IS_OPEN))
#define Uisclose(c)  (MapValue(uctype,c)&(IS_CLOSE))

/* for bidi algorithms */
#define Uismirrored(c)  (MapValue(uctype,c)&(IS_MIRROR))

#define Uisleftright(c) (MapValue(uctype,c)&(IS_LEFT_RIGHT))
#define Uisrightleft(c) (MapValue(uctype,c)&(IS_RIGHT_LEFT))
#define Uiseuronum(c)   (MapValue(uctype,c)&(IS_EURO_NUM))
#define Uiseurosep(c)   (MapValue(uctype,c)&(IS_EURO_NUM_SEP))
#define Uiseuroterm(c)  (MapValue(uctype,c)&(IS_EURO_NUM_TERM))
#define Uisarabnum(c)   (MapValue(uctype,c)&(IS_ARAB_NUM))
#define Uiswhite(c)     (MapValue(uctype,c)&(IS_WHITESPACE))
#define Uisneutral(c)   (MapValue(uctype,c)&(IS_OTHER_NEUT))

#define Uisprivate(c) ((0xE000<=(c)&&(c)<=0xF8FF)||\
		       (0xDB80<=(c)&&(c)<=0xDBFF))

#define Uissurrogatehigh(c) ((c)>=0xD800 && (c)<0xDC00)
#define Uissurrogatelow(c) ((c)>=0xDC00 && (c)<0xE000)

#define Usurrogate(H,L) (0x10000+((((H)&0x03FF)<<11)|((L)&0x03FF)))

/* these macros only work correct for c>0x10000) */
#define Utosurrogatehigh(c) (((((c)-0x10000)>>11)&0x03FF)|0xD800)
#define Utosurrogatelow(c)  (((c)&0x03FF)|0xDC00)

#define Uisxdigit(c) ((MapValue(uchexval,c))!= -1)

#define UCheckConv(m,c) (MapValue(m,(c))?MapValue(m,(c)):(c))

#define Utolower(c)  (UCheckConv(uclow,c))
#define Utoupper(c)  (UCheckConv(ucup,c))
#define Utotitle(c)  (UCheckConv(uctitle,c))
#define Utonoaccent(c) (UCheckConv(ucnoaccent,c))

#define Utoinitial(c)  (UCheckConv(ucinitial,c))
#define Utomedial(c)   (UCheckConv(ucmedial,c))
#define Utofinal(c)    (UCheckConv(ucfinal,c))
#define Utoisolated(c) (UCheckConv(ucisolated,c))

#define Utovalue(c)  (MapValue(ucvalue,c))
#define Utofvalue(c) ((float)(MapValue(ucvalue,c))/((float)MapValue(ucdenum,c)))
#define Utoxvalue(c) (MapValue(uchexval,c))


/* for decomposition: a decomposition string and a type (font/super/sub) */
/* decomposition string */
#define Udecomp(c) (MapValue(ucdecomp,c))
/* type of decomposition (a tag) */
#define Udecomptype(c) (MapValue(ucdetag,c))


/* for bidi */
#define Utomirror(c) (MapValue(ucmirror,c))

/* for adding accents */
#define Utospacing(c) (MapValue(ucspacevar,c))
#define Ucombclass(c) (MapValue(uccancom,c))

/* initialising function for unitype. */
extern void unitype_init(void);

#endif



