/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Technical University of Eindhoven (TUE)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of TUE not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  TUE makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** TUE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL TUE
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
** Technical University of Eindhoven.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "flexarray.h"


#define BLOCKSIZE 32
#define NEWSIZE(A)  (BLOCKSIZE * ((A+1)/BLOCKSIZE+1))

static int expand_flex(FlexArray *fl, int sz)
{
    if (fl->max < sz+1) {
        void *temp;

	if (fl->arr)
	    temp = (void *) realloc(fl->arr, fl->size * NEWSIZE(sz));
	else
	    temp = (void*) malloc(fl->size * NEWSIZE(sz));
	if (!temp) return 0; /* possible garbage collection */
	fl->arr = temp;
	fl->max = NEWSIZE(sz);
    }
    return 1;
}

static void membwd(void *d, void *s, void *e)
{
    char *g, *h;
    g = (char*) d;
    h = (char*) s;
    while (h != (char*)e) *g++ = *h++;
}

int int_cmp(const void *x, const void *y)
{
    return (*((int*)x) < *((int*)y) ? -1 : (*((int*)x) == *((int*)y) ? 0 : 1));
}

int fx_contains(FlexArray *fl, void *item)
{
    int i=0;

    if (!fl->nr) return 0;
    while (i<fl->nr) {
	if (!(fl->comp((char*)fl->arr+i*fl->size, item)))
	    return i+1;
	i++;
    }
    return 0;
}

void fx_remove(FlexArray *fl, void *item)
{
    int i,delta,cmpres;
    if (!fl->nr) return;
    delta=0;
    for (i=0; i<fl->nr; i++) {
	cmpres = !(fl->comp((char*)fl->arr+i*fl->size, item));
	if (delta && !cmpres)
	    membwd((char*)fl->arr+(i-delta)*fl->size,
		   (char*)fl->arr+i*fl->size,
		   (char*)fl->arr+(i+1)*fl->size);
	if (cmpres) delta++;
    }
    fl->nr = fl->nr-delta;
}

#define copy_item(A,B,S) membwd(A,B,B+S)

void fx_add(FlexArray *fl, void *item)
{
    if (!expand_flex(fl, fl->nr)) return;
    copy_item((char*)fl->arr+fl->nr*fl->size, (char*)item, fl->size);
    fl->nr++;
}

/*void fx_insert(FlexArray *fl, int pos, void *item)
{
    if (pos<0) pos=0;
    if (pos>=fl->nr)
	if (expand_flex(fl,fl->nr+1))
	    pos=fl->nr;
	else
	    return;
    if (pos<fl->nr)
	memfwd((char*)fl->arr+fl->nr*fl->size,
	       (char*)fl->arr+(fl->nr-1)*fl->size,
	       (char*)fl->arr+pos*fl->size); 
    copy_item((char*)fl->arr+pos*fl->size, (char*)item, fl->size);
}
*/
int fx_switch(FlexArray *fl, void *olditem, void *newitem)
{
    int i,changed=0;
    i=0;
    while (i<fl->nr) {
	if (!fl->comp((char*)fl->arr+i*fl->size, olditem)) {
	    copy_item((char*)fl->arr+i*fl->size, (char*)newitem, fl->size);
	    changed=i+1;
	}
	i++;
    }
    return changed;
}

void fx_init(FlexArray *fl, int sz, int (*cmp)(const void*,const void*))
{
    fl->size=sz;
    fl->comp=cmp;
    fl->max=0;
    fl->nr=0;
    fl->arr=NULL;
}

void fx_clear(FlexArray *fl)
{
    fl->max=0;
    fl->nr=0;
    if (fl->arr) free(fl->arr);
    fl->arr = 0;
}

int fx_set(FlexArray *fl, int pos, void *item)
{
    if (pos<0) return 0;
    if (pos>=fl->nr && !expand_flex(fl,fl->nr)) return 0;
    if (pos>=fl->nr) pos = fl->nr;
    copy_item((char*)fl->arr+pos*fl->size, (char*)item, fl->size);
    if (pos==fl->nr) fl->nr++;
    return pos+1;
}

void *fx_copy(FlexArray *fl)
{
    void *res;
    res = malloc(fl->size*fl->nr);
    if (res) memcpy(res, fl->arr, fl->size*fl->nr);
    return res;
}

