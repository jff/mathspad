#ifndef MP_USERDEF_H
#define MP_USERDEF_H

#include "types.h"
#include "prototype.h"
#include "sequence.h"

/* userdefined functions */

extern FuncRef lookup_user_function(char *name);
extern int user_define(char *name, char *description, Prototype *prototype,
		       int nr_local,
		       Value *localvar, char **localname, Sequence *sequence);
extern void user_destroy(FuncRef udef);
extern Prototype *proto_user(FuncRef function);
extern Type  restype_user_function(FuncRef function);

extern int call_user_function(FuncRef userdef, Value **vlist);


#endif
