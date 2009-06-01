/*
**  Code to handle 16bit virtual font collections.
*/

#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "filefind.h"
#include "unifont.h"
#include "unimap.h"

#define DEBUG_MESSAGE printf

/* This array is used for returning properties about characters.
** The items are used in a cycle. */

#define INFOBUFMAX 16
static CharInfo infobuf[INFOBUFMAX];
static int infopos=0;

/* datastructures for reading files and storing information. */
/* a font config file has no particular layout, although the
** attribute definitions should come first.
** In a config file, you can define:
** *  which attributes are allowed and which values they have.
**    For N attributes, it spans up an N-dimentional structure for
**    virtual fonts.
** *  the encodings used for the fonts.
** *  ranges of characters to select certain characters from a
**    font/encoding.
** *  fonts with their attributes.  An automatic detection provides
**    default values for attributes, which can be changed by redefining
**    them.  (automatic detection depends on the XLFD name).
** *  alias for encodings, attributes, attribute values and ranges.
**    These aliases allow you to increase the performance of the
**    automatic detection.  If the attribute values are used to
**    create menus or messages, the values can be given in localised
**    versions while detection is still possible.
*/
#define MAXATTRIB 15


typedef struct REMAPREC REMAPREC;
struct REMAPREC {
  short attrgroup;
  short oldval;
  short newval;
  REMAPREC *next;
};

typedef struct RANGEREC RANGEREC;
struct RANGEREC {
    char *name;   /* name, shared with alias list */
    int *ranges;  /* list of pairs of integers, (start, end)*  */
    RANGEREC *next;
};

typedef struct ATTRIBREC ATTRIBREC;
struct ATTRIBREC {
    char *name; /* attribute name, shared with alias list */
    short num;  /* attribute group */
    short value; /* attribute value */
    ATTRIBREC *next;  /* next attrib definition */
};

typedef struct ENCODINGREC ENCODINGREC;
struct ENCODINGREC {
    char *mapfilename; /* name, shared with alias list */
    MapUchar encmap;
    ENCODINGREC *next;
};

typedef struct FONTREC FONTREC;
struct FONTREC {
    char *fontname;
    int attribpos;   /* calculated combination of attributes */
    int loaded;
    ENCODINGREC *encoding;
    RANGEREC *range;
    int fonttype;
    int bt,bm,bmax;
    unsigned long font; /* for X: Font */
    void *systemdata; /* for X: XFontStruct*  */
    FONTREC *next;
};

typedef struct ALIASREC ALIASREC;
struct ALIASREC {
    char *name;
    int aliastype;
    void *aliasdata;
    ALIASREC *next;
};

static struct {
    int def;
    int max;
    int multby;
    int divby;  /* multby/max */
} attribinfo[MAXATTRIB];
static int maxattrib=0;

/* A=attribute combination, G=group, V=value */
#define change_attribute(A,G,V) \
        ((((A)/attribinfo[G].multby)*attribinfo[G].multby)+ \
	((V)%attribinfo[G].max)*(attribinfo[G].divby)+ \
	(((A)%(attribinfo[G].divby))))
#define attribtovalue(A,G) \
    (((A)/attribinfo[G].divby)%attribinfo[G].max)

static FONTREC *fontlist=NULL;
static ATTRIBREC *attriblist=NULL;
static ENCODINGREC *encodinglist=NULL;
static RANGEREC *rangelist=NULL;
static ALIASREC *aliaslist=NULL;
static REMAPREC *remaplist=NULL;

/* put an array on top of fontlist for faster access. It is constructed
** after the fontlist is created.
*/
static FONTREC **fontarr=NULL;
static int fontarrmax=0;

static MapInt empty_charmap=0;

/*  def_charmap contains the list of available symbols and how they
**  can be printed. If a certain combination of attributes is not
**  valid (for instance bold italic sans-serif Japanees), this map is
**  used to get a default character, which may not have the correct
**  attributes, but the same position. It does not need
**  to be equal to any of the other character mappings.
*/
static MapInt def_charmap=0;

/* remaptable describes how a combination of attributes is changed
** when no characters are available for that combination.
*/
static int *remaptable=0;

