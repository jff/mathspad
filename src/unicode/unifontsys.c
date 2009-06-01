#include <X11/Xlib.h>

static Display *display=0;
static int enabled_range=0;

#include "unifontX11.h"

/* detect the attributes for a given font. The name of the font is
** available
*/
static void detect_attributes(FONTREC *frec)
{
    /* detect attributes:  !!! X11 specific code
    ** * Divide the font name in parts between '-' signs
    ** * For each part, check if it is an alias
    ** * If the encoding is not set at the end, combine the
    **   last two parts and check if that is an alias.
    **
    ** Keep track which attributes are already set to make sure
    ** no redefinitions are made.
    */
    char *c,*h;
    int attrset[MAXATTRIB];
    char *t;
    void *aliasdata;
    int aliastype;
    int tosetleft;
    int i;
    if (frec->fontname[0]!='-') return;
    for (i=0; i<maxattrib; i++) attrset[i]=0;
    tosetleft=maxattrib+2; /* range and encoding */
    h=t=frec->fontname;
    while (t && tosetleft) {
	*t='-';
	t++;
	c=t;
	t=strchr(t,'-');
	if (t) *t='\0';
	/* skip fields of the form "*" , "" , "normal"
	** "normal" is skipped because it does not specify any features
	** of the given font and disables correct size detection if
	** normal is also specified as a size attribute
	*/
	if (*c && *c!='*' && strcasecmp(c, "normal")) {
	    aliasdata=find_aliasname(c,&aliastype);
	    while (aliasdata) {
		switch (aliastype) {
		case RANGEALIAS:
		    if (!frec->range) {
			frec->range=aliasdata;
			tosetleft--;
		    }
		    break;
		case ENCODINGALIAS:
		    if (!frec->encoding) {
			frec->encoding=aliasdata;
			tosetleft--;
		    }
		    break;
		case ATTRIBGROUPALIAS:
		    break;
		default:
		    /* ATTRIBVALUEALIAS(n) */
		    for (i=0; i<maxattrib; i++) {
			if (!attrset[i] &&
			    aliastype==ATTRIBVALUEALIAS(i)) {
			    ATTRIBREC *ar=aliasdata;
			    attrset[i]=1;
			    tosetleft--;
			    frec->attribpos =
                                change_attribute(frec->attribpos,
						 ar->num, ar->value);
			    continue;
			}
		    }
		    break;
		}
		aliasdata=find_nextname(&aliastype);
	    }
	}
    }
    if (!frec->encoding) {
	/* try to find encoding in last two items
	** c is after a last minus. Move one '-' back.
	*/
	c--;c--;
	while (c>h && *c!='-') c--;
	if (c!=h) {
	    c++;
	    frec->encoding=find_alias(c,ENCODINGALIAS);
	}
    }
}

/* Detect multiple fonts from one font name (which used * fields).
** It returns a list of matching fonts.
** If there is no server connection or the font name does not match
** any available fonts, then the name is ignored and an empty list
** is returned.
*/
static FONTREC *multi_detect_attributes(FONTREC *frec)
{
  FONTREC *result=0;
  int i,n;
  char **fontnamelist;

  if (!display) return 0;
  fontnamelist = XListFonts(display, frec->fontname, 4096, &n);
  if (!fontnamelist) return 0;
  for (i=0; i<n; i++) {
    FONTREC *newfrec;
    if (strstr(fontnamelist[i], "-0-0-")) continue;
    newfrec = malloc(sizeof(FONTREC));
    *newfrec = *frec;
    newfrec->fontname = malloc(sizeof(char)*(strlen(fontnamelist[i])+1));
    strcpy(newfrec->fontname, fontnamelist[i]);
    newfrec->next = result;
    result = newfrec;
    detect_attributes(newfrec);
  }
  XFreeFontNames(fontnamelist);
  return result;
}

/* load the font in the window system, using the information available
** in FREC (attributes/range/name/encoding). Return FONT_SUCCESS if
** the function succeeded. Return FONT_ERROR if it failed.
*/
static char rangebuffer[1024];
static int load_system_font(FONTREC *frec)
{
    XFontStruct *fs;
    char *name;
    if (frec->range && enabled_range) {
	/* add range to fontname */
	int bpos;
	int *ilist;
	ilist=frec->range->ranges;
	strcpy(rangebuffer,frec->fontname);
	bpos=strlen(frec->fontname);
	rangebuffer[bpos++]='[';
	while (*ilist>=0 && bpos<1000) {
	    if (ilist[0]==ilist[1]) {
		sprintf(rangebuffer+bpos," %i", ilist[0]);
	    } else {
		sprintf(rangebuffer+bpos," %i~%i", ilist[0],ilist[1]);
	    }
	    bpos= bpos+strlen(rangebuffer+bpos);
	    ilist=ilist+2;
	}
	rangebuffer[bpos++]=']';
	rangebuffer[bpos]='\0';
	name=rangebuffer;
    } else {
	name=frec->fontname;
    }
    frec->systemdata= fs = XLoadQueryFont(display, name);
    if (fs) {
        frec->font = fs->fid;
        frec->fonttype=0;
        if (fs->per_char) {
            frec->fonttype=1;
            frec->bmax=0;
        }
        if (fs->min_byte1 || fs->max_byte1) {
            frec->fonttype+=2;
            frec->bt=fs->max_char_or_byte2-fs->min_char_or_byte2+1;
            frec->bm=
                frec->bt*fs->min_byte1+fs->min_char_or_byte2;
            frec->bmax=
                frec->bt*(fs->max_byte1-fs->min_byte1+1);
        } else {
            frec->bt=0;
            frec->bm = fs->min_char_or_byte2;
            frec->bmax = fs->max_char_or_byte2-fs->min_char_or_byte2+1;
        }
#ifdef DEBUG
        fprintf(stderr, "Successfully load font '%s',\n"
                "which is %s and %s-byte.\n",
                name,
                (frec->fonttype&1?"proportional":"fixed-width"),
                (frec->fonttype&2?"double":"single"));
#endif
        return FONT_SUCCESS;
    } else {
        fprintf(stderr, "Unable to load font '%s'.\n",
                name);
        return FONT_ERROR;
    }
}

/* return RESULT if information is available on position POS in
** font FREC. Otherwise, return NULL.
*/
static CharInfo *info_system_char(FONTREC *frec, int pos, CharInfo *result)
{
    int i;
    XFontStruct *fs;
    fs=frec->systemdata;
    result->font=frec->font;result->pos=pos;
    switch (frec->fonttype) {
    case 0: case 2: result->sysinfo = &(fs->max_bounds); break;
    case 1:
        i= pos-frec->bm;
        if (i>=0 && i<=frec->bmax)
            result->sysinfo = fs->per_char+i;
        else return NULL;
        break;
    case 3:
        i= (pos>>8)*frec->bt+(pos&0xff)-frec->bm;
        if (i>=0 && i<=frec->bmax)
            result->sysinfo = fs->per_char+i;
        else return NULL;
        break;
    }
    return result;
}

static void font_system_data(void *data)
{
  display = (Display*) data;
  if (display) {
    /* check whether range selection is supported by the X server */
    XFontStruct *fs;
    fs = XLoadQueryFont(display, "-*-courier-*-iso8859-*[82 86]");
    if (!fs) {
      enabled_range=0;
    } else {
      enabled_range=1;
      XFreeFont(display, fs);
    }
  }
}
