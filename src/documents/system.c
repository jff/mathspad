/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
** BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
** DAMAGES OR ANY DAMAGE WHATSOEVER RESULTING FROM
** LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
** CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
** OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
** OF THIS SOFTWARE.
** 
** 
** Roland Backhouse & Richard Verhoeven.
** Department of Mathematics and Computing Science.
** Eindhoven University of Technology.
**
********************************************************************/
/*
** file  system.c
** doel  directories, filenamen en  variabelen initiele waarden geven
*/

#include <stdlib.h>
#include <string.h>
#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "config.h"

#define USERDIR "mathspad" DIRSEPSTR
#define LATEXDIR "latex"   DIRSEPSTR
#define NOTATIONDIR "stencils" DIRSEPSTR
#define FONTFILENAME "MPFonts"
#define KEYPATH MATHPADKEYS

#ifndef MATHPADHOME
 #error Set the -DMATHPADHOME='"..."' option the makefile.
#endif

/*
**  Deze directories worden gebruikt bij het zoeken naar een file. Zo wordt het
**  mogelijk dat gebruikers eigen files gebruiken, die zijn aangepast
**  aan de eisen van die gebruiker.
*/


char **arguments;
int  number_of_arguments;
Char *progname = NULL,
     *homedir = NULL,
     *userdir = NULL,
     *latexdir = NULL,
     *notationdir = NULL,
     *fontfile[NR_SIZE] = { NULL },
     *keypath = NULL,
     *program_dir = NULL,
     *program_latexdir = NULL,
     *program_notationdir = NULL,
     *program_fontfilename = NULL,
     *program_keypath = NULL;
Char **help_dirs = NULL;
int nr_help_dirs = 0;

Char *latex_line_unit,
     *latex_tab_unit,
     *latex_space_unit;

int latex_side = 3,
    line_space = 1,
    screen_tab = 20,
    screen_space = 1,
    notation_precedence = 0,
    textdots = 1,
    autowordwrap=1,
    singlemode=0,
    autodouble=0,
    saveonexit=0,
    wait_time = 250;

int symbol_is_open = 0,
    notation_is_open = 0,
    edit_is_open = 0,
    buffer_is_open =0,
    notadef_is_open = 0,
    default_is_open = 0,
    remark_is_open = 0,
    find_is_open = 0,
    menu_is_open = 0,
    symbol_iconized = 0,
    edit_iconized = 0,
    notation_iconized = 0,
    notadef_iconized = 0,
    buffer_iconized = 0,
    default_iconized = 0,
    menu_iconized = 0,
    find_iconized = 0,
    remark_iconized = 0;

