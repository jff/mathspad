#ifndef MP_VARIABLE_H
#define MP_VARIABLE_H

#include "types.h"
#include "unicode.h"

/*  handle global and local variables and constants */

/* for parsing */
/* define a global variable */
extern Argument define_global_variable(Type type, char *name);
/* define a program variable, to give the language access to program
** variables
*/
extern Argument define_program_variable(Type type, char *name, void *address);
/* define an integer constant (sharing) */
extern Argument define_int_constant(int n);
/* define a string constant (sharing) */
extern Argument define_string_constant(Uchar *s);
/* define a real constant (sharing) */
extern Argument define_real_constant(double r);

/* start a new list of local variables */
extern void new_local_variables(void);
/* add a local variable to the local list */
extern Argument define_local_variable(Type type, char *name);
/* get the complete list of local variables */
extern Value *get_local_variables(int *numvars);

/* during execution */
extern void push_local_variables(Value *list, int nr);
extern void pop_local_variables(void);
extern Value *get_local_values(int *len);
extern Value *get_value(Argument arg);
extern Type get_type(Argument arg);

/* for debugging purposes */
extern char **get_local_names(void);
extern void destroy_local_names(char **locnam);

extern Argument lookup_variable(char *name);
extern Type     lookup_arg_type(Argument arg);

extern void variable_init(void);

#endif
