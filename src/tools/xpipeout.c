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
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <X11/Xlib.h>
#ifndef FNONBLOCK
#define FNONBLOCK 0
#endif
#ifndef FNONBLCK
#define FNONBLCK 0
#endif
#ifndef O_NONBLOCK
#define O_NONBLOCK 0
#endif
#define NONBLOCKFLAGS FNONBLOCK|FNONBLCK|O_NONBLOCK

Display *display;
char *propname="MPOUTPUT";
char *endname="MPOUTTEST";
char *displayname="";
int propempty=0;
Window testwin=0;
Atom dummy,textprop, testend;
FILE *outf;
int outfnr;

#define OUTPUTTIME 1
#define NOOUTPUTTIME 10

int alarm_needed=NOOUTPUTTIME;


static int error_handler(Display *d __attribute__((unused)), XErrorEvent *err __attribute__((unused)))
{
    exit(0); return 0;
}

static int ioerror_handler(Display *d __attribute__((unused)))
{
    exit(0); return 0;
}

static void server_init(void)
{
    int i;
    XSetWindowAttributes winattr;

    if ((display=XOpenDisplay(displayname))==NULL) {
	fprintf(stderr, "xpipeout: Cannot connect to server %s\n",
		XDisplayName(displayname));
	exit(-1);
    }
    i = DefaultScreen(display);
    textprop = XInternAtom(display, "TEXT", True);
    dummy = XInternAtom(display, propname, False);
    testend = XInternAtom(display, endname, False);
    if (!testwin) testwin = RootWindow(display,i);
    winattr.event_mask = PropertyChangeMask | StructureNotifyMask;
    XSetErrorHandler(error_handler);
    XSetIOErrorHandler(ioerror_handler);
    XChangeWindowAttributes(display, testwin, CWEventMask, &winattr);
    XChangeProperty(display, testwin, testend, textprop,8, PropModeReplace,
		    (unsigned char*)"Yes", 3);
}

typedef struct STRSTACK STRSTACK;
struct STRSTACK {
    char *ptr;
    char *str;
    int len;
    STRSTACK *next;
};

STRSTACK *beginstack = NULL;
STRSTACK *endstack = NULL;
int buffersize=0;
int critical=0;

static void nonblock_write(char *c, int b)
{
    int wrres=1;

    /* try to clear the buffer */
    critical=1;
    if (buffersize) {
	do {
	    wrres=write(outfnr, beginstack->str, beginstack->len);
	    if (wrres!=-1) {
		beginstack->len -= wrres;
		beginstack->str += wrres;
		buffersize=buffersize-wrres;
		if (!beginstack->len) {
		    STRSTACK *h=beginstack;
		    free(beginstack->ptr);
		    beginstack=h->next;
		    free(h);
		}
	    }
	} while (wrres!=-1 && beginstack);
    }
    if (!buffersize)
	wrres=write(outfnr, c, b);
    else
	wrres= -1;
    if (wrres>0) {
	c=c+wrres;
	b=b-wrres;
    }
    if (b) {
	/* Unable to write. Add it to the stack. */
	STRSTACK *h;
	h=(STRSTACK *)malloc(sizeof(STRSTACK));
	h->next=NULL;
	h->len=b;
	buffersize+=b;
	h->str=h->ptr=(char*) malloc(b*sizeof(char));
	while (b--) h->str[b]=c[b]; /* or: memcpy(h->str, c, b); */
	if (!beginstack)
	    beginstack=endstack=h;
	else {
	    endstack->next=h;
	    endstack=h;
	}
    }
    critical=0;
    if (alarm_needed) {
	if (buffersize)
	    alarm_needed=OUTPUTTIME;
	else
	    alarm_needed=NOOUTPUTTIME;
	alarm(alarm_needed);
    }
}

static void handle(int sig)
{
    if (sig==SIGIO || sig==SIGURG || sig==SIGALRM) {
	signal(sig, handle);
	if (!critical) nonblock_write("",0);
    } else {
	signal(sig, SIG_DFL);
	XDeleteProperty(display, testwin, testend);
	XCloseDisplay(display);
	exit(0);
    }
}

