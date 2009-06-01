#include <stdio.h>

#include "translate.h"
#include "markup.h"
#include "unicode.h"
#include "unistring.h"
#include "color_markup.h"

static Uchar *color_handle(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int color=attr->fgcolor;
    switch (lowvalue&0x300) {
    case 0x000:  color=color_set_red(color, lowvalue&0xff); break;
    case 0x100:  color=color_set_green(color, lowvalue&0xff); break;
    case 0x200:  color=color_set_blue(color, lowvalue&0xff); break;
    case 0x300:  color=color_set_cube(color, lowvalue&0xff); break;
    default: break;
    }
    attr->fgcolor=color;
    return 0;
}

static Uchar colorbuf[256];

/* no i18n for hex-values yet */

static char valtohex[]="0123456789ABCDEF";

static Uchar *color_print(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int val;
    Uchar *v;
    Uchar *cb;

    val=lowvalue&0xff;
    switch (lowvalue&0x300) {
    case 0x000:  v=translate("Red"); break;
    case 0x100:  v=translate("Green"); break;
    case 0x200:  v=translate("Blue"); break;
    case 0x300:  v=translate("ColorCube"); break;
    default: v=translate(""); break;
    }
    Ustrncpy(colorbuf, v, 240);
    cb=colorbuf;
    while (*cb) cb++;
    *cb++=':';
    *cb++=valtohex[(val>>4)&0xf];
    *cb++=valtohex[val&0xf];
    return colorbuf;
}

void color_markup_init(void)
{
    if (!add_markup_handler(ColorAttrib, color_print, color_handle)) {
	fprintf(stderr, UstrtoLocale(translate("Unable to add color handler.\n")));
    }
}
