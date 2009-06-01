#ifndef MP_FUNCTION_H
#define MP_FUNCTION_H

#include "types.h"
#include "prototype.h"

/* To make internal functions available to the language
** Prototypes are used to check if variables are passed correctly.
** Since arguments have to be passed correctly, an extra function
** might be needed to call the internal function.
*/

/* to report errors */
extern int error_status;

typedef void (*Function)();

/* Define an internal function. */
extern FuncRef define_function(char *name, char *description,
			       Prototype *prototype, Function funcptr);

/* Lookup an internal function  */
extern FuncRef lookup_function(char *name);

/* the prototype of a function */
extern Prototype *proto_function(FuncRef function);

/* the result type of a function */
extern Type restype_function(FuncRef function);

/* call internal function */
extern int call_function(FuncRef function, Value **vlist);

#endif

