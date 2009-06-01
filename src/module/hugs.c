#ifdef HUGS
#include <signal.h>

static void popup_hugs(void *data, int n);
static void popup_hugssignal(void *data, int n);
static void popup_hugsoption(void *data, int n);
#endif
#if HUGS
    {"Hugs",11,1,NULL,popup_misc},
#endif
#if HUGS
    {"Start Hugs", 1, 0,NULL,popup_hugs},
    {"Type of target", 2,0,NULL,popup_hugs},
    {"Info on identifier", 10,0,NULL,popup_hugs},
    {"Clear State", 3,0,NULL,popup_hugs},
    {"Eval target", 4,0,NULL,popup_hugs},
    {"Load file ...", 5,0,NULL,popup_hugs},
    {"Load project ...", 12, 0,NULL, popup_hugs},
    {"Reload last file", 6,0,NULL,popup_hugs},
    {"Add file ...", 7,0,NULL,popup_hugs},
    {"Collect Garbage", 9,0,NULL,popup_hugs},
    {"Send signal", 0,1, NULL, popup_hugssignal},
    {"Set option", 0,1, NULL, popup_hugsoption},
    {"Quit", 8, 0, NULL, popup_hugs},
    {NULL,0,0,NULL,NULL},
    {"Alarm", SIGALRM, 0,NULL,popup_hugssignal},
    {"Interrupt", SIGINT, 0,NULL, popup_hugssignal},
    {NULL,0,0,NULL,NULL},
    {"Statistics on", 1,0,NULL,popup_hugsoption},
    {"Statistics off", 2,0,NULL,popup_hugsoption},
    {"Types on", 3,0,NULL,popup_hugsoption},
    {"Types off", 4,0,NULL,popup_hugsoption},
    {"Error Terminate on", 5,0,NULL,popup_hugsoption},
    {"Error Terminate off", 6,0,NULL,popup_hugsoption},
    {"GC message on", 7,0,NULL,popup_hugsoption},
    {"GC message off", 8,0,NULL,popup_hugsoption},
    {"Kind Errors on", 9,0,NULL,popup_hugsoption},
    {"Kind Errors off", 10,0,NULL,popup_hugsoption},
    {"Show on", 11,0,NULL,popup_hugsoption},
    {"Show off", 12,0,NULL,popup_hugsoption},
    {NULL,0,0,NULL,NULL},
#endif
#ifdef HUGS
extern unsigned char *parse_error(unsigned char **s);
extern void hugsparse_init(void);
static int hugsparse(unsigned char *text, unsigned int *num)
{
    unsigned char *c,*d,*errs;
    unsigned int i=0,state=1;
    if (!text) return 0;
    while (i< *num) {
	if (text[i]=='\n') {
	    state=1;
	} else if (state==1) {
	    if (text[i]=='1') {
		errs=text+i;
		c=d=text+i;
		while (isdigit(*c) || isupper(*c)) c++;
		if (*c=='[') {
		    do {
		      c=parse_error(&d);
		    } while (c[0]=='!' && *d!='\0'); 
		    if (c[0]=='?') {
		        string_to_window("Hugs Messages", (char*)(c+1));
		    } else if (c[0]=='#') {
			structure_to_window("Hugs Messages");
			cleanup_nodestack();
		    } else {
			message(MP_CLICKREMARK, (char*)c);
		    }
		    c=errs;
		    *num=*num-(d-c);
		    while (aig(*c++=*d++));i--;
		}
	    }
	    state=0;
	}
	i++;
    }
    return 0;
}

static int hugstemplates=0;
void start_hugs(void)
{
    if (!hugstemplates) {
	char *buffer;
	buffer=concat("/home/rcb5/mathpad/stencils/hugslink.mps","");
	message(MP_MESSAGE, "Loading Hugslink templates ...");
	hugstemplates = new_notation_window();
	hugstemplates = load_notation_window(hugstemplates, buffer);
	message(MP_MESSAGE, "Loading Hugslink templates done.");
    }
    hugsparse_init();
    open_program("hugsscript %i &", "Hugs Shell", hugsparse);
}
#endif

#ifdef HUGS
    case 11:
	start_hugs();
	break;
#endif
#ifdef HUGS

static void hugs_load(void *data __attribute__((unused)), char *c)
{
    char buffer[1000];
    sprintf(buffer, ":l %s\n",c);
    edit_string_to_proces(buffer, "Hugs Shell");
}

static void hugs_load_proj(void *data __attribute__((unused)), char *c)
{
    char buffer[1000];
    sprintf(buffer, ":p %s\n",c);
    edit_string_to_proces(buffer, "Hugs Shell");
}

static void hugs_add(void *data __attribute__((unused)), char *c)
{
    char buffer[1000];
    sprintf(buffer, ":a %s\n",c);
    edit_string_to_proces(buffer, "Hugs Shell");
}

static void popup_hugs(void *data __attribute__((unused)), int n)
{
    switch (n) {
    case 1: start_hugs(); break;
    case 2: edit_string_to_proces(":t (%1)\n", "Hugs Shell"); break;
    case 3: edit_string_to_proces(":l\n", "Hugs Shell"); break;
    case 4: edit_string_to_proces("(%1)\n", "Hugs Shell"); break;
    case 5:
	fileselc_open(hugs_load, NULL,
		      "Load file in Hugs session",
		      "$HUGSDIR", "*.hs", "", menuwin);
	break;
    case 12:
	fileselc_open(hugs_load_proj, NULL,
		      "Load project in Hugs session",
		      "$HUGSDIR", "*.prj", "", menuwin);
	break;
    case 6: edit_string_to_proces(":r\n", "Hugs Shell"); break;
    case 7:
	fileselc_open(hugs_add, NULL,
		      "Add file to Hugs session",
		      "$HUGSDIR", "*.hs", "", menuwin);
	break;
    case 8: edit_string_to_proces(":q\n", "Hugs Shell"); break;
    case 9: edit_string_to_proces(":gc\n", "Hugs Shell"); break;
    case 10: edit_string_to_proces(":i %1\n", "Hugs Shell"); break;
    case 11: edit_signal_to_proces(2,"Hugs Shell");
    default: break;
    }
}

static void popup_hugssignal(void *data __attribute__((unused)), int n)
{
    edit_signal_to_proces(n,"Hugs Shell");
}

static void popup_hugsoption(void *data __attribute__((unused)), int n)
{
    switch (n) {
    case 1:  edit_string_to_proces(":set +s\n", "Hugs Shell"); break;
    case 2:  edit_string_to_proces(":set -s\n", "Hugs Shell"); break;
    case 3:  edit_string_to_proces(":set +t\n", "Hugs Shell"); break;
    case 4:  edit_string_to_proces(":set -t\n", "Hugs Shell"); break;
    case 5:  edit_string_to_proces(":set +f\n", "Hugs Shell"); break;
    case 6:  edit_string_to_proces(":set -f\n", "Hugs Shell"); break;
    case 7:  edit_string_to_proces(":set +g\n", "Hugs Shell"); break;
    case 8:  edit_string_to_proces(":set -g\n", "Hugs Shell"); break;
    case 9:  edit_string_to_proces(":set +k\n", "Hugs Shell"); break;
    case 10: edit_string_to_proces(":set -k\n", "Hugs Shell"); break;
    case 11: edit_string_to_proces(":set +u\n", "Hugs Shell"); break;
    case 12: edit_string_to_proces(":set -u\n", "Hugs Shell"); break;
    }
}
#endif
