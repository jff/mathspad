#include "reshuffle.h"

int reshuffle(int len, int *direct, void **item)
{
    /* smarter versions might be possible */
    int i,lb,ub,max;
    max=0;
    for (i=0; i<len; i++) {
	if (direct[i]>max) max=direct[i];
    }
    while (max) {
	i=0;
	while (i<len) {
	    while (i<len && direct[i]!=max) i++;
	    if (i!=len) {
		void *c;
		lb=i;
		while (i<len && direct[i]==max) { direct[i]=max-1; i++; }
		ub=i-1;
		while (lb<ub) {
		    c=item[lb];
		    item[lb]=item[ub];
		    item[ub]=c;
		    lb++;
		    ub--;
		}
	    }
	}
	max--;
    }
    return 0;
}
