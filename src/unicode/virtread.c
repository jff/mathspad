#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <string.h>
#include "filefind.h"
#include "unicode.h"
#include "virtread.h"


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

typedef struct RANGEREC RANGEREC;
struct RANGEREC {
    char *name;   /* name, shared with alias list */
    int *ranges;  /* list of pairs of integers, (start, end)*  */
    RANGEREC *next;
};

typedef struct ATTRIBREC ATTRIBREC;
struct ATTRIBREC {
    char *name; /* attribute name, shared with alias list */
    char num;  /* attribute group */
    char value; /* attribute value */
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
    ENCODINGREC *encoding;
    RANGEREC *range;
    int fonttype;
    int bt,bm,bmax;
    Font font;
    XFontStruct *fstruct;
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
static int lastattrib=0;

/* A=attribute combination, G=group, V=value */
#define change_attribute(A,G,V) \
        ((((A)/attribinfo[(int)G].multby)*attribinfo[(int)G].multby)+ \
	((V)%attribinfo[(int)G].max)*(attribinfo[(int)G].divby)+ \
	(((A)%(attribinfo[(int)G].divby))))
#define attribtovalue(A,G) \
    (((A)/attribinfo[(int)G].divby)%attribinfo[(int)G].max)

static FONTREC *fontlist=NULL;
static ATTRIBREC *attriblist=NULL;
static ENCODINGREC *encodinglist=NULL;
static RANGEREC *rangelist=NULL;
static ALIASREC *aliaslist=NULL;

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
    while (n && (n->aliastype!=aliastype || strcmp(n->name,name))) {
	n=n->next;
    }
    return (n ? n->aliasdata : NULL);
}
static ALIASREC *lastfind=NULL;

