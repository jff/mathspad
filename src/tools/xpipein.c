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
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <X11/Xlib.h>

typedef struct STRSTACK STRSTACK;
struct STRSTACK {
    char *str;
    int len;
    STRSTACK *next;
};

STRSTACK *beginstack = NULL;
STRSTACK *endstack = NULL;
int totlen=0;
int propempty=1;
int other_prog_ex=0;
Display *display;
char *propname="MPINPUT";
char *endname="MPINTEST";
char *displayname="";
int minsize=1,maxsize=0x7fffffff;
int allocsize=1000;
Window testwin=0;
Atom dummy,testend,textprop;

/* INPUTTIME:    alarm time when there is still unflushed input (short)
** NOINPUTTIME:  alarm time when there is no input available    (long)
** (both in seconds >0 )
*/
#define INPUTTIME 1
#define NOINPUTTIME 10

static void check_property(void)
{
    Atom actt;
    int actf;
    unsigned long nit, baf;
    unsigned char *prp;

    XGetWindowProperty(display, testwin, testend, 0, 512, False, textprop,
		       &actt, &actf, &nit, &baf, &prp);
    other_prog_ex |= (nit==3 && !strcmp((char*)prp, "Yes"));
    XFree(prp);
}

static int error_handler(Display *d __attribute__((unused)), XErrorEvent *err __attribute__((unused)))
{
    exit(0); return 0; /* keeps compiler quite */
}

static int ioerror_handler(Display *d __attribute__((unused)))
{
    exit(0); return 0; /* keeps compiler quite */
}

