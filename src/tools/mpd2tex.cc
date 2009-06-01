/* An example C program that loads an mpd document and converts it to
** latex.  For new files, this acts as a Output function. For old files, the
** content of the file might change due to automatic conversion from
** one format to another (which could result in corrupt templates).
** Under Unix, this program is a C++ program as it has to be linked with
** a C++ compiler.
**
** For linking, the following object files need to be compiled and
** linked.
**
** editor.o        C interface to C++ functions 
**   select.o      C++ module, to handle selections
**   node.o        C++ module, to handle nodes
**   editwindow.o  C++ module, to handle windows/documents
**   mark.o        C++ module, to handle pointers inside nodes
**   marker.o      C++ module, to handle pointers inside nodes
** fileread.o      C module, to read and write files
** notatype.o      C module, to handle templates, versions and stencils
** intstack.o      C module, to handle stacks of integers.
** flex.o          C module, to handle flexable arrays
** funcs.o         C module with some general function
**
** All those files should all be system independent.
** Some modules made calls to system specific functions, like drawing
** functions. Dummy definitions for those functions are located in 
** load_and_save.def
**
** Some functions might call Unix specific function which are not
** available in MSDOS/Windows.  Default definitions for those functions
** are located in unix.def
*/


extern "C" {
#include <stdio.h>
#include <stdlib.h>

/* type definitions */
#include "mathpad.h"

#define KEYCODE int
/* all the C++ function for tree manipulations and selections */
#include "editor.h"

/* the module to read/write a file */
#include "fileread.h"

/* the module to handle templates, versions and stencils */
#include "notatype.h"

/* for reading font definitions */
#include "system.h"
#include "fonts.h"

/* for generating LaTeX */
#include "latexout.h"

/* some dummy definitions which are needed for linking */
  /* #include "mpd2tex.def" */

/* some dummy definitions for Unix specific functions */
  /* #include "unix.def" */
}

int main(int argc, char **argv)
{
    FILE *f;
    void *windowdata;

    /* initialise the environment variables and directories */
    system_init(0,0);

    /* load font configuration files */
    /* make_fonts(); */

    /* initialise the stencil module */
    notatype_init();

    /* create a dummy window, to store the loaded document */
    windowdata=open_editwindow(0,0,0);

    /* open the input file */
    f=fopen(argv[1], "rb");

    /* add templates to a global database */
    edit_fnr=0;

    /* read the file */
    read_file(f,DOCUMENTFILE);

    /* get the document from the node stack */
    load_editwindow(windowdata);

    /* clean up all stacks and close the file */
    cleanup_nodestack();
    cleanup_filestack();
    cleanup_stencilstack();
    fclose(f);

    /* open the output file */
    f=fopen(argv[2], "wb");


    tex_set_file(f);
    tex_mode(0);
    tex_placeholders(1);
    latex_editwindow(windowdata);
    tex_unset();
    fclose(f);

}
