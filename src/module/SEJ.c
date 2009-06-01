#ifdef SEJ
    {"Test Popup", 14, 0, NULL, popup_misc},
#endif
#ifdef SEJ
static MENU *prevmenu=NULL;
static char *menus[]= {
    "A,B,C,D,-,E",
    "B,C,D,A,-,E",
    "C,D,A,B,-,E",
    "D,A,B,F,-,E",
    NULL,
    "A,B,C,D,E,G,E,D,C,B,A,B,C,D,E,G,E,D,C,B,A,B,C,D,E,G,E,D,C,B,A,B,C,D,E,G,E,D,C,B,A",
    NULL
};

static void echo_string(void *string, int n)
{
    message(n,(char*)string);
}

static void cycle_popup(void *string, int n __attribute__((unused)))
{
    MENU *nm=NULL;
    char buffer[50];
    int i=0;
    char *c;
    if (string) {
	nm=build_menu("Test");
	c=string;
	do {
	    if (!*c || *c==',') {
		buffer[i]=0;
		if (buffer[0]=='-') {
		    add_item(nm, "", NULL, NULL, 0);
		} else if (buffer[0]=='G') {
		    add_item(nm, buffer, echo_string, "Geen G versie",1);
		} else {
		    add_item(nm,buffer, cycle_popup, menus[buffer[0]-'A'], 0);
		}
		i=0;
	    } else {
		buffer[i++]=*c;
	    }
	} while (*c++);
    }
    prevmenu=popup_replace(prevmenu, nm);
}
#endif
#ifdef SEJ
    case 14:
	cycle_popup("A,B,C,D,--,E",0);
	break;
#endif

