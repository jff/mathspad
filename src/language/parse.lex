 #undef input
 #undef unput
 #undef output
 #undef yywrap
 #define input() layout_input()
 #define unput(c) layout_unput(c)
 #define output(c) layout_output(c)
 #define yywrap() layout_wrapup()
%%
Include		return (INCLUDE);
Input		return (INPUT);
Clear		return (CLEAR);
Layout		return (LAYOUT);
Menu		return (MENU);
Keyboard	return (KEYBOARD);
Translation	return (TRANSLATION);
Function	return (FUNCTION);
Var		return (VARIABLE);
if		return (IF);
elseif		return (ELSEIF);
else		return (ELSE);
fi		return (FI);
do		return (DO);
elsedo		return (ELSEDO);
od		return (OD);
Options		return (OPTIONS);
Pin		return (PIN);
LeftRight	return (LEFTRIGHT);
RightLeft	return (RIGHTLEFT);
Separator	return (SEPARATOR);
Title		return (TITLE);
Default		return (DEFAULT);
Button		return (BUTTON);
Image		return (IMAGE);
Scrollbar	return (SCROLLBAR);
Left		return (LEFT);
Right		return (RIGHT);
Bottom		return (BOTTOM);
Top		return (TOP);
Geometry	return (GEOMETRY);
Type		return (TYPE);
Edit		return (EDIT);
Comment		return (COMMENT);
Program		return (PROGRAM);
Console		return (CONSOLE);
Buffer		return (BUFFER);
Symbol		return (SYMBOL);
Stencil		return (STENCIL);
Define		return (DEFINE);
FindReplace	return (FINDREPLACE);
All		return (ALL);
Shell		return (SHELL);
FileSelect	return (FILESELECT);
Remark		return (REMARK);
\!\=		return (NOTEQUAL);
\<\=		return (LESSEQUAL);
\>\=		return (GREATEREQUAL);
\&\&		return (LOGICAND);
\|\|		return (LOGICOR);
\^\^		return (LOGICXOR);
\:\=		return (ASSIGN);
\.\.\.          return (RANGE);
\-		return (MINUS);
\+		return (ADD);
\*		return (MULTIPLY);
\/		return (DIVIDE);
\%		return (REMAINDER);
\!		return (LOGICNOT);
\~		return (BITNOT);
\@		return (LAZYREF);
\=		return (EQUAL);
\<		return (LESS);
\>		return (GREATER);
\&		return (BITOR);
\|		return (BITAND);
\^		return (BITXOR);
0[0-7]+        { yylval.ival=strtol(yytext,NULL,8); return (INTEGER); }
0[Xx][0-9a-fA-F]+ { yylval.ival=strtol(yytext,NULL,16); return (INTEGER); }
[0-9]+         { yylval.ival=strtol(yytext,NULL,10); return (INTEGER); }
[0-9]+\.[0-9]* |
[0-9]+[Ee][-+]?[0-9]+ |
\.[0-9]+    |
\.[0-9]+[Ee][-+]?[0-9]+ | 
[0-9]+\.[0-9]*[Ee][-+]?[0-9]+ { sscanf(yytext, "%lf", &yylval.rval); return (REAL); }
\"[^"]*		{ int i;
		for (i=yyleng-1; i>=0 && yytext[i]=='\\'; i--);
		if ((yyleng-i)%2) {
			char *conv;
                        int i;
			Uchar *ucon,*slashconv;
			input();
                        conv=(char*) malloc((yyleng+1)*sizeof(char));
                        for (i=0; i<yyleng-1; i++) {
                          conv[i]=yytext[i+1];
                        }
                        conv[i]=0;
                        ucon=LocaletoUstr(conv);
			free(conv);
                        slashconv = (Uchar*)malloc((yyleng+1)*sizeof(Uchar));
			convertslash(slashconv, ucon);
			yylval.utval=slashconv;
			return (STRING);
		} else {
			yymore();
		}
		}
`[^`]*		{ int i;
		for (i=yyleng-1; i>=0 && yytext[i]=='\\'; i--);
		if ((yyleng-i)%2) {
			input();
			convertslashascii(yytext, yytext+1, yyleng-1);
			parse_key(yytext, &yylval.keyval.key,
					  &yylval.keyval.mode);
			return (KEY);
		} else {
			yymore();
		}
		}
'\\?.'    |
'\\x[0-9A-Fa-f]+' |
'\\u[0-9A-Fa-f]+' |
'\\[0-7]+' { Uchar b[40];
                  char bc[40];
                  int i;
                  for (i=0; i<yyleng-2 && i<39;i++) {
                     bc[i]=yytext[i+1];
                  }
                  bc[i]=0;
		  convertslash(b,LocaletoUstr(bc));
		  yylval.ival= b[0];
		  return (INTEGER);
           }
[A-Za-z][A-Za-z0-9_]*	{ Type t;
			  t = lookup_type(yytext);
			  if (t) {
			     yylval.ival=t;
			     return (TYPEVAL);
			  }
			  yylval.tval=ident_buffer[current_id++];
			  if (current_id==MAXIDENTIFIER) current_id=0;
                          strncpy(yylval.tval,yytext, IDENTLENGTH-1);
                          return (IDENTIFIER); }
\/\*[^/]*	{ int i;
		  if (yytext[yyleng-1]=='*') {
		    input();
		  } else {
		    yymore();
		  }
		}
[ \t\n]		;
.		return yytext[0];