static void server_init(void)
{
    int i;
    XSetWindowAttributes winattr;

    if ((display=XOpenDisplay(displayname))==NULL) {
	fprintf(stderr, "xpipein: Cannot connect to server %s\n",
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
    XDeleteProperty(display,testwin,dummy);
    check_property();
    propempty=1;
}

static void set_property(int size)
{
    STRSTACK *h;
    char *info=NULL;
    char *c,*d;
    int i=0,j=0;
    c = info = (char*) malloc(size*sizeof(char));
    h = beginstack;
    while (h && j<size) {
	d = h->str;
	i = h->len;
	while (i && j<size) {
	    i--;j++;
	    *c++=*d++;
	}
	if (!i) {
	    free(h->str);
	    beginstack = h->next;
	    free(h);
	    h = beginstack;
	    if (!h) endstack=NULL;
	} else {
	    c=h->str;
	    h->len=i;
	    while (i) {
		i--;
		*c++=*d++;
	    }
	}
    }
    totlen=totlen-j;
    XChangeProperty(display,testwin,dummy,textprop,8,
		    PropModeAppend,(unsigned char*)info,j);
    XFlush(display);
    propempty=0;
    free(info);
}

FILE *f;

int iosig_on=1;
int alarm_needed=NOINPUTTIME;
int oldfileflag=0;
int filenof=0;

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

XEvent event;

static void check_event(void)
{
    if (event.type==PropertyNotify &&
	event.xproperty.atom==dummy &&
	event.xproperty.state==PropertyDelete)
	propempty=1;
    else if (event.type==PropertyNotify &&
	     event.xproperty.atom==testend &&
	     event.xproperty.state==PropertyNewValue)
	check_property();
    else if (event.type==PropertyNotify &&
	     event.xproperty.atom==testend &&
	     event.xproperty.state==PropertyDelete) {
	XDeleteProperty(display, testwin, testend);
	XCloseDisplay(display);
	exit(0);
    } else if (event.type==DestroyNotify &&
	       event.xdestroywindow.window==testwin)
	exit(0);
}

static void check_events(void)
{
    int n=1,i;
 
    while (n) {
	i=n=XEventsQueued(display, QueuedAfterFlush);
	while (i) {
	    i--;
	    XNextEvent(display, &event);
	    check_event();
	}
    }
}

static void read_input(void)
{
    int i,c;
    i=c=0;
    fcntl(filenof,F_SETFL,oldfileflag|NONBLOCKFLAGS);
    while (i==c) {
	if (!endstack) {
	    beginstack=endstack=(STRSTACK*) malloc(sizeof(STRSTACK));
	    endstack->str=(char*) malloc(sizeof(char)*allocsize);
	    endstack->len=0;
	    endstack->next=NULL;
	} else if (allocsize-endstack->len < 100) {
	    endstack->next = (STRSTACK*) malloc(sizeof(STRSTACK));
	    endstack=endstack->next;
	    endstack->str=(char*) malloc(sizeof(char)*allocsize);
	    endstack->len=0;
	    endstack->next=0;
	}
	c=allocsize-endstack->len;
	errno=0;
	i= fread(endstack->str+endstack->len, 1, c, f);
	if (errno) clearerr(f);
	if (i>0) {
	    endstack->len+=i;
	    totlen+=i;
	}
    }
    fcntl(filenof,F_SETFL,oldfileflag);
    errno=0;
}

static void get_and_set(void)
{
    iosig_on=0;
    read_input();
    check_events();
    if (propempty && other_prog_ex && totlen>=minsize)
	set_property(totlen<maxsize?totlen:maxsize);
    if (alarm_needed) {
	if (totlen>=minsize) {
	    alarm_needed=INPUTTIME;
	} else alarm_needed=NOINPUTTIME;
	alarm(alarm_needed);
    }
    iosig_on=1;
}

static void input_handle(int sig __attribute__((unused)))
{
    signal(sig, input_handle);
    if (iosig_on) get_and_set();
    errno=0;
}

static void alarm_handle(int sig __attribute__((unused)))
{
    signal(sig, alarm_handle);
    if (alarm_needed && iosig_on) get_and_set();
    errno=0;
}

static void handle(int sig __attribute__((unused)))
{
    set_property(0);
    XDeleteProperty(display, testwin, testend);
    XCloseDisplay(display);
    exit(0);
}

int main(int argc, char **argv)
{
    int i=1;
    int endprop=0;
    f = stdin;
    filenof=fileno(f);
    oldfileflag=fcntl(filenof,F_GETFL);
    for (i=1; i<argc; i++) {
	if (argv[i][0]=='-') {
	    switch (argv[i][1]) {
	    case 'T':  propname = argv[i+1]; i++; break;
	    case 'E':  endname = argv[i+1]; i++; break;
	    case 'm':
		if (argv[i][2]=='i') {
		    minsize = strtol(argv[i+1],NULL, 0);
		    if (minsize<1) minsize=1;
		    i++;
		} else if (argv[i][2]=='a') {
		    maxsize = strtol(argv[i+1],NULL,0);
		    if (maxsize<minsize) maxsize=minsize;
		    i++;
		}
		break;
	    case 'a':
	        allocsize=strtol(argv[i+1],NULL,0);
		if (allocsize<32) allocsize=32;
		i++;
		break;
	    case 'd':
		if (!strcmp(argv[i]+2, "isplay")) {
		    displayname = argv[i+1];
		    i++;
		}
		break;
	    case 'w':
		if (!strcmp(argv[i]+2,"indow")) {
		    testwin = strtol(argv[i+1],NULL,0);
		    i++;
		}
		break;
	    default: break;
	    }
	}
    }
    server_init();
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
    signal(SIGALRM, alarm_handle);
#endif
#ifdef SIGTERM
    signal(SIGTERM, handle);
#endif
#ifdef SIGURG
    signal(SIGURG,  input_handle);
#endif
#ifdef SIGIO
    signal(SIGIO,  input_handle);
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
#ifdef SIGPOLL
    signal(SIGPOLL, input_handle);
#endif

    /*
    **  How it works:
    **  - use an alarm for the Xevents. The Xevents are not critical and
    **    the alarm is only used to check if the window is still there.
    **  - while there is input available:
    **    - block in a fgetc call
    **    - put the character back if no error occured
    **      (alarm might have removed the block)
    **    - read as much as possible with non-blocking reads.
    **    - check all available Xevents
    **    - put the input in an Xproperty if possible
    **  - after all the input is processed, wait until all the input is
    **    piped through the Xproperties. (normal Xevent loop)
    **
    */
    get_and_set();
    alarm(alarm_needed);
    while (!feof(f)) {
	errno=0;
	i=fgetc(f);
	if (i!=EOF && !errno) {
	    ungetc(i,f);
	    get_and_set();
	}
    }
    /* no input available, wait for Xevents until all input is pushed */
    alarm_needed=0;
    while (!endprop) {
	XNextEvent(display, &event);
	check_event();
	if (propempty && other_prog_ex) {
	    endprop = (totlen==0);
	    set_property(totlen<maxsize?totlen:maxsize);
	}
    }
    XDeleteProperty(display, testwin, testend);
    XCloseDisplay(display);
    return 0;
}
