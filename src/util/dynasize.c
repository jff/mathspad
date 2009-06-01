
/* for malloc and realloc */
#include "memman.h"

/* for Uchar */
#include "unicode.h"

int set_string_size(Uchar **str, int *length, int newlength)
{
    if (newlength< *length) return 1;
    if (!*str) {
	*str=(Uchar*) malloc(sizeof(Uchar)*newlength);
	if (!*str) return 0;
    } else {
	Uchar *nbuf;
	nbuf=(Uchar*) realloc(str, newlength*sizeof(Uchar));
	if (!nbuf) return 0;
	*str=nbuf;
    }
    *length=newlength;
    return 1;
}

