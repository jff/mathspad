#include <stdlib.h>
#include <stdio.h>
#include "unicode.h"
#include "unistring.h"
#include "unitype.h"
#include "markup.h"

int valid_start_position(Uchar *string)
{
    return (!Uismodifier(*string) && (!Uissurrogatelow(*string)));
}

typedef struct SurroHandler SurroHandler;
struct SurroHandler {
    Uchar highval;
    HandleFunc print;
    HandleFunc handle;
    SurroHandler *next;
};

/* a list is used. Alternative: use an array or hash table */
static SurroHandler *surrolist=NULL;
static SurroHandler *current_handler=NULL;
static Uchar surrohighvalue=0;
/* Could print the name of the character if it is defined. At the moment,
** just print the hex value: S+xxxxx
*/
static Uchar surrobuffer[10];
static char surrobuf[10];

static Uchar *surroprint(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int cb;
    cb=Usurrogate(surrohighvalue,lowvalue);
    sprintf(surrobuf,"S+%.5x", cb);
    for (cb=0; cb<8; cb++) {
	surrobuffer[cb]=surrobuf[cb];
    }
    return surrobuffer;
}
static Uchar defaultsurro[2] = { 0xFFFD,0};
static Uchar *surrohandle(int lowvalue, Attribute *attr, RedirectFunc *func)
{
    int cb;
    cb=Usurrogate(surrohighvalue, lowvalue);
    attr->plane=((cb&0xff0000)>>16);
    defaultsurro[0]=cb&0xffff;
    return defaultsurro;
}

static SurroHandler surrodefhandler = { 0, surroprint, surrohandle, 0};

int add_surrogate_handler(Uchar highvalue, HandleFunc print,
			  HandleFunc handle)
{
    SurroHandler *sh;
    sh=malloc(sizeof(SurroHandler));
    if (!sh) return 0;
    sh->next=surrolist;
    surrolist=sh;
    sh->print=print;
    sh->handle=handle;
    sh->highval=highvalue;
    return 1;
}



int add_markup_handler(MarkupGroup mg, HandleFunc print, HandleFunc handle)
{
    return add_surrogate_handler((Uchar) mg, print, handle);
}

void set_handle(Uchar highvalue)
{
    SurroHandler *sh=surrolist;
    while (sh && sh->highval!=highvalue) sh=sh->next;
    if (sh) {
	current_handler=sh;
    } else {
	current_handler=&surrodefhandler;
    }
    surrohighvalue=highvalue;
}

int high_surro(void)
{
    return surrohighvalue;
}

Uchar *handle_surro(Uchar lowvalue, Attribute *attr, RedirectFunc *func)
{
    if (current_handler) {
	return (*current_handler->handle)(lowvalue, attr, func);
    } else {
	return NULL;
    }
}

Uchar *print_surro(Uchar lowvalue, Attribute *attr, RedirectFunc *func)
{
    if (current_handler) {
	return (*current_handler->print)(lowvalue, attr, func);
    } else {
	return NULL;
    }
}


#include "font_markup.h"
void markup_init(void)
{
    font_markup_init();
}