static void *find_aliasname(char *name, int *aliastype)
{
    ALIASREC *n=aliaslist;
    while (n && strcmp(name,n->name)) n=n->next;
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
    while (n && strcmp(lastfind->name,n->name)) n=n->next;
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

/* put an array on top of fontlist for faster access. It is constructed
** after the fontlist is created.
*/

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

int load_font_config(PathInfo pmap, char *confname)
{
    FILE *f;
    FONTREC **flist=&fontlist;
    ATTRIBREC **alist=&attriblist;
    ENCODINGREC **elist=&encodinglist;
    RANGEREC **rlist=&rangelist;
    FONTREC *frec;
    char buffer[512];
    char *bpos;
    int linecount=0;
    f=open_file(pmap, confname, "r");
    if (!f) {
	fprintf(stderr, "Unable to load font configuration\n");
	return 0;
    }
    while (*flist) flist=&(*flist)->next;
    while (*alist) {
	if ((*alist)->value==-1) lastattrib++;
	alist=&(*alist)->next;
    }
    while (*elist) elist=&(*elist)->next;
    while (*rlist) rlist=&(*rlist)->next;
    frec=NULL;
    while (fgets(buffer, 511, f)) {
	linecount++;
	bpos=buffer;
	skip_spaces(bpos);
	if (!*bpos || *bpos=='#') continue;
	if (!strncmp(bpos,"ATTRIBUTE", 9)) {
	    /* define a new attribute:
	    ** ATTRIBUTE name value0 value1 value2 value3
	    ** value? is a string used as a reference
	    ** defval is the default value
	    ** name is the name used to define the attribute
	    ** (so, a line of the form 'name value1' defines the attribute
	    **  name to be equal to value1 for the current font)
	    **
	    ** The font attribute detection algorithm uses these values
	    ** to find default attributes for fonts from their name.
	    */
	    char *c,*h;
	    char tc;
	    int i=-1;
	    c=bpos+9;
	    tc=*c;
	    h=get_string(&c,&tc);
	    while (h) {
		*alist=malloc(sizeof(ATTRIBREC));
		(*alist)->name=
		add_alias(h,
			  ((i<0)?ATTRIBGROUPALIAS:ATTRIBVALUEALIAS(lastattrib)),
			  *alist);
		(*alist)->value=i;
		(*alist)->num=lastattrib;
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
		attribinfo[lastattrib].max=i;
		if (lastattrib) i=i*attribinfo[lastattrib-1].multby;
		attribinfo[lastattrib].multby=i;
		attribinfo[lastattrib].divby=i/attribinfo[lastattrib].max;
		attribinfo[lastattrib].def=0;
		lastattrib++;
	    } else if (i<0) {
		fprintf(stderr, "%s:%i: No attribute name given\n",
			confname, linecount);
	    }
	} else if (!strncmp(bpos, "ATTRALIAS", 9)) {
	    /* Redefine the attributes for a certain group */
	    /* For example:
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
	    if (!find_alias) {
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
	    ** Unicode U+0041.  Sorry, no internationalisation for numbers.
	    */
	    char *c, *h;
	    char tc;
	    RANGEREC *rr;
	    int len, actnum;
	    c=bpos+5;
	    tc=*c;
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr, "%s:%i: No name spacified\n",
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
	    /* define an encoding for a font. */
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
	    ** and encoding.
	    */
	    char *c,*h;
	    char tc;

	    c=bpos+4;
	    tc=*c;
	    h=get_string(&c,&tc);
	    frec = (*flist) = malloc(sizeof(FONTREC));
	    frec->fontname=malloc(sizeof(char)*(c-h+1)); /* c-h==strlen(h) */
	    strcpy(frec->fontname, h);
	    frec->attribpos=0;
	    frec->encoding=NULL;
	    frec->range=NULL;
	    frec->fonttype=frec->bt=frec->bm=frec->bmax=0;
	    frec->font=0;
	    frec->fstruct=NULL;
	    frec->next=NULL;
	    flist=&frec->next;
	    /* detect attributes:  !!! X11 specific code
	    ** * divide the font name in parts between '-' signs
	    ** * for each part, check if it is an alias
	    ** If the encoding is not set at the end, combine the
	    ** last two parts and check if that is an alias.
	    ** Keep track which attributes are already set to make
	    ** sure no redefinitions are made.
	    */
	    {  int attrset[MAXATTRIB];
	       char *t;
	       void *aliasdata;
	       int aliastype;
	       int tosetleft;
	       int i;
	       /* add an '-' at the end to process the last item */
	       c[0]='-';c[1]='\0';
	       c=h;
	       for (i=0; i<lastattrib; i++) attrset[i]=0;
	       tosetleft=lastattrib+2; /* range and encoding */
	       t=strchr(h,'-');
	       while (t && tosetleft) {
		   /* skip field of the form "*" and "" */
		   if (*c!='*' && *c!='-') {
		       *t='\0';
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
			       for (i=0; i<lastattrib; i++) {
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
		       *t='-';
		   }
		   t++;
		   c=t;
		   t=strchr(t,'-');
	       }
	    }
	    if (!frec->encoding) {
		/* try to find encoding in last two items
		** c is at a last position, after the added '-'
		** Remove it and search backward.
		*/
		c--;
		*c='\0';
		while (c>h && *c!='-') c--;   c--;
		while (c>h && *c!='-') c--;
		if (c!=h) {
		    c++;
		    frec->encoding=find_alias(c,ENCODINGALIAS);
		}
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
		fprintf(stderr,"%s:%i: unknow attribute group %s\n",
			confname, linecount, h);
		continue;
	    }
	    agr=ar->num;
	    /* get attribute value and look it up */
	    h=get_string(&c,&tc);
	    if (!h) {
		fprintf(stderr,"%s:%i: no attribute value specified.\n",
			confname, linecount);
		aval=NULL;
	    } else {
		aval=find_alias(h,ATTRIBVALUEALIAS(agr));
	    }
	    if (!aval) {
		char *vname;
		fprintf(stderr,"%s:%i: invalid attribute %s\n",
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
		/* change font attribute of current font */
		frec->attribpos=change_attribute(frec->attribpos, aval->num,
						 aval->value);
	    } else {
		fprintf(stderr, "%s:%i: Not defining a font yet\n",
			confname, linecount);
	    }
	}
    }
    close_file(f);
    return 0;
}
	
int main(int argc, char **argv)
{
    PathInfo pinfo;
    FONTREC *fr;
    ENCODINGREC *er;
    ALIASREC *ar;
    RANGEREC *rr;
    ATTRIBREC *abr;
    char attribcode[16];
    int i;

    pinfo=make_pathinfo("VIRTUALPATH", ".:/usr/lib/unicode", ".font");
    for (i=1; i<argc; i++) {
	load_font_config(pinfo,argv[i]);
    }
    fr=fontlist;
    printf("\nFonts:\n");
    attribcode[lastattrib]='\0';
    while (fr) {
	for (i=0;i<lastattrib; i++) {
	    attribcode[i]='A'+attribtovalue(fr->attribpos,i);
	}
	printf("%s\t%s\t%s\t%s\n", attribcode,
               (fr->encoding?fr->encoding->mapfilename:"<??>"),
               (fr->range?fr->range->name:"all"), fr->fontname);
	fr=fr->next;
    }
    er=encodinglist;
    printf("\nEncodings:\n");
    while (er) {
	printf("%s\n", er->mapfilename);
	er=er->next;
    }
    printf("\nAttributes\n");
    abr=attriblist;
    while (abr) {
	printf("%.2i %.2i\t%s\n", abr->num, abr->value, abr->name);
	abr=abr->next;
    }
    printf("\nRanges:\n");
    rr=rangelist;
    while (rr) {
	int *s;
	printf("  %s:", rr->name);
	s=rr->ranges;
	while (*s>=0) {
	    if (s[0]==s[1]) printf(" %i",s[0]);
            else printf(" %i-%i", s[0],s[1]);
	    s=s+2;
	}
	printf("\n");
	rr=rr->next;
    }
    printf("\nAliases\n");
    ar=aliaslist;
    while (ar) {
	printf(" %.4i\t%s\t%s\n", ar->aliastype, ar->name,
	       *((char**)ar->aliasdata));
	ar=ar->next;
    }
    printf("\nAttribute Info:\n");
    for (i=0; i<lastattrib; i++) {
	printf(" %.4i %.4i %.4i\n", attribinfo[i].max, attribinfo[i].multby,
	       attribinfo[i].divby);
    }
    return 0;
}

