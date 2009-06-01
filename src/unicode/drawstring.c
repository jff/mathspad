#include "unicode.h"
#include "unitype.h"
#include "unifont.h"
#include <stdio.h>

/* a WordItem resembles the X11 XTextItem16. */
/* a FontChar resembles the X11 XChar2b */
typedef struct {
    unsigned char byte1;
    unsigned char byte2;
} FontChar;

typedef struct {
    FontChar *chars;
    int nchars;
    int delta;
    Font fontid;
} WordItem;

static WordItem drawitem[5000];
static FontChar drawstring[5000];

/* all spacing alternatives are placed on a line and will not
** change the main text stream. The line is selected by the
** offset of the diacritic, which depends on the size of the glyph
** and the how it should be combined.
*/
/* number of allowed accents */
#define MAXACCENT 200
/* number of different vertical offset position.
** If the string starts with a character with 25 accents, the rest
** of the string might not be rendered perfect.
*/
#define MAXOFFSET 25

static FontChar accentstring[MAXACCENT]; /* contains the actual accents */
static int accentpos=0;              /* position in accentstring to use */

static struct {
    WordItem  item[MAXACCENT];
    int length;  /* length of the itemlist when drawn (in pixels) */
    int pos;     /* position of item to use next (number of used items) */
    int hoffset; /* position where first item should be drawn (in pixels) */
    int voffset; /* vertical offset of accents */
} accentline[MAXOFFSET];
static int maxaccentline=0;

/* An example (length is in number of characters instead of pixels):
** 
** String to draw:   abc^d`eg     (^ and ` on top of c and d)
** To place them correct, suppose ^ should be shifted 2 up and ` 5 up.
** Then, the following data will be constructed, (assuming no font switches
** are needed)
**
** drawitem[0]:    chars=drawstring
**                 nchar=6
**                 delta=0
** drawstring:     "abcdeg"
** accentline[0]:  length=3
**                 pos=1
**                 hoffset=2
**                 voffset=2
**                 item[0].delta=0
**                 item[0].chars=accentstring
**                 item[0].nchars=1
** accentline[1]:  length=4
**                 pos=1
**                 hoffset=3
**                 voffset=5
**                 item[0].delta=0
**                 item[0].chars=accentstring+1
**                 item[0].nchars=1
** maxaccentline:  2
** accentstring:   "^`"
*/
static void clear_accents(void)
{
    int i;
    accentpos=0;
    for (i=0; i<maxaccentline; i++) {
        accentline[i].pos=0;
        accentline[i].hoffset=0;
	accentline[i].voffset=0;
        accentline[i].length=0;
    }
    maxaccentline=0;
}

static int find_accent_line(int voffset)
{
    int i=0;
    /* find match */
    while (i<maxaccentline && accentline[i].voffset != voffset) i++;
    if (i!=maxaccentline) return i;
    if (maxaccentline==MAXOFFSET) {
	/* No positions left. Find closest line. */
	int best, delta,j;
	j=accentline[0].voffset-voffset; if (j<0) j= -j;
	best=0; delta=j;
	i=1;
	while (i<MAXOFFSET) {
	    j=accentline[i].voffset-voffset;
	    if (j<0) j= -j;
	    if (j<delta) { delta=j; best=i; }
	    i++;
	}
	return i;
    } else {
	/* make new accent line */
	maxaccentline++;
	accentline[0].voffset=voffset;
	return i;
    }	
}

static void add_accent(int voffset, Font fontnr, Uchar cpos,
                       int hoffset, int deltafor, int deltaafter)
{
    int acline;
    WordItem *it;
    acline=find_accent_line(voffset);
    if (!accentline[acline].pos) {
        accentline[acline].hoffset=hoffset+deltafor;
        accentline[acline].length=hoffset+deltafor;
        it=&accentline[acline].item[0];
    } else {
        it=&accentline[acline].item[accentline[acline].pos];
    }
    it->fontid=fontnr;
    it->chars=accentstring+accentpos;
    it->nchars=1;
    it->delta=hoffset+deltafor-accentline[acline].length;
    accentstring[accentpos].byte1= (cpos>>8)&0xff;
    accentstring[accentpos++].byte2= cpos&0xff;
    accentline[acline].pos++;
    accentline[acline].length=hoffset-deltaafter;
}

extern int draw_words(int x, int y, WordItem *txt, int len);

static void draw_accents(int x, int y)
{
    int i;
    for (i=0; i<maxaccentline; i++) {
	draw_words(x+accentline[i].hoffset, y-accentline[i].voffset,
		   accentline[i].item, accentline[i].pos);
    }
    clear_accents();
}

