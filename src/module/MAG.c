#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mathpad.h"
#include "system.h"
#include "message.h"
#include "sources.h"
#include "remark.h"
#include "edit.h"
#include "editor.h"
#include "notatype.h"
#include "latexout.h"
#include "unistring.h"
#include "parse.h"
#include "language.h"


int MAG_parse(unsigned char *buffer, unsigned int *len)
{
  lex_open_string(buffer);
  parse_input();
  *len=0;
  return 1;
}

void MAG_start(Char *title)
{
  open_program(translate("MAGscript %i&"), title, MAG_parse);
}

static Type tlist[3][4] = {
  { StringType, 0 }
};

static int call_str(int (*fcal)(), void **args)
{
  return (*fcal)(*((Uchar**)args[0]));
}

int init_library(void)
{
  Prototype *pt;

  pt = define_prototype(tlist[0],1, 0, call_str);
  define_function("MAG_start", "Start the MAG system.",
		  pt, MAG_start);
  return 1;
}