/*  allchars contains a set of all available characters with there attributes
**  the fonts used for these characters may not be loaded. Which attributes
**  are valid and how they are combined is not fixed yet, since this is part
**  of a library.
*/
static MapInt *allchars=0;
static int allcharsize=0;

/* the map of the current attributes is stored for faster lookup */
static int current_attr=0;

static int defrange[4]={0,0xffff,-1,-1};

static PathInfo pmap=0;

static int build_remaptable(void)
{
  REMAPREC *rmp;
  int i;
  if (remaptable) free(remaptable);
  remaptable= malloc(sizeof(int)*allcharsize);
  for (i=0; i<allcharsize; i++) remaptable[i]= -1;
  rmp= remaplist;
  while (rmp) {
    int blksz,offset,size,diff,i,j;
    blksz=attribinfo[rmp->attrgroup].multby;
    size = attribinfo[rmp->attrgroup].divby;
    offset = size*rmp->oldval;
    diff = size*(rmp->newval-rmp->oldval);
    for (i=0; i<allcharsize; i=i+blksz) {
      for (j=i+offset; j<i+offset+size; j++) {
	remaptable[j] = j+diff;
      }
    }
    rmp=rmp->next;
  }
  return 0;
}

static int add_font_range(int fontnr, int attribpos, MapUchar encoding,
			  Uchar start, Uchar end)
{
    int i,j,k;
    MapInt change=NULL;
    j=attribpos;
    if (allchars[j]==empty_charmap) allchars[j]=MapIntCreate();
    change=allchars[j];
    /* optimisation:  skip empty submaps to reduce loops */
    for (i=start; i<=end; i++) {
	if (!(i%UNI_MAPSIZE) && EmptySubmap(encoding,i)) {
	    i=i+UNI_MAPSIZE-1;
	} else if ((j=MapValue(encoding, i))) {
            k=(fontnr<<16)+i;
            if (!MapValue(change,j)) MapIntDefine(change,j,k);
            if (!MapValue(def_charmap, j)) MapIntDefine(def_charmap, j, k);
        }
    }
    return 1;
}

static int build_font_structure(void)
{
    /* the lists may have changed (additions). Update the structures
    ** allchars, fontarr and def_charmap, in that order.
    */
    int i;
    FONTREC *frec;
    if (!empty_charmap) empty_charmap = MapIntCreate();
    if (!def_charmap) def_charmap=MapIntCreate();
    i=(maxattrib>0 ? attribinfo[maxattrib-1].multby:1);
    if (allcharsize < i) {
	MapInt *nall;
	if (allchars) nall=realloc(allchars,sizeof(MapInt)*i);
	else nall=malloc(sizeof(MapInt)*i);
	while (allcharsize<i) nall[allcharsize++]=empty_charmap;
	allchars=nall;
    }
    frec=fontlist;
    i=0;
    while (frec) { if (frec->encoding) i++; frec=frec->next; };
    if (i!=fontarrmax) {
	/* number of fonts changed. */
	/* expand font array */
	FONTREC **farr;
	if (fontarr) farr=realloc(fontarr,sizeof(FONTREC*)*i);
	else farr=malloc(sizeof(FONTREC*)*i);
	frec=fontlist;
	i=0;
	while (frec) {
	    if (frec->encoding) {
		farr[i++]=frec;
	    } else {
		fprintf(stderr, "Skipped %s\n", frec->fontname);
	    }
	    frec=frec->next;
	}
	/* add all fonts after fontarrmax */
	while (fontarrmax<i) {
	    int *range;
	    if (!farr[fontarrmax]->encoding->encmap) {
		FILE *f;
		int j;
		char *buffer;
		f=open_file(pmap,
			    farr[fontarrmax]->encoding->mapfilename,
			    "rb");
		if (f) {
		    j=file_size(pmap,farr[fontarrmax]->encoding->mapfilename);
		    buffer=malloc(j);
		    j=fread(buffer,1, j, f);
		    close_file(f);
		    MapUcharLoad(farr[fontarrmax]->encoding->encmap,buffer);
		}
		/* load encoding file */
	    }
	    if (!farr[fontarrmax]->range) {
		range=defrange;
	    } else {
		range=farr[fontarrmax]->range->ranges;
	    }
	    while (*range>=0) {
		add_font_range(fontarrmax, farr[fontarrmax]->attribpos,
			       farr[fontarrmax]->encoding->encmap,
			       range[0],range[1]);
		range=range+2;
	    }
	    fontarrmax++;
	}
	fontarr=farr;
    }
    build_remaptable();
    return 1;
}

