/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
** Permission to use, copy, modify and distribute this software
** and its documentation for any purpose is hereby granted
** without fee, provided that the above copyright notice appear
** in all copies and that both that copyright notice and this
** permission notice appear in supporting documentation, and
** that the name of EUT not be used in advertising or publicity
** pertaining to distribution of the software without specific,
** written prior permission.  EUT makes no representations about
** the suitability of this software for any purpose. It is provided
** "as is" without express or implied warranty.
** 
** EUT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS
** SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL EUT
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
** Eindhoven University of Technology.
**
********************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include "memman.h"
#include "fstate.h"

#include "unistring.h"

typedef unsigned short STATE;

typedef unsigned int ARROW;

#define EPSILON 0x7F7E
#define DUMMY   0x7F7F

#define arr_lab(A)  ((A)&0xFFFF)
#define arr_dest(A) ((A)>>16)

typedef unsigned int BITSET;

int setsize=0;

#define BIBS (8*sizeof(BITSET))
#define contains(A,B) (((A)[(B)/BIBS]&(0x1<<((B)%BIBS)))>0)
#define add(A,B) ((A)[(B)/BIBS] |= (0x1<<((B)%BIBS)))

static void unite(BITSET *a, BITSET *b, BITSET *c)
{
    int i;
    for (i=0; i<setsize; i++) a[i]=b[i]|c[i];
}

static int equal(BITSET *a, BITSET *b)
{
    int i;
    for (i=0; i<setsize; i++) if (a[i]!=b[i]) return 0;
    return 1;
}

static void copy(BITSET *a, BITSET *b)
{
    int i;
    for (i=0; i<setsize; i++) a[i]=b[i];
}

static void clear(BITSET *a)
{
    int i;
    for (i=0; i<setsize; i++) a[i]=0;
}

typedef
struct {
    int final;
    int pos;
    BITSET *bs;
    int nra;
    ARROW *arr;
} STATECHANGE;

#define init_sc(A,B,C,D,E,F) { (A)->final=(A)->pos=(A)->nra=0; \
			    (A)->arr = (B); \
			    if (C || D) {(A)->nra++; *(B++)=(((D)<<16)+(C));} \
			    if (E || F) {(A)->nra++; *(B++)=(((F)<<16)+(E));} \
			 }

typedef
struct {
    int states;
    int max;
    STATECHANGE *changes;
} FINITESTATE;

