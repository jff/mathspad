 #undef input
 #undef unput
 #undef output
 #undef yywrap
 #define input() str_input()
 #define unput(c) str_unput(c)
 #define output(c) str_output(c)
 #define yywrap()  str_wrapup()
%%
\ o\      { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(compose_uid),0); */
            lastchar=COMPOSE;
            return (COMPOSE); }
\ \*\     { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(times_uid),0); */
            lastchar=TIMES;
            return (TIMES); }
\+        { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(plus_uid),0); */
            lastchar=PLUS;
            return (PLUS); }
\=        { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(equal_uid),0); */
            lastchar=EQUAL;
            return (EQUAL); }
AND       { lastchar=AND;
	    return (AND); }
[a-zA-Z]* { Char buffer[512];int len;
            if (!strncmp(yytext, "I", yyleng)) {
	      buffer[0]=0; len=0;
	      make_node(MP_Op, buffer,len, notation_with_number(id_uid), 0);
	    } else {
	      for (len=0; len<yyleng && len<511; len++) {
		buffer[len]=yytext[len];
	      }
	      buffer[len]=0;
	      /* copy function name */
	      make_node(MP_Id,buffer,len,0,0);
	    }
            lastchar=FUNCTION;
            return (FUNCTION); }
[0-9]+ |
[a-zA-Z]*\![0-9]+   { Char buffer[512]; int len;
		      for (len=0;
			   len<yyleng && len<511 && yytext[len]!='!';
			   len++) {
			buffer[len]=yytext[len];
		      }
		      buffer[len]=0;
		      make_node(MP_Id,buffer,len,0,0);
		      lastchar=IDENTIFIER;
                      return (IDENTIFIER); }
[(]        { if (lastchar==')') {
               /* Char buffer[1];
               make_node(MP_Op,buffer,0,notation_with_number(apply_uid),0);
               */
               lastchar=(APPLY);
	       unput('(');
               return (APPLY);
             }
             lastchar='('; return '(';
           }
[)] { lastchar=')'; return ')'; }
[ \n\t]    ;
. { lastchar=yytext[0]; return yytext[0]; }

