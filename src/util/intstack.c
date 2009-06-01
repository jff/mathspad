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
#include "intstack.h"

static INTSTACK *freeint = 0;

int push_int(INTSTACK **s, int i)
{
    INTSTACK *h = freeint;
    if (!h)
	h = (INTSTACK*) malloc(sizeof(INTSTACK));
    else
	freeint = h->next;
    if (!h) return 0;
    h->next = *s;
    h->nr = i;
    *s = h;
    return 1;
}

int pop_int(INTSTACK **s)
{
    if (*s) {
	int i=(*s)->nr;
	INTSTACK *h = (*s)->next;
	(*s)->next = freeint;
	freeint = (*s);
	(*s) = h;
	return i;
    } else return 0;
}

void remove_int(INTSTACK **s, int nr)
{
    if (*s) {
	INTSTACK *h = *s;
	INTSTACK *g = 0;
	while (h && h->nr==nr) {
	    g = h;
	    h = h->next;
	}
	if (g) {
	    g->next = freeint;
	    freeint = *s;
	    *s = h;
	}
	while (h) {
	    if (h->nr==nr) {
		g->next = h->next;
		h->next = freeint;
		freeint = h;
		h = g->next;
	    } else {
		g = h;
		h = h->next;
	    }
	}
    }
}

void free_int(INTSTACK *s)
{
    if (s) {
	INTSTACK *h = s;
	while (h->next) h=h->next;
	h->next = freeint;
	freeint = s;
    }
}



