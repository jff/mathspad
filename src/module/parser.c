#ifdef PARSER
#include "match.h"
#endif
#ifdef PARSER
    {"Parse",10,0,NULL,popup_misc},
#endif
#ifdef PARSER
    case 10:
	{
	    char *psel=NULL;
	    tex_set_string(&psel);
	    tex_placeholders(ON);
	    tex_mode(MPTEX);
	    latex_selection(1);
	    tex_unset();
	    if (psel && parse_text(psel)) include_selection();
	    cleanup_nodestack();
	    free(psel);
	}
	break;	
#endif