/* lastchar:   temporary variable for drawing purposes, information
**             in it might change.
** sptype:     combining type.
** spacingvar: information on the character to be used as a character
** deltafor:   horizontal delta before accent is placed
** deltaafter: horizontal delta after accent is placed
** deltavert:  vertical delta before accent is place
*/
static int calc_deltas(CharStruct *lastchar, unsigned char sptype,
                       CharStruct *spacingvar, 
                       int *deltafor, int *deltaafter, int *deltavert)
{
    int spwidth, lastw;
    int spsep=1;  /* number of pixels between character and accent */

    spwidth=CharWidth(spacingvar);
    lastw=CharWidth(lastchar);
    /* horizontal adjustment */
    switch (sptype) {
    case CCBelowLeftAttached:
    case CCAboveLeftAttached:
    case CCBelowLeft:
    case CCAboveLeft:
        *deltafor=-lastw;
        *deltaafter=lastw-spwidth;
        break;
    case CCBelowRightAttached:
    case CCAboveRightAttached:
    case CCBelowRight:
    case CCAboveRight:
        *deltafor=-spwidth;
        *deltaafter=0;
        break;
    case CCBelowAttached:
    case CCAboveAttached:
    case CCBelow:
    case CCAbove:
    case CCOverlay:
        *deltafor=(-lastw-spwidth)/2;
        *deltaafter=-((-lastw-spwidth)/2)-spwidth;
        break;
    case CCRightAttached:
    case CCRight:
        *deltafor=0;
        *deltaafter=0;
        break;
    case CCLeftAttached:
    case CCLeft:
        *deltafor=-lastw-spwidth;
        *deltaafter=lastw;
        break;
    default:
        *deltafor=-spwidth;
        *deltaafter=0;
        break;
    }
    /* vertical adjustment */
    switch (sptype) {
    case CCAboveLeftAttached:
    case CCAboveRightAttached:
    case CCAboveAttached:
        spsep=0;
    case CCAboveLeft:
    case CCAboveRight:
    case CCAbove:
        *deltavert=CharAscent(lastchar)+CharDescent(spacingvar)+spsep;
        CharAscent(lastchar)+=(spsep+CharAscent(spacingvar)+
			       CharDescent(spacingvar));
        break;
    case CCBelowRightAttached:
    case CCBelowLeftAttached:
    case CCBelowAttached:
        spsep=0;
    case CCBelowRight:
    case CCBelowLeft:
    case CCBelow:
        *deltavert= -spsep-CharDescent(lastchar)-CharAscent(spacingvar);
        CharDescent(lastchar)+=(spsep+CharAscent(spacingvar)+
			       CharDescent(spacingvar));
        break;
    case CCRightAttached:
    case CCRight:
    case CCLeftAttached:
    case CCLeft:
    case CCOverlay:
        /* center allign */
        *deltavert = (CharAscent(lastchar) -
		      (CharAscent(lastchar)+CharDescent(lastchar))/2 -
		      (CharAscent(spacingvar)-
		       (CharAscent(spacingvar)+CharDescent(spacingvar))/2));
        break;
    default:
        *deltavert=0;
        break;
    }
    return 1;
}

