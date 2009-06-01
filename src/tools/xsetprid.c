#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int main(int argc, char **argv)
{
    Display *display;
    int i;
    Atom pridat,textat;
    char *dpname="";
    char *propname="PROCESSID";
    char *pidstr="";
    Window testwin=0;
    for (i=1; i<argc; i++) {
	if (argv[i][0]=='-') {
            switch (argv[i][1]) {
            case 'P':  propname = argv[i+1]; i++; break;
            case 'd':
                if (!strcmp(argv[i]+2, "isplay")) {
                    dpname = argv[i+1];
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
        } else {
	    pidstr=argv[i];
	}
    }
    if ((display=XOpenDisplay(dpname))==NULL) {
	fprintf(stderr, "%s: Can not connect to server %s\n",
		argv[0], XDisplayName(dpname));
	exit(-1);
    }
    i=DefaultScreen(display);
    textat=XInternAtom(display,"TEXT",True);
    pridat=XInternAtom(display,propname, False);
    if (!testwin) testwin=RootWindow(display, i);
    XChangeProperty(display,testwin, pridat, textat, 8,
		    PropModeReplace,pidstr,strlen(pidstr));
    XFlush(display);
    XCloseDisplay(display);
    return 0;
}