/* alias types: */
#define ENCODINGALIAS 1
#define RANGEALIAS 2
#define ATTRIBGROUPALIAS 4
/* n is the group number */
#define ATTRIBVALUEALIAS(n) (((n)+1)*8)

static char *add_alias(char *name, int aliastype, void *data)
{
    ALIASREC *n;
    n=malloc(sizeof(ALIASREC));
    n->next=aliaslist;
    n->name=malloc(sizeof(char)*(strlen(name)+1));
    strcpy(n->name,name);
    n->aliastype=aliastype;
    n->aliasdata=data;
    aliaslist=n;
    return n->name;
}

static void *find_alias(char *name, int aliastype)
{
    ALIASREC *n=aliaslist;
    while (n && (n->aliastype!=aliastype || strcasecmp(n->name,name))) {
	n=n->next;
    }
    return (n ? n->aliasdata : NULL);
}
static ALIASREC *lastfind=NULL;

static void *find_aliasname(char *name, int *aliastype)
{
    ALIASREC *n=aliaslist;
    while (n && strcasecmp(name,n->name)) n=n->next;
    lastfind=n;
    if (n) {
	*aliastype=n->aliastype;
	return n->aliasdata;
    } else {
	*aliastype=0;
	return NULL;
    }
}
static void *find_nextname(int *aliastype)
{
    ALIASREC *n;
    n=(lastfind?lastfind->next:NULL);
    while (n && strcasecmp(lastfind->name,n->name)) n=n->next;
    lastfind=n;
    if (n) {
	*aliastype=n->aliastype;
	return n->aliasdata;
    } else {
	*aliastype=0;
	return NULL;
    }
}

static char *find_type(int aliastype)
{
    ALIASREC *n=aliaslist;
    while (n && n->aliastype!=aliastype) n=n->next;
    lastfind=n;
    return (n?n->name:NULL);
}

static char *find_type_next(void)
{
    ALIASREC *n;
    n=(lastfind?lastfind->next:NULL);
    while (n && n->aliastype!=lastfind->aliastype) n=n->next;
    lastfind=n;
    return (n?n->name:NULL);
}

#define skip_spaces(A)  while (*(A) && isspace(*(A))) (A)++
#define find_spaces(A)  while (*(A) && !isspace(*(A))) (A)++

static char *get_string(char **c, char *bufres)
{
    char *res,*w;
    w=*c;
    *w=*bufres;
    skip_spaces(w);
    if (*w=='"') {
	w++;
	res=w;
	while (*w && *w!='"') w++;
	if (*w=='"') *w=' ';
    } else if (*w) {
	res=w;
	find_spaces(w);
    } else {
	res=NULL;
    }
    *bufres=*w;
    *w='\0';
    *c=w;
    return res;
}

static char *get_integer(char *c, int *result)
{
    int i;
    int base;
    char xmax;
    switch (*c) {
    case '0':
	if (toupper(c[1])=='X') {
	    c=c+2;
	    base=16;
	    xmax='G';
	} else {
	    base=8;
	    xmax='8';
	}
	break;
    case 'U':
	c++;
	if (*c=='+') c++;
	base=16;
	xmax='G';
	break;
    case '\'':
	c++;
	*result=(unsigned char) *c;
	c++;
	if (*c=='\'') c++;
	return c;
	break;
    default:
	base=10;
	xmax='9'+1;
	break;
    }
    i=0;
    while (isxdigit(*c) && toupper(*c)<xmax) {
	if (isalpha(*c)) {
	    i=i*base+10+(toupper(*c)-'A');
	} else {
	    i=i*base+(*c-'0');
	}
	c++;
    }
    *result=i;
    return c;
}

