#include <stdlib.h>
#include <stdio.h>

#include "translate.h"
#include "memman.h"
#include "markup.h"
#include "unicode.h"
#include "unistring.h"
#include "dynasize.h"

typedef struct ISOLANG {
    char iso2name[4];   /* 2 character code (ISO 639)  ascii */
    char iso3Tname[4];  /* 3 character code (ISO 639-2/T) ascii */
    char iso3Bname[4];  /* 3 character code (ISO 639-2/B) ascii */
    char *fullname;     /* full language name in UTF */
} ISOLANG;

static const ISOLANG isolang[] = {
    /* Automatically generated */
#include "iso639.c"
};

typedef struct UNOFLANG {
    char *codename;  /* code name (as used for MIME tags) */
    char *fullname;  /* language description */
} UNOFLANG;

/* languages not in ISO 639-2 (unofficial) / not in no language code */
static const UNOFLANG unoflang[] = {
    { "x-klingon", "Klingon (from Star Trek)" },
};

#define MAXISOLANG (sizeof(isolang)/sizeof(ISOLANG))
#define MAXUNOFLANG (sizeof(unoflang)/sizeof(UNOFLANG))

static Uchar *langbuf=NULL;
static int langbufsize=0;


static Uchar *lang_handle(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int i;
    i = lowvalue&0x3ff;
    if (i<MAXISOLANG) attr->language=i;
    else if (1023-i < MAXUNOFLANG) attr->language=i;
    return 0;
}

static Uchar *lang_print(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int i;
    char *name;
    Uchar *unname;
    char *lang;
    Uchar *unlang;
    i = lowvalue&0x3ff;
    if (i<MAXISOLANG) name=isolang[i].fullname;
    else if (1023-i<MAXUNOFLANG) name=unoflang[1023-i].fullname;
    else name="unknown";
    unname=translate(name);
    unlang=translate("Language");
    i=Ustrlen(unname)+Ustrlen(unlang)+5;
    if (!set_string_size(&langbuf, &langbufsize, i)) return 0;	
    langbuf[0]='[';
    Ustrcpy(langbuf+1, unlang);
    i=Ustrlen(langbuf);
    langbuf[i++]=':';
    Ustrcpy(langbuf+i, unname);
    i=i+Ustrlen(langbuf+i);
    langbuf[i++]=']';
    langbuf[i]='\0';
    return langbuf;
}

void lang_markup_init(void)
{
    if (!add_markup_handler(LangAttrib, lang_print, lang_handle)) {
	fprintf(stderr, UstrtoLocale(translate("Unable to add language handler.\n")));
    }
}
