#include <stdlib.h>
#include <stdio.h>

#include "markup.h"
#include "unicode.h"
#include "unistring.h"
#include "box_markup.h"
#include "translate.h"

/* box_handle opens or closes boxes or changes attributes of the current
** box.
*/

static Uchar *box_handle(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    return NULL;
}

/* do not interpret the markup value, but print it instead */
static Uchar *box_print(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    return NULL;
}

void box_markup_init(void)
{
    if (!add_markup_handler(BoxAttrib, box_print, box_handle)) {
	fprintf(stderr, UstrtoLocale(translate("Unable to add box handler.\n")));
    }
}
