
#include <stdlib.h>
#include <stdio.h>
#include "unimap.h"
#include "unistring.h"
#include "filefind.h"

static MapUstr decomp=NULL;

int main( int argc, char **argv)
{
    Uchar j;
    Uchar *stack[40];
    PathInfo mappath=0;
    int spos=0;
    int fsize;
    char *n;
    char *buffer;
    FILE *f;
    mappath=make_pathinfo("MAPPATH", ".", ".map");
    n="Decomposition";
    buffer = malloc((fsize=file_size(mappath,n))+1);
    f=open_file(mappath, n,"rb");
    fread(buffer,fsize+1,1,f);
    close_file(f);
    MapUstrLoad(decomp, buffer);
    /* print full decomposition of all characters */
    for (j=0; j<0xffff; j++) {
	stack[0]=MapValue(decomp,j);
	if (stack[0]) {
	    spos=0;
	    while (spos || *stack[spos]) {
		if (!*stack[spos]) {
		    spos--;
		} else if ((stack[spos+1]=MapValue(decomp,*stack[spos]))) {
		    stack[spos]++;spos++;
		} else {
		    printf("0x%0.4X ", *stack[spos]);
		    stack[spos]++;
		}
	    }
	    printf(" -> 0x%0.4X\n", j);
	}
    }
    return 0;
}