/* system specific functions to load, inspect or test fonts */
#include "unifontsys.c"

int font_load_config(char *confname)
{
    FILE *f;
    FONTREC **flist=&fontlist;
    ATTRIBREC **alist=&attriblist;
    ENCODINGREC **elist=&encodinglist;
    RANGEREC **rlist=&rangelist;
    REMAPREC **rmlist=&remaplist;
    FONTREC *frec;
    char buffer[512];
    char *bpos;
    int linecount=0;
    if (!pmap) {
	pmap=make_pathinfo("MAPPATH",DEFAULTMAPPATH,".map,.ufont");
    }
    f=open_file(pmap, confname, "r");
    if (!f) {
	fprintf(stderr, "Unable to find font configuration '%s'\n", confname);
	return 0;
    }
    /* find the end of these lists */
    while (*flist) flist=&(*flist)->next;
    while (*alist) {
	if ((*alist)->value==-1) maxattrib++;
	alist=&(*alist)->next;
    }
    while (*elist) elist=&(*elist)->next;
    while (*rlist) rlist=&(*rlist)->next;
    while (*rmlist) rmlist=&(*rmlist)->next;
    frec=NULL;
    /* read in the file, line by line */
    /* a buffer of 511 character should suffice */
    while (fgets(buffer, 511, f)) {
	linecount++;
	bpos=buffer;
	skip_spaces(bpos);
	/* skip empty line or comments (starting with #) */
	if (!*bpos || *bpos=='#') continue;
	if (!strncmp(bpos,"ATTRIBUTE", 9)) {
	    /* define a new attribute:
	    ** ATTRIBUTE name value0 value1 value2 value3
	    ** value? is a string used as a reference
	    ** name is the name used to define the attribute
	    ** (so, a line of the form 'name value1' defines the attribute
	    **  name to be equal to value1 for the current font)
	    **
	    ** The font attribute detection algorithm uses these values
	    ** to find default attributes for fonts from their name.
	    */
	    char *c,*h;
	    char tc;
	    int i= -1;
	    c=bpos+9;
	    tc=*c;
	    h=get_string(&c,&tc);
	    while (h) {
		*alist=malloc(sizeof(ATTRIBREC));
		(*alist)->name=
		add_alias(h,
			  ((i<0)?ATTRIBGROUPALIAS:ATTRIBVALUEALIAS(maxattrib)),
			  *alist);
		(*alist)->value=i;
		(*alist)->num=maxattrib;
		h=get_string(&c,&tc);
		i++;
		(*alist)->next=NULL;
		alist=&(*alist)->next;
	    }
	    /* at least one (perhaps two?) value should be specified */
	    if (i>=0) {
		if (!i) {
                    /* no values given */
		    /* h still points to name of the attribute */
		    fprintf(stderr,
			    "%s:%i: No values given for attribute %s\n"
			    "%s:%i: Assuming attribute is not used\n",
			    confname, linecount, h,confname,linecount);
		    i=1;
		}
		attribinfo[maxattrib].max=i;
		if (maxattrib) i=i*attribinfo[maxattrib-1].multby;
		attribinfo[maxattrib].multby=i;
		attribinfo[maxattrib].divby=i/attribinfo[maxattrib].max;
		attribinfo[maxattrib].def=0;
		maxattrib++;
	    } else if (i<0) {
		fprintf(stderr, "%s:%i: No attribute name given\n",
			confname, linecount);
	    }
	} else if (!strncmp(bpos, "ATTRALIAS", 9)) {
	    /* Redefine the attributes for a certain group. These
	    ** attributes might be used in menus and you can adjust
	    ** the menus by changing the attribute names.  Useful
	    ** for localization, for example:
	    ** ATTRIBUTE Weight Medium Bold
	    ** ...
	    ** ATTRALIAS Weight Gewicht Normaal Vet
	    */
	    char *c,*h;
	    char tc;
	    int aliastype;
	    void *aliasdata;
	    ATTRIBREC *ar;
	    c=bpos+9;
	    tc=*c;
	    h=get_string(&c,&tc);
	    aliasdata=find_alias(h,ATTRIBGROUPALIAS);
	    if (!aliasdata) {
		fprintf(stderr, "%s:%i: Unknow attribute name %s\n",
			confname, linecount, h);
		continue;
	    }
	    ar=aliasdata;
	    aliastype=ar->num;
	    h=get_string(&c,&tc);
	    while (ar && (ar->num==aliastype) && h) {
		ar->name=add_alias(h,(ar->value>=0?
				      ATTRIBVALUEALIAS(aliastype):
				      ATTRIBGROUPALIAS),
				   ar);
		ar=ar->next;
		h=get_string(&c,&tc);
	    }
	    if (h) {
		fprintf(stderr, "%s:%i: Too many attributes given\n.",
			confname, linecount);
	    }
	    if (ar && (ar->num==aliastype)) {
		fprintf(stderr, "%s:%i: Not enough values given\n",
			confname, linecount);
	    }
	} else if (!strncmp(bpos, "ATTRREMAP", 9)) {
	    /* Add an entry for remapping an attribute within a group */
	    char *c, *h;
	    char tc;
	    int aliastype;
	    void *aliasdata;
	    ATTRIBREC *ar;
	    ATTRIBREC *oav, *nav;
	    c = bpos+9;
	    tc = *c;
	    h=get_string(&c,&tc);
	    aliasdata=find_alias(h, ATTRIBGROUPALIAS);
	    if (!aliasdata) {
	      fprintf(stderr, "%s:%i: Unknown attribute name %s\n",
		      confname, linecount, h);
	      continue;
	    }
	    ar = aliasdata;
	    aliastype = ar->num;
	    h=get_string(&c, &tc);
	    oav=NULL; nav=NULL;
	    if (h) {
	      oav = find_alias(h, ATTRIBVALUEALIAS(aliastype));
	    }
	    if (oav) {
	      h=get_string(&c, &tc);
	      if (h) {
		nav = find_alias(h, ATTRIBVALUEALIAS(aliastype));
		if (!nav) {
		  fprintf(stderr,
			  "%s:%i: Unknown attribute value %s for group %s\n",
			  confname, linecount, h, ar->name);
		}
	      }
	    } else {
	      fprintf(stderr,
		      "%s:%i: Unknown attribute value %s for group %s\n",
		      confname, linecount, h, ar->name);
	    }
	    if (ar && oav && nav) {
	      *rmlist = malloc(sizeof(REMAPREC));
	      (*rmlist)->attrgroup = aliastype;
	      (*rmlist)->oldval = oav->value;
	      (*rmlist)->newval = nav->value;
	      (*rmlist)->next =0;
	      rmlist = &((*rmlist)->next);
	    }
	} else if (!strncmp(bpos, "ALIAS",5)) {
	    /* Define an alias for some identifier. */
	    char *c,*h;
	    char tc;
	    int aliastype;
	    void *aliasdata;
	    c=bpos+5;
	    tc=*c;
	    h=get_string(&c,&tc);
	    aliasdata=find_aliasname(h,&aliastype);
	    if (!aliasdata) {
		fprintf(stderr, "%s:%i: identifier %s unknown\n",
			confname, linecount, h);
		continue;
	    }
	    h=get_string(&c,&tc);
	    while (h) {
		/* check if h is already defined for that type */
		if (!find_alias(h,aliastype)) {
		    add_alias(h,aliastype, aliasdata);
		}
		h=get_string(&c,&tc);
	    }
	} else if (!strncmp(bpos, "RANGE", 5)) {
	    /* define a range.
	    ** syntax: RANGE name n k-l s-t
	    ** n,k,l,s and t are integers in one of the following formats:
	    ** decimal 65, octal 0101, hexadecimal 0x41, character 'A',
	    ** Unicode U+0041.  Sorry, no i18n for numbers yet.
	    ** The integers indicate the positions in the encoding of
	    ** the font, not in the Unicode encoding.
	    */
	    char *c, *h;
	    char tc;
	    RANGEREC *rr;
	    int len, actnum;
	    c=bpos+5;
	    tc=*c;
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr, "%s:%i: No name specified\n",
			confname, linecount);
		continue;
	    }
	    (*rlist)=rr=malloc(sizeof(RANGEREC));
	    rr->name=add_alias(h,RANGEALIAS, rr);
	    rr->next=NULL;
	    rlist=&rr->next;
	    *c=tc;
	    /* upper limit to the length of the needed array is the length
	    ** of the string, since each number takes at least two positions
	    ** Add 2 for a termination pair.
	    */
	    skip_spaces(c);
	    len=strlen(c);
	    actnum=0;
	    rr->ranges=malloc(sizeof(int)*(len+2));
	    while (*c && actnum+1<len) {
		h=c;
		c=get_integer(c, rr->ranges+actnum);
		actnum++;
		if (*c=='-') {
		    c++;
		    c=get_integer(c,rr->ranges+actnum);
		} else {
		    rr->ranges[actnum]=rr->ranges[actnum-1];
		}
		actnum++;
		if (!isspace(*c)) {
		    find_spaces(c);
		    tc=*c;
		    *c=0;
		    fprintf(stderr, "%s:%i: strange range item %s\n",
			    confname, linecount, h);
		    *c=tc;
		}
		skip_spaces(c);
	    }
	    rr->ranges[actnum++]= -1;
	} else if (!strncmp(bpos, "ENCODING", 8)) {
	    /* Define an encoding for a font.
	    ** Multiple encoding can occur on the same line.
	    ** The encoding is only loaded when it is actually used.
	    */
	    char *c,*h;
	    char tc;
	    c=bpos+8;
	    tc=*c;
	    h=get_string(&c,&tc);
	    while (h) {
		*elist=malloc(sizeof(ENCODINGREC));
		(*elist)->encmap=0;
		(*elist)->mapfilename=add_alias(h, ENCODINGALIAS, *elist);
		(*elist)->next=0;
		elist=&(*elist)->next;
		h=get_string(&c,&tc);
	    }
	} else if (!strncmp(bpos, "FONTENC",7)) {
	    /* Define the encoding of the current font.
	    ** The name of the encoding has to be defined in an
	    ** "ENCODING" line, or as an alias for an encoding.
	    */
	    char *c, *h;
	    char tc;
	    ENCODINGREC *er;
	    if (!frec) {
		fprintf(stderr, "%s:%i: Not defining a font yet\n",
			confname,linecount);
		continue;
	    }
	    c=bpos+7;
	    tc=*c;
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr, "%s:%i: No encoding specified\n",
			confname, linecount);
		continue;
	    }
	    er=find_alias(h, ENCODINGALIAS);
	    if (!er) {
		fprintf(stderr, "%s:%i: Unknown encoding %s\n",
			confname, linecount, h);
	    } else {
		frec->encoding=er;
	    }
	} else if (!strncmp(bpos, "FONTRANGE",9)) {
	    /* Define the range of a font. Certain fonts might be too
	    ** large or contain too many bad characters.  The range
	    ** is used to select certain characters from a font.
	    ** It will be used when the virtual font is defined and
	    ** when the font is loaded.
	    */
	    char *c, *h;
	    char tc;
	    RANGEREC *rr;
	    if (!frec) {
		fprintf(stderr, "%s:%i: Not defining a font yet\n",
			confname,linecount);
		continue;
	    }
	    c=bpos+9;
	    tc=*c;
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr, "%s:%i: No range specified\n",
			confname, linecount);
		continue;
	    }
	    rr=find_alias(h, RANGEALIAS);
	    if (!rr) {
		fprintf(stderr, "%s:%i: Unknown range %s\n",
			confname, linecount, h);
	    } else {
		frec->range=rr;
	    }
	} else if (!strncmp(bpos, "FONT",4)) {
	    /* Start defining a new font with auto-detection of attributes
	    ** and encoding. The auto-detection should not load the font
	    ** itself, but rather use the name of the font to detect
	    ** attributes: matching alias names, using a database.
	    */
	    char *c,*h;
	    char tc;
	    FONTREC *reslist;

	    c=bpos+4;
	    tc=*c;
	    h=get_string(&c,&tc);
	    frec = malloc(sizeof(FONTREC));
	    frec->fontname=malloc(sizeof(char)*(c-h+1)); /* c-h==strlen(h) */
	    strcpy(frec->fontname, h);
	    frec->attribpos=0;
	    frec->encoding=NULL;
	    frec->range=NULL;
	    frec->fonttype=frec->loaded=frec->bt=frec->bm=frec->bmax=0;
	    frec->font=0;
	    frec->systemdata=NULL;
	    frec->next=NULL;
	    /* system specific attribute detection */
	    reslist = multi_detect_attributes(frec);
	    /* Could check if reslist empty. However, it would indicate
	    ** that the font is not available.  Just ignore it or complain.
	    ** If the font would be added anyhow, it could result in messages
	    ** when the font has to be loaded.
	    */
	    if (reslist) {
	      free(frec->fontname);
	      free(frec);
	      *flist = reslist;
	      frec = reslist;
	      while (*flist) flist = &((*flist)->next);
	    } else {
	      free(frec->fontname);
	      free(frec);
	    }
	} else {
	    /* The line starts the name of an attribute followed by
	    ** its value. Find the attribute and the value and change
	    ** the attribute of the current font.
	    */
	    char *c,*h;
	    char tc;
	    ATTRIBREC *ar;
	    ATTRIBREC *aval;
	    int agr;
	    c=bpos;
	    tc=*c;
	    /* get attribute name and look it up */
	    h=get_string(&c,&tc);
	    ar=find_alias(h,ATTRIBGROUPALIAS);
	    if (!ar) {
		fprintf(stderr,"%s:%i: Unknow attribute group %s\n",
			confname, linecount, h);
		continue;
	    }
	    agr=ar->num;
	    /* get attribute value and look it up */
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr,"%s:%i: No attribute value specified.\n",
			confname, linecount);
		aval=NULL;
	    } else {
		aval=find_alias(h,ATTRIBVALUEALIAS(agr));
	    }
	    if (!aval) {
		/* Invalid attribute value.
		** Print valid attribute values as feedback.
		*/
		char *vname;
		fprintf(stderr,"%s:%i: Invalid attribute %s\n",
			confname, linecount, h);
		fprintf(stderr,"%s:%i: use one of:", confname, linecount);
		vname=find_type(ATTRIBVALUEALIAS(agr));
		while (vname) {
		    fprintf(stderr, " %s", vname);
		    vname=find_type_next();
		}
		fprintf(stderr, "\n");
		continue;
	    } else if (frec) {
		/* change font attribute of current collection of fonts */
	        FONTREC *flist;
		flist = frec;
		while (flist) {
		  flist->attribpos=change_attribute(flist->attribpos,
						    aval->num,
						    aval->value);
		  flist=flist->next;
		}
	    } else {
		fprintf(stderr, "%s:%i: Not defining a font yet or previous font not found.\n",
			confname, linecount);
	    }
	}
    }
    close_file(f);
    return build_font_structure();
}