/* draw a string. If DRY is positive, do not actually draw the string, only
** calculate. (DRY indicates the x position where draw should stop.
** X and Y indicate the location where the string should be drawn.
** ( 0,0 if DRY is positive ). FONTATTRIB indicates the font to use.
** STRING contains the string.  POS is used to return to position in 
** STRING where drawing has stopped.  LEFT, RIGHT, ASCENT and DESCENT
** are used to return the geometry of the drawn word.
*/
static char debugbuf[0x10000];
/* If performance is more important than size, then store the constructed
** data (WordItem and placed accents) in the box for fast drawing.
*/
void draw_string(Uchar *string, short fontattrib,
		 int x, int y, int dry, int *detectpos, int *left,
		 int *right, int *ascent, int *descent)
{
    /* String might contain combining marks or characters with diacritics
    ** which have to be decomposed.  To add the diacritics to the characters,
    ** a list of lists is used, where each sublist contains the diacritics
    ** that need to be placed that the same location.
    */
    int asc=-0x7FFF,des=-0x7FFF,lef=0;
    int pos, itpos=0, stpos=0;
    Font f,lastf=0;
    int clinepos=0;
    int spacealt=0,dfor=0,dafter=0,dvert=0,sptype=0;
    Uchar *alt[15];  /* for decomposing composed strings */
    unsigned char altdectag[15]; /* decomposition tag, not used yet */
    int altp;
    int lastwidth;
    CharInfo *ci;
    CharStruct lastchar;

    drawitem[0].chars=0;
    drawitem[0].nchars=drawitem[0].delta=0;
    drawitem[0].fontid=None;
    clear_accents();
    lastwidth=0;
    altp=0;alt[0]=string;altdectag[0]=0;
    font_set_attributes(fontattrib);
    clinepos=0;
    /* string can not contain newlines or special markup, just characters and
    ** spaces in display order.
    */
    while (altp>=0) {
        if (!*alt[altp]) {
	    altp--;
	} else {
	    if (dry && !altp && clinepos>=dry) {
		/* detection and in main string and past detect position. */
		*detectpos=alt[0]-string;
		break;
	    }
	    /*
	    if (cursorpos>=0 && !altp && alt[0]-string==cursorpos) {
		cursorx=clinepos;
	    }
	    */
	    ci=character_info(*alt[altp]);
	    if (!ci) {
		/* No symbol found. Try to decompose */
		alt[altp+1]= Udecomp(*alt[altp]);
		if (!debugbuf[*alt[altp]]) {
		    fprintf(stderr, "? %04x : %s\n",
			    *alt[altp],(alt[altp+1]?"Dec":"???"));
		    debugbuf[*alt[altp]]++;
		}
		if (alt[altp+1]) {
		    altdectag[altp+1]=Udecomptype(*alt[altp]);
		    alt[altp]++;
		    altp++;
		} else {
		    /* No decomposition. Try spacing alternative */
		    if (Uiscombining(*alt[altp]) && Utospacing(*alt[altp])) {
			ci=character_info(Utospacing(*alt[altp]));
			if (debugbuf[*alt[altp]]<2) {
			    fprintf(stderr, "? %04x : %s\n",
				    *alt[altp], (ci?"Spacing":"NoSp"));
			    debugbuf[*alt[altp]]+=2;
			}
			/* spacing alternative possible */
			if (!ci) {
			    /* use default rendering for *alt[altp] */
			    ci=character_info(0xB1);
			} else {
			    spacealt=1;
			    sptype=Ucombclass(*alt[altp]);
			}
			alt[altp]++;
		    } else {
			/* use default rendering for *alt[altp] */
			ci=character_info(0xB1);
			alt[altp]++;
		    }
		}
	    } else {
		alt[altp]++;
	    }
	    if (ci) {
		f=ci->font;
		pos=ci->pos;
		if (spacealt) {
		    calc_deltas(&lastchar, sptype,
				(CharStruct*)(ci->sysinfo),
				&dfor, &dafter, &dvert);
		    add_accent(dvert, f, pos,
			       clinepos, dfor, dafter);
		    f=0; pos=0;
		    if (CharAscent(&lastchar)>asc) asc=CharAscent(&lastchar);
		    if (CharDescent(&lastchar)>des) des=CharDescent(&lastchar);
		} else {
		    lastchar= *(ci->sysinfo);
		    if (f!=lastf) {
			itpos++;
			drawitem[itpos]=drawitem[0];
			/* add italic correction if needed */
			drawitem[itpos].delta = 0;
			drawitem[itpos].chars=drawstring+stpos;
			drawitem[itpos].fontid=f;
			lastf=f;
		    }
		    drawstring[stpos].byte1= (pos>>8)&0xff;
		    drawstring[stpos++].byte2= pos&0xff;
		    drawitem[itpos].nchars++;
		    clinepos+=CharWidth(&lastchar);
		    if (dry) {
			if (CharAscent(&lastchar)>asc) {
			    asc=CharAscent(&lastchar);
			}
			if (CharDescent(&lastchar)>des) {
			    des=CharDescent(&lastchar);
			}
			if (alt[0]==string+1 && !altp &&
			    CharLeft(&lastchar) > lef) {
			    lef=CharLeft(&lastchar);
			}
		    }
		}
		spacealt=0;
	    }
	}
    }
    *left=lef;
    *ascent=asc;
    *descent=des;
    if (CharRight(&lastchar)>CharWidth(&lastchar)) {
	*right=clinepos + CharRight(&lastchar)-CharWidth(&lastchar);
    } else {
	*right=clinepos;
    }
    if (dry) {
	/* detect size or cursor position */
	/* If the cursor needs to be placed, set information
	** on where the cursor should appear (x,y, size)
	*/
    } else {
	draw_words(x,y,drawitem+1, itpos);
	draw_accents(x,y);
    }
    return;
}
