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
/*
**   File: message.c
*/

#include "mathpad.h"
#include "system.h"
#include "funcs.h"
#include "sources.h"
#include "output.h"
#include "remark.h"
#include "message.h"


void message(int lvl, Char *text)
{
    Bool to_standard = MP_False;

    if (menu_is_open) {
	message_time = last_time;
	switch (lvl) {
	case MP_MESSAGE:
	    out_message(text);
	    break;
	case MP_MESSAGECURS:
	    out_message_curs(text);
	    break;
	case MP_ERROR:
	    out_message(text);
	    XBell(display, 0);
	    break;
	case MP_CLICKREMARK:
	    kind_of_remark = LONG_REMARK;
	    to_standard = (!remark_make(0, NULL, NULL, REMARK_CENTRE,
					text, NULL, NULL, 0, NULL));
	    break;
	case MP_KEYREMARK:
	    kind_of_remark = SHORT_REMARK;
	    to_standard = (!remark_make(0, NULL, NULL, REMARK_CENTRE,
					text, NULL, NULL, 0, NULL));
	    break;
	default:   /* MP_EXIT -... */
	    to_standard = MP_True;
	    break;
	}
    } else
	to_standard = MP_True;
    if (to_standard) {
	/*
	**  beep beep
	*/
	fprintf(stderr, "%s: %s\n", UstrtoLocale(progname), UstrtoLocale(text));
	if (lvl>MP_KEYREMARK) {
	    /*
	    **  mogelijk menu_bad_end() of server_close()  (geven zelf exit)
	    */
	    exit(lvl-MP_EXIT);
	}
    }
}

void message2(int lvl, Char *text1, Char *text2)
{
    Char *temp;

    temp = concat(text1, text2);
    message(lvl, temp);
    free(temp);
}
