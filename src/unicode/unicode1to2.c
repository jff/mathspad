#include <stdio.h>
#include <stdlib.h>
#include "unimap.h"

int main(int argc, char **argv)
{
    MapUchar onetwo;
    MapUchar rubbish;
    MapUchar newmap;
    char bufferot[0x80000];
    char bufferrubbish[0x80000];
    FILE *f;
    int i;
    Uchar j,k,iu;

    f=fopen("unicode1to2.map","rb");
    if (!f) return 1;
    fread(bufferot, 1, 20000,f);
    fclose(f);
    fread(bufferrubbish, 1, 0x80000, stdin);
    MapUcharLoad(rubbish, bufferrubbish);
    newmap = MapUcharCreate();
    MapUcharLoad(onetwo, bufferot);
    for (i=0; i<0x10000; i++) {
	iu=i;
	j=MapValue(rubbish, iu);
	k=MapValue(onetwo, j);
	if (k) MapUcharDefine(newmap, iu, k);
	else if (j) MapUcharDefine(newmap,iu,j);
    }
    MapUcharSave(newmap, stdout);
    return 0;
}