XEvent event;

int main(int argc, char **argv)
{
    int i;
    int close_con=0;

    outf = stdout;
    outfnr=fileno(outf);
    for (i=1; i<argc; i++) {
	if (argv[i][0]=='-') {
	    switch (argv[i][1]) {
	    case 'T':  propname = argv[i+1]; i++; break;
	    case 'E':  endname = argv[i+1]; i++; break;
	    case 'd':
		if (!strcmp(argv[i]+2, "isplay")) {
		    displayname = argv[i+1];
		    i++;
		}
		break;
	    case 'w':
		if (!strcmp(argv[i]+2,"indow")) {
		    testwin = strtol(argv[i+1], NULL, 0);
		    i++;
		}
		break;
	    default: break;
	    }
	}
    }
    i=1;
    /* set the fatal interrupts to stop correctly */
#ifdef SIGHUP
    signal(SIGHUP,  handle);
#endif
#ifdef SIGINT
    signal(SIGINT,  handle);
#endif
#ifdef SIGQUIT
    signal(SIGQUIT, handle);
#endif
#ifdef SIGILL
    signal(SIGILL,  handle);
#endif
#ifdef SIGABRT
    signal(SIGABRT, handle);
#endif
#ifdef SIGFPE
    signal(SIGFPE,  handle);
#endif
#ifdef SIGBUS
    signal(SIGBUS,  handle);
#endif
#ifdef SIGSEGV
    signal(SIGSEGV, handle);
#endif
#ifdef SIGSYS
    signal(SIGSYS,  handle);
#endif
#ifdef SIGPIPE
    signal(SIGPIPE, handle);
#endif
#ifdef SIGALRM
    signal(SIGALRM, handle);
#endif
#ifdef SIGTERM
    signal(SIGTERM, handle);
#endif
#ifdef SIGURG
    signal(SIGURG,  handle);
#endif
#ifdef SIGIO
    signal(SIGIO,  handle);
#endif
#ifdef SIGXCPU
    signal(SIGXCPU, handle);
#endif
#ifdef SIGXFSZ
    signal(SIGXFSZ, handle);
#endif
#ifdef SIGLOST
    signal(SIGLOST, handle);
#endif
#ifdef SIGUSR1
    signal(SIGUSR1, handle);
#endif
#ifdef SIGUSR2
    signal(SIGUSR2, handle);
#endif
    i=fcntl(outfnr, F_GETFL);
    fcntl(outfnr, F_SETFL, i|NONBLOCKFLAGS);
    server_init();
    alarm(alarm_needed);
    while (!propempty && !close_con) {
	XNextEvent(display, &event);
	if (event.type==PropertyNotify &&
	    event.xproperty.atom==dummy &&
	    event.xproperty.state==PropertyNewValue) {
	    long n=0;
	    long len=512; /* small to be able to write nonblocking */
	    Atom actt;
	    int actf;
	    unsigned long nit,baf=1;
	    unsigned char *prp;
	    while (baf) {
		XGetWindowProperty(display, testwin, dummy, n/4, len, True,
				   textprop, &actt, &actf, &nit, &baf, &prp);
		nonblock_write((char*)prp, nit);
		XFree(prp);
		n=n+nit;
	    }
	    propempty= 0; /* (n==0); */
	} else if (event.type==DestroyNotify &&
		   event.xdestroywindow.window == testwin)
	    close_con=1;
	nonblock_write("",0);
    }
    if (!close_con) XDeleteProperty(display, testwin, testend);
    XCloseDisplay(display);
    fcntl(outfnr, F_SETFL, i);
    alarm_needed=0;
    while (beginstack) {
	write(outfnr,beginstack->str, beginstack->len);
	free(beginstack->str);
	endstack=beginstack;
	beginstack=endstack->next;
	free(endstack);
    }
    return 0;
}