FSTATE *make_fstate(Uchar *mask, int minimize)
{
    FINITESTATE *ndfa;
    FINITESTATE *dfa;
    STATECHANGE *sc, *h;
    BITSET *buffer=NULL;
    int bsize=0;
    BITSET *all;
    BITSET *freestack[500];
    int ftop=0;
    FSTATE *fstate;
    BITSET *supstate;
    BITSET *dumstate;
    ARROW *csar, *abuf, *sabuf;
    int ls,cs;
    int fail=0;

    /*
    ** make a non-deterministic finite state machine for the
    ** specified mask. At the moment only * and ? are recognized.
    */
    ndfa = (FINITESTATE*) malloc( sizeof(FINITESTATE));
    ndfa->states = 1;
    ls = Ustrlen(mask)+1;
    ndfa->changes = (STATECHANGE*) malloc(sizeof(STATECHANGE)*ls*3);
    ndfa->max=ls*3;
    sabuf = abuf = (ARROW*) malloc(sizeof(ARROW)*ls*5);
    ls=0; /* last state */
    sc = ndfa->changes+ls;
    while (*mask) {
	if (*mask == '*') {
	    /*
	    **       eps          ?          eps
	    **   ls -----> ls+1 -----> ls+2 -----> ls+3
	    **   |          ^    eps    |           ^
	    **   |          |-----------|           |
	    **   |               eps                |
	    **   |----------------------------------|
	    */
	    init_sc(sc, abuf, EPSILON, ls+1, EPSILON, ls+3);
	    sc++;ls++;
	    init_sc(sc, abuf, DUMMY, ls+1, 0,0);
	    sc++;ls++;
	    init_sc(sc, abuf, EPSILON, ls-1, EPSILON, ls+1);
	    sc++;ls++;
	    ndfa->states+=3;
	} else {
	    if (*mask == '?') {
		/*
		**       ?
		**  ls -----> ls+1
		*/
		init_sc(sc, abuf, DUMMY, ls+1, 0,0);
	    } else {
		/*
		**     *mask
		**  ls -----> ls+1
		*/
		init_sc(sc, abuf, *mask, ls+1, 0,0);
	    }
	    sc++;
	    ls++;
	    ndfa->states++;
	}
	mask++;
    }
    init_sc(sc,abuf,0,0,0,0);
    /*
    ** Determine the epsilon closure sets. Each epsilon arrow points to
    ** a state with a larger number or the epsilon closure of that state
    ** only contains that state.
    */
    setsize = (ndfa->states)/BIBS+1;
    all = (BITSET*) malloc(setsize*(ndfa->states)*sizeof(BITSET));
#define epsclosure(A) (all+(A)*setsize)
    for (ls=0;ls<setsize*ndfa->states; ls++) all[ls]=0;
    for (ls=ndfa->states-1; ls>=0; ls--) {
	int i;
	add(epsclosure(ls), ls);
	sc = ndfa->changes+ls;
	for (i=0; i<sc->nra; i++) {
	    if (arr_lab(sc->arr[i])==EPSILON) {
		int j=arr_dest(sc->arr[i]);
		add(epsclosure(ls),j);
		unite(epsclosure(ls),epsclosure(ls),epsclosure(j));
	    }
	}
    }
    /*
    ** Make the non-deterministic finite state machine deterministic.
    ** (Using the algoritm with metastates (sets of states), divided in
    ** `known' and `unknown' metastates)
    */
    dfa = (FINITESTATE*) malloc( sizeof(FINITESTATE));
    dfa->states = 1;
    dfa->changes = (STATECHANGE*) malloc(sizeof(STATECHANGE)*32);
    dfa->max=32;
    bsize=0;

#define GET_SET(A) if (!bsize) \
    { buffer = (BITSET*) malloc(setsize*(ndfa->states+1)*sizeof(BITSET)); \
      freestack[ftop++]=buffer;bsize=ndfa->states+1; }\
      (A)=buffer; bsize--; buffer+=setsize;

    ls=1;
    cs=0;
    GET_SET(dfa->changes[cs].bs);
    copy(dfa->changes[cs].bs,epsclosure(0));
    dfa->changes[cs].final = contains(epsclosure(0),ndfa->states-1);
    csar = (ARROW*) malloc(2*ndfa->states*sizeof(ARROW));
    GET_SET(supstate);
    GET_SET(dumstate);
    while (cs != ls && !fail) {
	int i,j,n,ds;
	n=0;
	for (i=0; i<ndfa->states; i++) {
	    if (contains(dfa->changes[cs].bs, i)) {
		int k=0;
		for (k=0; k<ndfa->changes[i].nra; k++) {
		    unsigned int lab=arr_lab(ndfa->changes[i].arr[k]);
		    if (lab!=EPSILON) {
			int l=n;
			while (l>0 && arr_lab(csar[l-1])>lab) {
			    csar[l]=csar[l-1];
			    l--;
			}
			csar[l]=ndfa->changes[i].arr[k];
			n++;
		    }
		}
	    }
	}
	/*
	** csar contains all relevant arrows from the currect state.
	** for every label, the set of possible new states is calculated
	** the arrows are sorted assending w.r.t. the label.
	*/
	dfa->changes[cs].arr = (ARROW*) malloc((n+1)*sizeof(ARROW));
	dfa->changes[cs].nra = 0;
	i=n-1;
	clear(dumstate);
	ds=-1;
	while (i>=0 || ds<0) {
	    unsigned int lab;
	    lab=arr_lab(csar[i]);
	    /*
	    ** if there is no dummy label, add one (it will automatically
	    ** point to the empty metastate).
	    */
	    if (lab!=DUMMY && ds<0) lab=DUMMY;
	    copy(supstate,dumstate);
	    while (i>=0 && arr_lab(csar[i])==lab) {
		unite(supstate, supstate, epsclosure(arr_dest(csar[i])));
		i--;
	    }
	    /*
	    ** supstate contains the states reachable with a dummy arrow
	    ** or an arrow labelled lab
	    */
	    if (lab==DUMMY || !equal(dumstate, supstate)) {
		for (j=0; j<ls && !equal(supstate, dfa->changes[j].bs); j++);
		if (j==ls) {
		    if (dfa->max <dfa->states+1) {
			h = (STATECHANGE*) 
			    realloc(dfa->changes,sizeof(STATECHANGE)*
				    (dfa->max+32));
			if (!h) {
			    fail=1;
			    continue;
			}
			dfa->max += 32;
			dfa->changes = h;
		    }
		    GET_SET(dfa->changes[ls].bs);
		    dfa->states++;
		    copy(dfa->changes[ls].bs, supstate);
		    dfa->changes[ls].final = contains(supstate,ndfa->states-1);
		    ls++;
		}
		if (lab==DUMMY) {
		    copy(dumstate, supstate);
		    ds=j;
		}
		dfa->changes[cs].arr[dfa->changes[cs].nra++] = (j<<16)+lab;
	    }
	}
	cs++;
    }
    free(all);
    free(sabuf);
    for (ls=0; ls<ftop; ls++) free(freestack[ls]);
    free(ndfa->changes);
    free(ndfa);
    if (fail) {
	for (ls=0; ls<dfa->states; ls++) free(dfa->changes[ls].arr);
	free(dfa->changes);
	free(dfa);
	return NULL;
    }
    /*
    ** Minimize the deterministic finite state machine. This may take some
    ** time, especially if lots of ** and ?? are used.
    **
    ** Two states are equivalent if the outgoing arrows are all equal.
    **
    ** ls => maybe there are equivalent states
    */
    ls=minimize;
    while (ls) {
	int i,j=0;
	ls=0;
	for (i=1; !ls && i<dfa->states; i++) {
	    if (dfa->changes[i].arr) {
		for (j=0; !ls && j<i; j++) {
		    if (dfa->changes[j].arr &&
			dfa->changes[i].final==dfa->changes[j].final &&
			dfa->changes[i].nra==dfa->changes[j].nra) {
			int n;
			for (n=0; n<dfa->changes[i].nra &&
			     dfa->changes[i].arr[n]==dfa->changes[j].arr[n]; n++);
			ls = (n==dfa->changes[i].nra);
		    }
		}
	    }
	}
	if (ls) {
	    int k,l,m;
	    j--;i--;
	    for (k=0; k<dfa->states; k++) {
		for (l=0,m=0;l<dfa->changes[k].nra;l++) {
		    if (arr_dest(dfa->changes[k].arr[l])==(unsigned int)i)
			dfa->changes[k].arr[m] -= ((i-j)<<16);
		    else
			dfa->changes[k].arr[m] = dfa->changes[k].arr[l];
		    if (!l ||
			(arr_dest(dfa->changes[k].arr[0])!=
			 arr_dest(dfa->changes[k].arr[m])))
			m++;
		}
		dfa->changes[k].nra=m;
	    }
	    free(dfa->changes[i].arr);
	    dfa->changes[i].arr=NULL;
	    dfa->changes[i].nra=0;
	}
    }
    /*
    ** encode the finite state machine (minimized or not) in an easy to use
    ** string.
    */
    cs=1;
    for (ls=0; ls<dfa->states; ls++)
	if (dfa->changes[ls].arr) {
	    dfa->changes[ls].pos=cs;
	    cs+=2*(dfa->changes[ls].nra+1);
	}
    fstate = (FSTATE*) malloc(cs * sizeof(FSTATE));
    fstate[0]=cs;
    for (ls=0; ls<dfa->states; ls++)
	if (dfa->changes[ls].arr) {
	    int i,j;
	    j=dfa->changes[ls].pos;
	    fstate[j++]=dfa->changes[ls].final;
	    fstate[j++]=(dfa->changes[ls].nra-1)*2;
	    for (i=dfa->changes[ls].nra-1; i>=0; i--) {
		fstate[j]=arr_lab(dfa->changes[ls].arr[i]);
		if (fstate[j]==DUMMY) fstate[j]=0;
		j++;
		fstate[j++]=dfa->changes[arr_dest(dfa->changes[ls].arr[i])].pos;
	    }
	    free(dfa->changes[ls].arr);
	}
    free(dfa->changes);
    free(dfa);
    return fstate;
}

