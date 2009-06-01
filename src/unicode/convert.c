#include <stdlib.h>
#include "uniconv.h"
#include "unimap.h"
#include "unistring.h"
#include "filefind.h"

Uchar *convname;
Uchar *list;
int enlen;

void read_string(char *fname)
{
    FILE *f;
    char *buffer;
    int i;
    UConvID cid;
    UConverter *ucv;
    if (!(f=open_file(0,fname, "rb"))) return;
    i = file_size(0,fname);
    buffer=malloc((i+1));
    i=fread(buffer,1, i, f);
    close_file(f);
    buffer[i]=0;
    cid=UConvGetID(convname);
    ucv=UConvGet(cid);
    enlen=UConvEnLen(buffer, ucv);
    list=UConvEncode(buffer, NULL, ucv);
    free(buffer);
    f=fopen("/tmp/rubbish", "wb");
    if (f) {
      fwrite(list, 1,enlen*2,f);
      fclose(f);
    }
    /* i=UTFstrlen(buffer);
    list = (Uchar*) malloc(sizeof(Uchar)*(i+1));
    UTFtoUstr(buffer, list); */
}


int main(int argc, char **argv)
{
    Uchar enname[400];
    char convutf[400];
    UConvID convid;
    UConverter *uconv;
    int minimum,i;
    UConvID minid=-1;

    UConvLoadDatabase("UniConvert");
    UTFtoUstr(argv[1],enname);
    convname=enname;
    read_string(argv[2]);
    convid=0;
    minimum=Ustrlen(list)*2;
    printf("%40s\t%i\n", "2byte Unicode", minimum);
    while ((uconv=UConvGet(convid))) {
	if (UConvDeFail(list, uconv)) {
	    /* uconv fails */
	} else {
	    char fname[512];
	    char *b;
	    FILE *f;
	    i=UConvDeLen(list, uconv);
	    UstrtoUTF(UConvName(uconv),convutf);
	    sprintf(fname, "/tmp/rubbish.%s",convutf);
	    b=malloc(i+5);
	    UConvDecode(list, b,uconv);
	    f=fopen(fname,"wb");
	    fwrite(b,i,1,f);
	    fclose(f);
	    printf("%40s\t%i\n", convutf, i);
	    if (i<minimum) { minimum=i; minid=convid; }
	}
	convid++;
    }
    if (minid==-1) {
	printf("Minimum: 2byte Unicode (%i)\n", minimum);
    } else {
	uconv=UConvGet(minid);
	UstrtoUTF(UConvName(uconv), convutf);
	printf("Minimum: %s (%i)\n", convutf, minimum);
    }
    return 0;
}
