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
#include <stdio.h>
#include <ctype.h>

char buffer[5000];

int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
    int ng;
    char *c;

    while (fgets(buffer, 5000, stdin)) {
	switch (buffer[0]) {
	case '\n':
	case '@': break;
	case '+':
	case '*':
	case '-': printf("*\n"); fflush(stdout); break;
	case '#':
	case '&':
	case '?':
	    c=buffer+2;
	    while (!isspace(*c)) c++;
	    *c='\0';
	    printf("%s :", buffer+2);
	    c++;
	    ng=0;
	    if (buffer[0]!='#') {
		sscanf(c, "%i", &ng);
		while (!isspace(*c)) c++;
		c++;
	    }
	    while (!isspace(*c)) c++;
	    if (*c && ng) {
		char *h=c;
		while (*c && ng) {
		    c++;
		    while (*c && *c!=',' && *c!='\n') c++;
		    ng--;
		}
		if (*c!=',') { c[1]='\n'; c[2]='\0'; }
		*c++='\0';
		printf("%s", h);
	    }
	    printf(" : %s", c);
	    fflush(stdout);
	    break;
	default:
	    printf("%s", buffer);
	    fflush(stdout);
	    break;
	}
    }
    return 0;
}