int fstate_check(FSTATE *fs, Uchar *c)
{
    int i,j;
    i = 1;
    while (*c) {
	for (j=0;j<(int)fs[i+1] && fs[i+2+j]!= (unsigned short) *c; j+=2);
	i = fs[i+3+j];
	c++;
    }
    return fs[i];
}

void free_fstate(FSTATE *fs)
{
    free(fs);
}

#ifdef FSTATE_STAND_ALONE

static void print_cprog_finitestate(FSTATE *fs)
{
    int i,n;
    n=0;
    fprintf(stdout, "int dfa_check(unsinged short *s)\n{\n\tint i,j;\n"
	    "\tunsigned short c[%i] = {\n\t\t\t", fs[0]);
    fprintf(stdout, "0");
    i=1;
    while (i<fs[0]) {
	fprintf(stdout, ",\n\t/%c %i %c/\t", '*', i,'*');
	n=fs[i+1]/2+1;
	fprintf(stdout, "%i,%i",fs[i],fs[i+1]);
	i+=2;
	while (n) {
	    if (fs[i]>=' ' && fs[i]<127)
		fprintf(stdout, ",'%c',%i",fs[i],fs[i+1]);
	    else
		fprintf(stdout, ",%i,%i", fs[i],fs[i+1]);
	    n--;
	    i+=2;
	}
    }
    fprintf(stdout, "};\n\ti=1;\n\twhile (*s) {\n"
	    "\t\tfor (j=0;j<c[i+1] && c[i+2+j]!=(unsigned short)*s; j+=2);\n"
	    "\t\ti=c[i+3+j];\n\t\ts++;\n\t}\n\treturn c[i];\n}\n");
}

int main(int argc, char **argv)
{
    int i;
    FSTATE *fs;
    for (i=1; i<argc; i++) {
	fs = make_fstate(argv[i],1);
	fprintf(stdout, "/%c  FINITE STATE CHECK %s %c/\n",'*',argv[i],'*');
	print_cprog_finitestate(fs);
	free_fstate(fs);
    }
    return 0;
}
#endif
