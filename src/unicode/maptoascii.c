#include <stdio.h>
#include <stdlib.h>

#include "unimap.h"
#define BSIZE 4000
int main(int argc, char **argv)
{
    MapUchar testing;
    void *data;
    int i,j;
    data=malloc(100000);
    fread(data, 100000, 1, stdin);
    MapUcharLoad(testing, data);
    for (i=0; i<0x10000; i++)
	if ((j=MapValue(testing, i)))
	    printf("0x%.2X 0x%.4X\n", i, j);
    return 0;
}
