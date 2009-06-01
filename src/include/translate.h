#ifndef MP_TRANSLATE_H
#define MP_TRANSLATE_H

#include "unicode.h"

extern Uchar *translate(char *orig);
extern void set_translation8(char *orig, Uchar *trans);
extern void set_translation(Uchar *orig, Uchar *trans);

#endif