/* set attribute ATTRNR to VALUE */
void font_set_attribute(int attrnr, int value)
{
    if (attrnr<0 || value<0 || attrnr>=maxattrib ||
	value >= attribinfo[attrnr].max) return;
    current_attr=change_attribute(current_attr,attrnr,value);
}

void font_set_attributes(int attribcombo)
{
    if (attribcombo>=0 && attribcombo<attribinfo[maxattrib-1].multby) {
	current_attr=attribcombo;
    }
}

int font_get_attributes(void)
{
    return current_attr;
}

/* get current value of attribute ATTRNR */
int font_get_attribute(int attrnr)
{
    if (attrnr<0 || attrnr>=maxattrib) return 0;
    else return attribtovalue(current_attr,attrnr);
}

/* change one attribute in the given font */
int font_change_attribute(int attribcombo,  int attrnr, int value)
{
  if (attrnr<0 || value<0 || attrnr>=maxattrib ||
      value >=attribinfo[attrnr].max) return attribcombo;
  return change_attribute(attribcombo, attrnr, value);
}

/* get the attribute name of the some attribute */
char *font_get_name(int attrnr, int attrval)
{
    ATTRIBREC *ar;
    ar=attriblist;
    while (ar && (ar->num != attrnr || ar->value !=attrval)) ar=ar->next;
    if (ar) return ar->name;
    else return 0;
}

