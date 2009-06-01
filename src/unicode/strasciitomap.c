#include <stdio.h>
#include <stdlib.h>

#include "unimap.h"
#define BSIZE 4000
int main(int argc, char **argv)
{
    MapStr testing;
    char buffer[BSIZE];
    char content[0x100000];
    char *cp;
    int pos=0;
    int i,n=0;
    testing= MapStrCreate();
    for (i=2; i<33; i++) {
      content[pos]=i; content[pos+1]=0;
      cp=content+pos;
      MapStrDefine(testing, i, cp);
      pos=pos+2;
    }
    while (fgets(buffer, BSIZE,stdin)) {
	n++;
	if (sscanf(buffer, "%i", &i)==1) {
	  char *b;
	  b=buffer;
	  while (!isspace(*b)) b++;
	  b++;
	  if (!b) {
	    fprintf(stderr, "Error in line %i: %s", n, buffer);
	  } else {
	    cp=content+pos;
	    while (*b!='\n') {
	      content[pos++]=*b++;
	    }
	    content[pos++]=0;
	    MapStrDefine(testing, i, cp);
	  }
	}
    }
    umap_shared=1;
    MapStrSave(testing, stdout);
    return 0;
}
