#include <stdlib.h>
#include <stdio.h>

#include "translate.h"
#include "markup.h"
#include "unicode.h"
#include "unifont.h"
#include "unistring.h"

static Uchar *font_handle(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int attribute,value;
    int curattr;
    curattr=attr->font;
    attribute=(lowvalue&0x3e0)>>5;
    value=(lowvalue&0x1f);
    attr->font=font_change_attribute(curattr, attribute, value);
    return 0;
}

static Uchar fontbuf[256];
static Uchar *font_print(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int attribute,value;
    char *v;
    Uchar *fb;
    attribute=(lowvalue&0x3e0)>>5;
    value=(lowvalue&0x1f);
    v=font_get_name(attribute,-1);
    UTFtoUstr(v,fontbuf);
    fb=fontbuf;
    while (*fb) fb++;
    *fb++=':';
    v=font_get_name(attribute,value);
    UTFtoUstr(v,fb);
    return fontbuf;
}

void font_markup_init(void)
{
    if (!add_markup_handler(FontAttrib, font_print, font_handle)) {
	fprintf(stderr,
		UstrtoLocale(translate("Unable to add font handler.\n")));
    }
}