/* Currently, a default character table is used to remap all attributes
** at once. A better result would be achieved if the remapping uses a
** table to remap attributes, like LaTeX does. For example, an order
** on the available attribute groups creates an order on the
** possible attribute combinations:  replace the least important attribute
** by its default value and check if that glyph is available. Repeat this
** until a character is found. The remapping can be stored in a table,
** which can be replaced easily by a different table.
** (Attribute combinations are stored as integers, the table would just
**  remap each integer to a different integer until a fixed point is
**  reached.)
*/
CharInfo *default_character_info(Uchar c)
{
    int j,k;
    int ca,m;
    ca = remaptable[current_attr];

    k=j=m=0;
    while (ca!= -1 && m<10) {
      k=MapValue(allchars[ca], c);
      if (k) {
	j=k>>16;
	if (fontarr[j]->loaded==FONT_NOT_LOADED) {
	  fontarr[j]->loaded= load_system_font(fontarr[j]);
	}
	if (fontarr[j]->loaded!=FONT_SUCCESS) {
	  MapValue(allchars[ca],c)=0;
	  k=0;
	}
      }
      if (!k) {
	ca = remaptable[ca];
	m++;
      } else {
	break;
      }
    }
    if (!k) {
      k=MapValue(def_charmap, c);
      if (k) {
	j=k>>16;
	if (fontarr[j]->loaded==FONT_NOT_LOADED) {
	  fontarr[j]->loaded = load_system_font(fontarr[j]);
	}
	if (fontarr[j]->loaded!=FONT_SUCCESS) {
	  MapValue(def_charmap,c)=0;
	  k=0;
	}
      }
    }
    if (!k) return NULL;
    infopos=(infopos+1)%INFOBUFMAX;
    return info_system_char(fontarr[j],k&0xffff,infobuf+infopos);
}

