#ifndef MP_UNIFONTX11_H
#define MP_UNIFONTX11_H

/* define standard macros for system specific types and functions */

/* use the X11 definitions */
#include <X11/Xlib.h>

/* Font is defined. */
typedef Font FontID;

/* Define CharStruct as record for character information */

typedef XCharStruct CharStruct;

/* Define macros to address CharStruct fields
** Only those macros are use to address these fields.
** Redefinition of CharStruct is possible without
** any problems.
*/

#define CharWidth(CS)   ((CS)->width)
#define CharAscent(CS)  ((CS)->ascent)
#define CharDescent(CS) ((CS)->descent)
#define CharLeft(CS)    ((CS)->lbearing)
#define CharRight(CS)   ((CS)->rbearing)

#endif
