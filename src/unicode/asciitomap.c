#include <stdio.h>
#include <stdlib.h>

#include "unimap.h"
#define BSIZE 4000
int main(int argc, char **argv)
{
    MapUchar testing;
    MapChar testing1;
    int single;
    char buffer[BSIZE];
    int i,j,n=0;
    Uchar h;
    char h1;
    single= (argc>1 && argv[1][0]=='-' && argv[1][1]=='1');
    testing= MapUcharCreate();
    testing1=MapCharCreate();
    while (fgets(buffer, BSIZE,stdin)) {
	n++;
	if (sscanf(buffer, "%i %i", &i, &j)==2) {
	  h=j;h1=h;
	    if (!single) MapUcharDefine(testing, i, h);
	    else         MapCharDefine(testing1, i, h1);
	} else {
	    fprintf(stderr, "Error in line %i: %s", n,buffer);
	}
    }
    umap_shared=1;
    if (!single) MapUcharSave(testing, stdout);
    else         MapCharSave(testing1,stdout);
    return 0;
}