void system_init(int argc, char *argv[])
{
    char *temp;
    Char *c;
    char *g;
    int i,n=0,dhda=0;

    temp = getenv("HOME");
    if (temp) {
      homedir              = concat(FilenametoUstr(temp), translate(DIRSEPSTR));
    } else {
      homedir              = concat(translate("."), translate(DIRSEPSTR));
    }
    i=Ustrlen(homedir);
    if (homedir[i-2]==DIRSEPCHAR) homedir[i-1]='\0';
    userdir              = concat(homedir, translate(USERDIR));
    latexdir             = concat(userdir, translate(LATEXDIR));
    notationdir          = concat(userdir, translate(NOTATIONDIR));
    latex_line_unit      = concat(translate("2mm"),NULL);
    latex_tab_unit       = concat(translate("8.5mm"),NULL);
    latex_space_unit     = concat(translate("0.3em"),NULL);
    
    if (aig(temp = getenv("MATHPADHOME")))
      if (temp[strlen(temp)-1]==DIRSEPCHAR)
	program_dir = concat(translate(temp),NULL);
      else
	program_dir = concat(translate(temp), translate(DIRSEPSTR));
    else
      program_dir = concat(translate(MATHPADHOME DIRSEPSTR),NULL);
    /* make MATHPADFONTPATH equal to program_dir/fonts (if not set) */
    if (!getenv("MATHPADFONTPATH")) {
      char *d;
      int bl;
      bl=strlen("MATHPADFONTPATH=" MATHPADFONTPATH);
      d=malloc(sizeof(char)*(bl+1));
      if (d) {
	strcpy(d,"MATHPADFONTPATH=" MATHPADFONTPATH);
	putenv(d);
      }
    }
    /* make program_dir accessible with ~mathpad or $mathpad */
    if (!getenv("mathpad")) {
      char buffer[3000];
      int bl;
      char *d;
      sprintf(buffer,"mathpad=%s", UstrtoFilename(program_dir));
      bl=strlen(buffer);
      d = malloc(sizeof(d)*(bl+1));
      if (d) {
	strcpy(d,buffer);
	putenv(d);
      }
    }
    /* make program_dir accessible with ~mathspad or $mathspad */
    if (!getenv("mathspad")) {
      char buffer[3000];
      int bl;
      char *d;
      sprintf(buffer,"mathspad=%s", UstrtoFilename(program_dir));
      bl=strlen(buffer);
      d = malloc(sizeof(d)*(bl+1));
      if (d) {
	strcpy(d,buffer);
	putenv(d);
      }
    }
    /* add program_dir/bin to PATH variable */
    if (aig(temp = getenv("PATH"))) {
      char *p;
      i=strlen(temp)+strlen(UstrtoFilename(program_dir))+15;
      p= (char*) malloc(i*sizeof(char));
      strcpy(p,"PATH=");
      strcat(p,temp);
      i=strlen(p);
      p[i++]=':';
      strcpy(p+i,UstrtoFilename(program_dir));
      strcat(p+i,"bin");
      putenv(p);
    }
    if (aig(temp = getenv("MATHPADLATEX")))
      if (temp[strlen(temp)-1]==DIRSEPCHAR)
	program_latexdir = concat(FilenametoUstr(temp),NULL);
      else
	program_latexdir = concat(FilenametoUstr(temp), translate(DIRSEPSTR));
    else
      program_latexdir = concat(program_dir, translate(LATEXDIR));
    
    if (aig(temp = getenv("MATHPADSTENCIL")))
      if (temp[strlen(temp)-1]==DIRSEPCHAR)
	program_notationdir = concat(FilenametoUstr(temp), NULL);
      else
	program_notationdir = concat(FilenametoUstr(temp), translate(DIRSEPSTR));
    else
      program_notationdir = concat(program_dir, translate(NOTATIONDIR));
    
    if (aig(temp = getenv("MATHPADFONTS"))) {
      program_fontfilename = concat(translate(temp), NULL);
      fontfile[EDITFONT] = concat(translate(temp), NULL);
    } else {
      fontfile[EDITFONT]   = concat(translate(FONTFILENAME), NULL);
      program_fontfilename = concat(translate(FONTFILENAME), NULL);
    }
    temp = getenv("MATHPADHELPPATH");
    if (!temp) temp=":";
    g=temp;
    i=0;
    while (*g) { n+=(*g==':'); g++; i++; }
    help_dirs = (Char **) malloc(sizeof(char*) * (n*2+7));
    c = (Char*) malloc(sizeof(Char)*(n*10+i*2+2));
    g = temp;
    while (i>0 || (!i && !*g)) {
      int cp=0;
      help_dirs[nr_help_dirs]=c;
      while (aig(*c=*g++) && *c != ':') {
	cp+=(*c == '%');
	c++;i--;
      }
      i--;
      if (*c==':') *c='\0';
      if (help_dirs[nr_help_dirs]!=c) {
	if (*(c-1)!=DIRSEPCHAR && !cp) {
	  *c++=DIRSEPCHAR;
	  *c='\0';
	}
	if (!cp) {
	  nr_help_dirs++;
	  c++;
	  help_dirs[nr_help_dirs]=c;
	  concat_in(c,help_dirs[nr_help_dirs-1], translate("%.mpd"));
	  while (*c) c++;
	}
      } else if (!dhda) {
	dhda=1;
	help_dirs[nr_help_dirs++] =
	  concat(homedir, translate("mathspad" DIRSEPSTR "help" DIRSEPSTR "%.mpd"));
	help_dirs[nr_help_dirs++] =
	  concat(homedir, translate("mathspad" DIRSEPSTR "help" DIRSEPSTR));
	help_dirs[nr_help_dirs++] =
	  concat(program_dir, translate("help" DIRSEPSTR "%.mpd"));
	help_dirs[nr_help_dirs++] =
	  concat(program_dir, translate("help" DIRSEPSTR));
	help_dirs[nr_help_dirs++] = concat(translate("%.mpd"),NULL);
	help_dirs[nr_help_dirs] = concat(NULL,NULL);
      }
      nr_help_dirs++;
      c++;
    }
    for (i=1; i<NR_SIZE; i++)
      fontfile[i]= concat(NULL,NULL);
    if (aig(temp = getenv("MATHPADKEYS")))
      program_keypath = concat(translate(temp), NULL);
    else
      program_keypath = concat(translate(KEYPATH),NULL);
    keypath = concat(NULL,NULL);
    make_defaults(argc, argv);
}
