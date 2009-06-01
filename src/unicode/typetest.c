#include <stdlib.h>
#include <stdio.h>
#include "unitype.h"

int main(int argc, char **argv)
{
    int i,j;
    Uchar c;
    unitype_init();

    for (i=1; i<argc; i++) {
	sscanf(argv[i], "%i",&j);
	if (!j) j=argv[i][0];
	c=j;
	printf("%.4x\t%.8x\n",c, MapValue(uctype,c));
        if (Uisalpha(c)) { printf("\tisalpha\n");}
        if (Uisupper(c)) { printf("\tisupper\n");}
        if (Uislower(c)) { printf("\tislower\n");}
        if (Uistitle(c)) { printf("\tistitle\n");}
        if (Uismodifier(c)) { printf("\tismodifier\n");}
        if (Uisdigit(c)) { printf("\tisdigit\n");}
        if (Uisspace(c)) { printf("\tisspace\n");}
        if (Uispunct(c)) { printf("\tispunct\n");}
        if (Uisalnum(c)) { printf("\tisalnum\n");}
        if (Uissymbol(c)) { printf("\tissymbol\n");}
        if (Uisprint(c)) { printf("\tisprint\n");}
        if (Uisgraph(c)) { printf("\tisgraph\n");}
        if (Uiscntrl(c)) { printf("\tiscntrl\n");}
        if (Uiscombining(c)) { printf("\tiscombining\n");}
        if (Uisnonspacing(c)) { printf("\tisnonspacing\n");}
        if (Uismath(c)) { printf("\tismath\n");}
        if (Uiscurrency(c)) { printf("\tiscurrency\n");}
        if (Uisopen(c)) { printf("\tisopen\n");}
        if (Uisclose(c)) { printf("\tisclose\n");}
        if (Uismirrored(c)) { printf("\tismirrored\n");}
        if (Uisleftright(c)) { printf("\tisleftright\n");}
        if (Uisrightleft(c)) { printf("\tisrightleft\n");}
        if (Uiseuronum(c)) { printf("\tiseuronum\n");}
        if (Uiseurosep(c)) { printf("\tiseurosep\n");}
        if (Uiseuroterm(c)) { printf("\tiseuroterm\n");}
        if (Uisarabnum(c)) { printf("\tisarabnum\n");}
        if (Uiswhite(c)) { printf("\tiswhite\n");}
        if (Uisneutral(c)) { printf("\tisneutral\n");}
        if (Uisprivate(c)) { printf("\tisprivate\n");}
        if (Uissurrogatehigh(c)) { printf("\tissurrogatehigh\n");}
        if (Uissurrogatelow(c)) { printf("\tissurrogatelow\n");}
        if (Uisxdigit(c)) { printf("\tisxdigit\n");}
        printf("\ttosurrogatehigh\t\t%.4x\n",Utosurrogatehigh(c));
        printf("\ttosurrogatelow\t\t%.4x\n",Utosurrogatelow(c));
        printf("\ttolower\t\t%.4x\n",Utolower(c));
        printf("\ttoupper\t\t%.4x\n",Utoupper(c));
        printf("\ttotitle\t\t%.4x\n",Utotitle(c));
        printf("\ttonoaccent\t\t%.4x\n",Utonoaccent(c));
        printf("\ttoinitial\t\t%.4x\n",Utoinitial(c));
        printf("\ttomedial\t\t%.4x\n",Utomedial(c));
        printf("\ttofinal\t\t%.4x\n",Utofinal(c));
        printf("\ttoisolated\t\t%.4x\n",Utoisolated(c));
        printf("\ttovalue\t\t%.4x\n",Utovalue(c));
        printf("\ttofvalue\t\t%.4f\n",Utofvalue(c));
        printf("\ttoxvalue\t\t%.4x\n",Utoxvalue(c));
        printf("\ttomirror\t\t%.4x\n",Utomirror(c));
        printf("\ttospacing\t\t%.4x\n",Utospacing(c));
    }
    return 0;
}
	
