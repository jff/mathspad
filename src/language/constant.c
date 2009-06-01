#include "constant.h"
#include "variable.h"
#include "unistring.h"

/* Constants are stored as variables. To find the constants in the
** variable database, extra databases are used for the different types.
** Each databases stores a collection of Arguments and the
** compare function selects the correct values.
*/

Argument lookup_int_constant(int n)
{
    return define_int_constant(n);
}

Argument lookup_string_constant(Uchar *s)
{
    return define_string_constant(s);
}

Argument lookup_real_constant(double r)
{
    return define_real_constant(r);
}
