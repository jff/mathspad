#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unimap.h"

static Uchar dcbuf[15000];
static int dcp=0;

#define CODEVAL    field[0]
#define NAME       field[1]
#define CATEGORY   field[2]
#define CANCOMB    field[3]
#define BIDICAT    field[4]
#define CHARDECOMP field[5]
#define DECDIGVAL  field[6]
#define DIGVAL     field[7]
#define FRACTION   field[8]
#define MIRRORED   field[9]
#define OLDNAME    field[10]
#define COMMENT    field[11]
#define UPEQ       field[12]
#define LOWEQ      field[13]
#define TITLEEQ    field[14]

/* for character types */
#define IS_NONSPACE     (1<<0)
#define IS_COMBINING    (1<<1)
#define IS_DIGIT        (1<<2)
#define IS_OTHER_NUM    (1<<3)
#define IS_SPACE        (1<<4)
#define IS_LINE         (1<<5)
#define IS_PARAGRAPH    (1<<6)
#define IS_CONTROL      (1<<7)
#define IS_PRIVATE      (1<<8)
#define IS_NOT_ASSIGNED (1<<9)
#define IS_UPPERCASE    (1<<10)
#define IS_LOWERCASE    (1<<11)
#define IS_TITLECASE    (1<<12)
#define IS_MODIFIER     (1<<13)
#define IS_OTHER_LETTER (1<<14)
#define IS_DASH         (1<<15)
#define IS_OPEN         (1<<16)
#define IS_CLOSE        (1<<17)
#define IS_OTHER_PUNCT  (1<<18)
#define IS_MIRROR       (1<<19)
#define IS_MATH         (1<<20)
#define IS_CURRENCY     (1<<21)
#define IS_OTHER_SYMBOL (1<<22)

/* for bidi algorithm */
#define IS_LEFT_RIGHT    (1<<23)
#define IS_RIGHT_LEFT    (1<<24)
#define IS_EURO_NUM      (1<<25)
#define IS_EURO_NUM_SEP  (1<<26)
#define IS_EURO_NUM_TERM (1<<27)
#define IS_ARAB_NUM      (1<<28)
#define IS_WHITESPACE    (1<<29)
#define IS_OTHER_NEUT    (1<<30)

/* IS_SEGMENT_SEP is not needed: no character have that value
** IS_BLOCK_SEP == (IS_U2028 || IS_U2029)
** IS_COMMON_SEP == (IS_U002C || IS_U003A)
*/

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

static struct { char *str; int len; } formattag[]= {
    {"NST",3}, {"NST",3},
    {"circle>",7},
    {"compat>",7},
    {"final>",6},
    {"font>",5},
    {"fraction>",9},
    {"initial>",8},
    {"isolated>",9},
    {"medial>",7},
    {"narrow>",7},
    {"noBreak>",8},
    {"small>",6},
    {"square>",7},
    {"sub>",4},
    {"super>",6},
    {"vertical>",9},
    {"wide>",5},
    {NULL, 0}
};

/* extra tables needed for language/script dependent information */