#ifdef NO_VIRT_AUTO_DEFAULT
#define default_remap_info(A) NULL
#else
#define default_remap_info(A) default_character_info(A)
#endif

/* this function will probably be used very often, so it might need
** to be optimised. Some hints:
** -  adjust the map of allchars[current_attr]
**    by defining it as the character that is found
**    at the end.
** -  replace >>16 and &0xFFFF by short pointer references,
**    which might be endian-dependent.
** -  define a better .ufont file for constructing the virtual font
** -  add load/save support for the complete virtual font, with an
**    option to resolve all missing characters according to some
**    algorithm before saving it.  (such a file would be very
**    large and should contain information about fonts and attribute
**    names.)
**
** Remapping attributes could be difficult.  A character with diacritics
** can be constructed from the base character by adding the diacritics.
** Doing the remapping here, would make such a construction difficult,
** unless the remapping is turned off for decomposable characters.
** So, remapping of attributes should be done at a higher level.
** An example: a Japanese font might contain a character that is missing
** from the Latin Extended part. Using that character, which has different
** attributes, results in a bad layout, while combining characters with
** the same attributes could usually result in better layout.
**
*/
CharInfo *character_info(Uchar c)
{
    int j=0,k,ca=current_attr;

    k=MapValue(allchars[ca], c);
    if (k) {
	j=k>>16;
	if (fontarr[j]->loaded==FONT_NOT_LOADED) {
	    fontarr[j]->loaded = load_system_font(fontarr[j]);
	}
	if (fontarr[j]->loaded!=FONT_SUCCESS) {
	    MapValue(allchars[ca],c)=0;
	    k=0;
	}
    }
    if (!k) return default_remap_info(c);
    infopos=(infopos+1)%INFOBUFMAX;
    return info_system_char(fontarr[j],k&0xffff,infobuf+infopos);
}

void font_set_system_data(void *data)
{
  font_system_data(data);
}

/*  A collection of virtual fonts can saved to disk and loaded if
**  the algorithm to add fonts is deterministic.  The disk version
**  would just save all the maps, together with some index.
**  For optimisation, the save algorithm could resolve (some) missing
**  symbols where needed.
**  It will be rather difficult to merge to virtual fonts.
*/

