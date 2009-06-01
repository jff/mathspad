#ifndef MP_CONSTANT_H
#define MP_CONSTANT_H

#include "types.h"
#include "unicode.h"

/* Constants are stored in values as global variables.
** the lookup functions define new constants if they don't
** exist and return the index in the constant pool.
*/
extern Argument lookup_int_constant(int val);
extern Argument lookup_string_constant(Uchar *s);
extern Argument lookup_real_constant(double r);

#endif