int main(int argc, char **argv)
{
    char buffer[1000];
    int i,un;
    Uchar value1, value2,alt;
    Uchar *st;
    int category;
    unsigned char a;
    char *field[15];
    char *c;
    FILE *f;

    MapChar cancomclass;
    MapChar decomptag;
    MapInt gclass;
    MapUchar tonoaccent;
    MapUchar tolow;
    MapUchar toup;
    MapUchar totit;
    MapUchar tonum;
    MapUchar todenum;
    MapUchar tomedial;
    MapUchar tofinal;
    MapUchar toinitial;
    MapUchar toisolated;
    MapUchar tospacing;  /* non-spacing/combining marks can be displayed with
			 ** their spacing alternatives */
    MapUstr decomp;

    cancomclass=MapCharCreate();
    tonoaccent=MapUcharCreate();
    gclass=MapIntCreate();
    tolow=MapUcharCreate();
    toup=MapUcharCreate();
    totit=MapUcharCreate();
    tonum=MapUcharCreate();
    todenum=MapUcharCreate();
    alt=1;
    for (i=0; i<0x10000; i++) {
	MapUcharDefine(todenum, i,alt);
    }
    tomedial=MapUcharCreate();
    tofinal=MapUcharCreate();
    toinitial=MapUcharCreate();
    toisolated=MapUcharCreate();
    tospacing=MapUcharCreate();
    decomp=MapUstrCreate();
    decomptag=MapCharCreate();

    while (fgets(buffer, 1000, stdin)) {
	c=buffer;
	for (i=0; i<14; i++) {
	    field[i]=c;
	    c=strchr(c,';');
	    *c=0;
	    c++;
	}
	field[14]=c;
	while (*c!='\n') c++;
	*c=0;
	sscanf(CODEVAL, "%x", &i);
	un=i;
	category=0;
	switch (CATEGORY[0]) {
	case 'L':
	    switch (CATEGORY[1]) {
	    case 'u': category=IS_UPPERCASE;break;
	    case 'l': category=IS_LOWERCASE;break;
	    case 't': category=IS_TITLECASE;break;
	    case 'm': category=IS_MODIFIER;break;
	    case 'o': category=IS_OTHER_LETTER;break;
	    default: break;
	    }
	    break;
	case 'M':
	    switch (CATEGORY[1]) {
	    case 'n': category=IS_COMBINING;break;
	    case 'c': category=IS_NONSPACE;break;
	    default: break;
	    }
	    break;
	case 'N':
	    switch (CATEGORY[1]) {
	    case 'd': category=IS_DIGIT;break;
	    case 'o': category=IS_OTHER_NUM;break;
	    default: break;
	    }
	    break;
	case 'Z':
	    switch (CATEGORY[1]) {
	    case 's': category=IS_SPACE;break;
	    case 'l': category=IS_LINE;break;
	    case 'p': category=IS_PARAGRAPH;break;
	    default: break;
	    }
	    break;
	case 'C':
	    switch (CATEGORY[1]) {
	    case 'c': category=IS_CONTROL;break;
	    case 'o': category=IS_PRIVATE;break;
	    case 'n': category=IS_NOT_ASSIGNED;break;
	    default: break;
	    }
	    break;
	case 'P':
	    switch (CATEGORY[1]) {
	    case 'd': category=IS_DASH;break;
	    case 's': category=IS_OPEN;break;
	    case 'e': category=IS_CLOSE;break;
	    case 'o': category=IS_OTHER_PUNCT;break;
	    default: break;
	    }
	    break;
	case 'S':
	    switch (CATEGORY[1]) {
	    case 'm': category=IS_MATH;break;
	    case 'c': category=IS_CURRENCY;break;
	    case 'o': category=IS_OTHER_SYMBOL; break;
	    default: break;
	    }
	    break;
	default:
	    break;
	}
	switch (BIDICAT[0]) {
	case 'L': category|=IS_LEFT_RIGHT; break;
	case 'R': category|=IS_RIGHT_LEFT; break;
	case 'W': category|=IS_WHITESPACE; break;
	case 'O': category|=IS_OTHER_NEUT; break;
	case 'A': category|=IS_ARAB_NUM; break;
	case 'E':
	    switch (BIDICAT[1]) {
	    case 'N': category|=IS_EURO_NUM; break;
	    case 'S': category|=IS_EURO_NUM_SEP; break;
	    case 'T': category|=IS_EURO_NUM_TERM; break;
	    default: break;
	    }
	    break;
	default:  break;
	}
	if (!category)
	   printf("%s", CODEVAL);
	if (MIRRORED[0]=='Y') category= (category|IS_MIRROR);
	MapIntDefine(gclass, un, category);
	sscanf(UPEQ, "%x", &i);
	alt=i;
	MapUcharDefine(toup,un, alt);
	i=0;
	sscanf(LOWEQ, "%x", &i);
	alt=i;
	MapUcharDefine(tolow,un, alt);
	i=0;
	sscanf(TITLEEQ, "%x", &i);
	alt=i;
	MapUcharDefine(totit, un, alt);
	i=0;
	sscanf(CANCOMB, "%i", &i);
	a=i;
	MapCharDefine(cancomclass, un, a);
	i=0;
	sscanf(FRACTION, "%i", &i);
	value1=i;
	c=strchr(FRACTION,'/');
	if (c) {
	    c++;
	    i=0;
	    sscanf(c, "%i", &i);
	    value2=i;
	} else { value2=1; }
	MapUcharDefine(tonum,un,value1);
	MapUcharDefine(todenum,un,value2);
	c=CHARDECOMP;
	if (c[0]) {
	    a=DEC_CANON;
	    if (c[0]=='<') {
		c++;
		for (i=2; formattag[i].str; i++) {
		    if (!strncmp(c,formattag[i].str,formattag[i].len)) {
			a=i;
			c=c+formattag[i].len;
			break;
		    }
		}
		if (c[-1]=='<') {
		    printf("%s: unknow format tag in decomposition\n",
			   CODEVAL);
		    while (*c && *c++!='>');
		}
	    } else c--;
	    st=dcbuf+dcp;
	    MapCharDefine(decomptag, un, a);
	    value1=0;
	    do {
		c++;
		sscanf(c, "%x", &i);
		alt=i;
		dcbuf[dcp++]=alt;
		c=c+4;
		value1++;
	    } while (*c);
	    dcbuf[dcp++]=0;
	    MapUstrDefine(decomp,un, st);
	    i=value1;
	    alt=un;
	    switch (a) {
	    case DEC_FINAL:
		if (i==1) MapUcharDefine(tofinal, st[0], alt);
		break;
	    case DEC_MEDIAL:
		if (i==1) MapUcharDefine(tomedial, st[0], alt);
		break;
	    case DEC_ISOLATED:
		if (i==1) MapUcharDefine(toisolated, st[0], alt);
		break;
	    case DEC_INITIAL:
		if (i==1) MapUcharDefine(toinitial, st[0], alt);
		break;
	    case DEC_COMPAT:
		if (i==2 && st[0]==' ') MapUcharDefine(tospacing, st[1],alt);
		break;
	    default: break;
	    }
	}
    }
    umap_shared=1;
    f=fopen("CanComClass.map", "wb");
    MapCharSave(cancomclass, f);
    fclose(f);
    f=fopen("DeCompTag.map", "wb");
    MapCharSave(decomptag, f);
    fclose(f);
    f=fopen("GenClass.map", "wb");
    MapIntSave(gclass, f);
    fclose(f);
    f=fopen("ToNoAccent.map", "wb");
    MapUcharSave(tonoaccent, f);
    fclose(f);
    f=fopen("ToLower.map", "wb");
    MapUcharSave(tolow, f);
    fclose(f);
    f=fopen("ToUpper.map", "wb");
    MapUcharSave(toup, f);
    fclose(f);
    f=fopen("ToTitle.map", "wb");
    MapUcharSave(totit, f);
    fclose(f);
    f=fopen("ToNum.map", "wb");
    MapUcharSave(tonum, f);
    fclose(f);
    f=fopen("ToDenum.map", "wb");
    MapUcharSave(todenum, f);
    fclose(f);
    f=fopen("ToMedial.map", "wb");
    MapUcharSave(tomedial, f);
    fclose(f);
    f=fopen("ToFinal.map", "wb");
    MapUcharSave(tofinal, f);
    fclose(f);
    f=fopen("ToInitial.map", "wb");
    MapUcharSave(toinitial, f);
    fclose(f);
    f=fopen("ToIsolated.map", "wb");
    MapUcharSave(toisolated, f);
    fclose(f);
    f=fopen("ToSpacing.map", "wb");
    MapUcharSave(tospacing, f);
    fclose(f);
    f=fopen("Decomposition.map", "wb");
    MapUstrSave(decomp, f);
    fclose(f);
    return 0;
}

