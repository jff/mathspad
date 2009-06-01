#include "unitype.h"
#include "unimap.h"

MapInt  uctype;       /* character types/bidi behaviour */
MapUchar uclow;       /* lowercase */
MapUchar ucup;        /* upcase */
MapUchar uctitle;     /* title case */
MapChar uchexval;     /* hexvalue */
MapChar uccancom;     /* canonical composition class */
MapUstr ucdecomp;     /* decomposition string */
MapChar ucdetag;      /* decomposition information */
MapUchar ucinitial;   /* arabic initial form */
MapUchar ucmedial;    /* arabic medial form */
MapUchar ucfinal;     /* arabic final form */
MapUchar ucisolated;  /* arabic isolated form */
MapUchar ucvalue;     /* value of character */
MapUchar ucdenum;     /* denumerator value, usually 1 */
MapUchar ucspacevar;  /* spacing variant */
MapUchar ucmirror;    /* for bidi algorithm */
MapUchar ucnoaccent;  /* for getting base characters */

void unitype_init(void)
{
  if (!uctype) {
    MapIntLoadFile(uctype,"GenClass");

    MapUcharLoadFile(uclow,"ToLower");
    MapUcharLoadFile(ucup,"ToUpper");
    MapUcharLoadFile(uctitle,"ToTitle");
    MapUcharLoadFile(ucinitial,"ToInitial");
    MapUcharLoadFile(ucmedial,"ToMedial");
    MapUcharLoadFile(ucfinal,"ToFinal");
    MapUcharLoadFile(ucisolated,"ToIsolated");
    MapUcharLoadFile(ucvalue,"ToNum");
    MapUcharLoadFile(ucdenum,"ToDenum");
    MapUcharLoadFile(ucspacevar,"ToSpacing");
    MapUcharLoadFile(ucmirror,"ToMirror");
    MapUcharLoadFile(ucnoaccent,"ToNoAccent");

    MapUstrLoadFile(ucdecomp,"Decomposition");

    MapCharLoadFile(ucdetag,"DeCompTag");
    MapCharLoadFile(uccancom,"CanComClass");
    MapCharLoadFile(uchexval,"ToHexValue");
  }
}

