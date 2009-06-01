#ifdef ISPELL
static void make_ispell_popup(char *prp, int len)
{
    MENU ispellmenu;
    int i,j,nw;
    char *c;

    /*
    ** layout of Ispell popup:     Action:
    **
    ** Title:   Incorrect word.
    **          -----------------
    ** Actions: Ignore Once          
    **          Ignore Always      Ispell("@Word") (Accept word)
    **          Add to Dictionary  Ispell("*Word") (Personal dictionary)
    **          Stop               Ispell("#")     (Save personal dictionary)
    **          -----------------
    ** Misses:  Alt. 1             Replace("Word", "Alt. 1");   
    **          .... .             Replace("Word", ".... .");   
    **          Alt. n             Replace("Word", "Alt. n");   
    **          -----------------
    ** Guess:   Guess 1            Ispell("@Guess 1");  (Accept Guess)
    **          ..... .            Ispell("*Guess 1");  (Personal dict.)   
    **          Guess n            
    **          -----------------
    */
    for (i=0,nw=0; i<len; i++)
	nw=nw+(prp[i]==',')+(prp[i]==':');
    
}
#endif


