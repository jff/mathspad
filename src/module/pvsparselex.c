#include <stdio.h>
#include <stdlib.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# ifndef YYLMAX 
# define YYLMAX BUFSIZ
# endif 
#ifndef __cplusplus
# define output(c) (void)putc(c,yyout)
#else
# define lex_output(c) (void)putc(c,yyout)
#endif

#if defined(__cplusplus) || defined(__STDC__)

#if defined(__cplusplus) && defined(__EXTERN_C__)
extern "C" {
#endif
	int yyback(int *, int);
	int yyinput(void);
	int yylook(void);
	void yyoutput(int);
	int yyracc(int);
	int yyreject(void);
	void yyunput(int);
	int yylex(void);
#ifdef YYLEX_E
	void yywoutput(wchar_t);
	wchar_t yywinput(void);
#endif
#ifndef yyless
	int yyless(int);
#endif
#ifndef yywrap
	int yywrap(void);
#endif
#ifdef LEXDEBUG
	void allprint(char);
	void sprint(char *);
#endif
#if defined(__cplusplus) && defined(__EXTERN_C__)
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
	void exit(int);
#ifdef __cplusplus
}
#endif

#endif
# define unput(c) {yytchar= (c);if(yytchar=='\n')yylineno--;*yysptr++=yytchar;}
# define yymore() (yymorfg=1)
#ifndef __cplusplus
# define input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#else
# define lex_input() (((yytchar=yysptr>yysbuf?U(*--yysptr):getc(yyin))==10?(yylineno++,yytchar):yytchar)==EOF?0:yytchar)
#endif
#define ECHO fprintf(yyout, "%s",yytext)
# define REJECT { nstr = yyreject(); goto yyfussy;}
int yyleng;
#define YYISARRAY
char yytext[YYLMAX];
int yymorfg;
extern char *yysptr, yysbuf[];
int yytchar;
FILE *yyin = 0, *yyout = 0;
extern int yylineno;
struct yysvf { 
	struct yywork *yystoff;
	struct yysvf *yyother;
	int *yystops;};
struct yysvf *yyestate;
extern struct yysvf yysvec[], *yybgin;
 #undef input
 #undef unput
 #undef output
 #undef yywrap
 #define input() str_input()
 #define unput(c) str_unput(c)
 #define output(c) str_output(c)
 #define yywrap()  str_wrapup()
# define YYNEWLINE 10
yylex(){
int nstr; extern int yyprevious;
#ifdef __cplusplus
/* to avoid CC and lint complaining yyfussy not being used ...*/
static int __lex_hack = 0;
if (__lex_hack) goto yyfussy;
#endif
while((nstr = yylook()) >= 0)
yyfussy: switch(nstr){
case 0:
if(yywrap()) return(0); break;
case 1:

# line 10 "pvsparse.lex"
    { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(compose_uid),0); */
            lastchar=COMPOSE;
            return (COMPOSE); }
break;
case 2:

# line 14 "pvsparse.lex"
   { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(times_uid),0); */
            lastchar=TIMES;
            return (TIMES); }
break;
case 3:

# line 18 "pvsparse.lex"
       { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(plus_uid),0); */
            lastchar=PLUS;
            return (PLUS); }
break;
case 4:

# line 22 "pvsparse.lex"
       { /* Char buffer[1];
            make_node(MP_Op,buffer,0,notation_with_number(equal_uid),0); */
            lastchar=EQUAL;
            return (EQUAL); }
break;
case 5:

# line 26 "pvsparse.lex"
      { lastchar=AND;
	    return (AND); }
break;
case 6:

# line 28 "pvsparse.lex"
{ Char buffer[512];int len;
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
break;
case 7:

# line 42 "pvsparse.lex"
case 8:

# line 43 "pvsparse.lex"
  { Char buffer[512]; int len;
		      for (len=0;
			   len<yyleng && len<511 && yytext[len]!='!';
			   len++) {
			buffer[len]=yytext[len];
		      }
		      buffer[len]=0;
		      make_node(MP_Id,buffer,len,0,0);
		      lastchar=IDENTIFIER;
                      return (IDENTIFIER); }
break;
case 9:

# line 53 "pvsparse.lex"
       { if (lastchar==')') {
               /* Char buffer[1];
               make_node(MP_Op,buffer,0,notation_with_number(apply_uid),0);
               */
               lastchar=(APPLY);
	       unput('(');
               return (APPLY);
             }
             lastchar='('; return '(';
           }
break;
case 10:

# line 63 "pvsparse.lex"
{ lastchar=')'; return ')'; }
break;
case 11:

# line 64 "pvsparse.lex"
   ;
break;
case 12:

# line 65 "pvsparse.lex"
{ lastchar=yytext[0]; return yytext[0]; }
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

6,
0, 

6,
0, 

12,
0, 

11,
12,
0, 

11,
0, 

11,
12,
0, 

12,
0, 

9,
12,
0, 

10,
12,
0, 

3,
12,
0, 

7,
12,
0, 

4,
12,
0, 

6,
12,
0, 

6,
12,
0, 

8,
0, 

7,
0, 

6,
0, 

6,
0, 

2,
0, 

1,
0, 

5,
6,
0, 
0};
# define YYTYPE unsigned char
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	1,6,	1,7,	15,22,	
16,23,	0,0,	0,0,	0,0,	
0,0,	1,8,	1,9,	0,0,	
1,10,	0,0,	0,0,	0,0,	
0,0,	1,11,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	6,15,	0,0,	0,0,	
0,0,	0,0,	1,12,	0,0,	
0,0,	0,0,	1,13,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
14,20,	20,20,	24,20,	0,0,	
0,0,	0,0,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
1,14,	1,14,	1,14,	1,14,	
2,6,	2,7,	6,16,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	2,10,	
7,17,	7,17,	7,17,	7,17,	
7,17,	7,17,	7,17,	7,17,	
7,17,	7,17,	21,24,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	2,12,	0,0,	0,0,	
21,20,	0,0,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	2,14,	
2,14,	2,14,	2,14,	11,18,	
11,18,	11,18,	11,18,	11,18,	
11,18,	11,18,	11,18,	11,18,	
11,18,	13,19,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,21,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	13,20,	
13,20,	13,20,	13,20,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		yyvstop+1,
yycrank+-92,	yysvec+1,	yyvstop+3,
yycrank+0,	0,		yyvstop+5,
yycrank+0,	0,		yyvstop+7,
yycrank+0,	0,		yyvstop+10,
yycrank+15,	0,		yyvstop+12,
yycrank+88,	0,		yyvstop+15,
yycrank+0,	0,		yyvstop+17,
yycrank+0,	0,		yyvstop+20,
yycrank+0,	0,		yyvstop+23,
yycrank+167,	0,		yyvstop+26,
yycrank+0,	0,		yyvstop+29,
yycrank+192,	0,		yyvstop+32,
yycrank+14,	yysvec+13,	yyvstop+35,
yycrank+3,	0,		0,	
yycrank+4,	0,		0,	
yycrank+0,	yysvec+7,	yyvstop+38,
yycrank+0,	yysvec+11,	yyvstop+40,
yycrank+0,	yysvec+7,	0,	
yycrank+15,	yysvec+13,	yyvstop+42,
yycrank+78,	yysvec+13,	yyvstop+44,
yycrank+0,	0,		yyvstop+46,
yycrank+0,	0,		yyvstop+48,
yycrank+16,	yysvec+13,	yyvstop+50,
0,	0,	0};
struct yywork *yytop = yycrank+314;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,   1,   1,   1,   1,   1,   1, 
 40,  41,   1,   1,   1,   1,   1,   1, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 48,  48,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,  65,  65,  65,  65,  65, 
 65,  65,  65,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
0};
char yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#pragma ident	"@(#)ncform	6.11	97/01/06 SMI"

int yylineno =1;
# define YYU(x) x
# define NLSTATE yyprevious=YYNEWLINE
struct yysvf *yylstate [YYLMAX], **yylsp, **yyolsp;
char yysbuf[YYLMAX];
char *yysptr = yysbuf;
int *yyfnd;
extern struct yysvf *yyestate;
int yyprevious = YYNEWLINE;
#if defined(__cplusplus) || defined(__STDC__)
int yylook(void)
#else
yylook()
#endif
{
	register struct yysvf *yystate, **lsp;
	register struct yywork *yyt;
	struct yysvf *yyz;
	int yych, yyfirst;
	struct yywork *yyr;
# ifdef LEXDEBUG
	int debug;
# endif
	char *yylastch;
	/* start off machines */
# ifdef LEXDEBUG
	debug = 0;
# endif
	yyfirst=1;
	if (!yymorfg)
		yylastch = yytext;
	else {
		yymorfg=0;
		yylastch = yytext+yyleng;
		}
	for(;;){
		lsp = yylstate;
		yyestate = yystate = yybgin;
		if (yyprevious==YYNEWLINE) yystate++;
		for (;;){
# ifdef LEXDEBUG
			if(debug)fprintf(yyout,"state %d\n",yystate-yysvec-1);
# endif
			yyt = yystate->yystoff;
			if(yyt == yycrank && !yyfirst){  /* may not be any transitions */
				yyz = yystate->yyother;
				if(yyz == 0)break;
				if(yyz->yystoff == yycrank)break;
				}
#ifndef __cplusplus
			*yylastch++ = yych = input();
#else
			*yylastch++ = yych = lex_input();
#endif
#ifdef YYISARRAY
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
#else
			if (yylastch >= &yytext[ yytextsz ]) {
				int	x = yylastch - yytext;

				yytextsz += YYTEXTSZINC;
				if (yytext == yy_tbuf) {
				    yytext = (char *) malloc(yytextsz);
				    memcpy(yytext, yy_tbuf, sizeof (yy_tbuf));
				}
				else
				    yytext = (char *) realloc(yytext, yytextsz);
				if (!yytext) {
				    fprintf(yyout,
					"Cannot realloc yytext\n");
				    exit(1);
				}
				yylastch = yytext + x;
			}
#endif
			yyfirst=0;
		tryagain:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"char ");
				allprint(yych);
				putchar('\n');
				}
# endif
			yyr = yyt;
			if ( (int)yyt > (int)yycrank){
				yyt = yyr + yych;
				if (yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
# ifdef YYOPTIM
			else if((int)yyt < (int)yycrank) {		/* r < yycrank */
				yyt = yyr = yycrank+(yycrank-yyt);
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"compressed state\n");
# endif
				yyt = yyt + yych;
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transitions */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				yyt = yyr + YYU(yymatch[yych]);
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"try fall back character ");
					allprint(YYU(yymatch[yych]));
					putchar('\n');
					}
# endif
				if(yyt <= yytop && yyt->verify+yysvec == yystate){
					if(yyt->advance+yysvec == YYLERR)	/* error transition */
						{unput(*--yylastch);break;}
					*lsp++ = yystate = yyt->advance+yysvec;
					if(lsp > &yylstate[YYLMAX]) {
						fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
						exit(1);
					}
					goto contin;
					}
				}
			if ((yystate = yystate->yyother) && (yyt= yystate->yystoff) != yycrank){
# ifdef LEXDEBUG
				if(debug)fprintf(yyout,"fall back to state %d\n",yystate-yysvec-1);
# endif
				goto tryagain;
				}
# endif
			else
				{unput(*--yylastch);break;}
		contin:
# ifdef LEXDEBUG
			if(debug){
				fprintf(yyout,"state %d char ",yystate-yysvec-1);
				allprint(yych);
				putchar('\n');
				}
# endif
			;
			}
# ifdef LEXDEBUG
		if(debug){
			fprintf(yyout,"stopped at %d with ",*(lsp-1)-yysvec-1);
			allprint(yych);
			putchar('\n');
			}
# endif
		while (lsp-- > yylstate){
			*yylastch-- = 0;
			if (*lsp != 0 && (yyfnd= (*lsp)->yystops) && *yyfnd > 0){
				yyolsp = lsp;
				if(yyextra[*yyfnd]){		/* must backup */
					while(yyback((*lsp)->yystops,-*yyfnd) != 1 && lsp > yylstate){
						lsp--;
						unput(*yylastch--);
						}
					}
				yyprevious = YYU(*yylastch);
				yylsp = lsp;
				yyleng = yylastch-yytext+1;
				yytext[yyleng] = 0;
# ifdef LEXDEBUG
				if(debug){
					fprintf(yyout,"\nmatch ");
					sprint(yytext);
					fprintf(yyout," action %d\n",*yyfnd);
					}
# endif
				return(*yyfnd++);
				}
			unput(*yylastch);
			}
		if (yytext[0] == 0  /* && feof(yyin) */)
			{
			yysptr=yysbuf;
			return(0);
			}
#ifndef __cplusplus
		yyprevious = yytext[0] = input();
		if (yyprevious>0)
			output(yyprevious);
#else
		yyprevious = yytext[0] = lex_input();
		if (yyprevious>0)
			lex_output(yyprevious);
#endif
		yylastch=yytext;
# ifdef LEXDEBUG
		if(debug)putchar('\n');
# endif
		}
	}
#if defined(__cplusplus) || defined(__STDC__)
int yyback(int *p, int m)
#else
yyback(p, m)
	int *p;
#endif
{
	if (p==0) return(0);
	while (*p) {
		if (*p++ == m)
			return(1);
	}
	return(0);
}
	/* the following are only used in the lex library */
#if defined(__cplusplus) || defined(__STDC__)
int yyinput(void)
#else
yyinput()
#endif
{
#ifndef __cplusplus
	return(input());
#else
	return(lex_input());
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyoutput(int c)
#else
yyoutput(c)
  int c; 
#endif
{
#ifndef __cplusplus
	output(c);
#else
	lex_output(c);
#endif
	}
#if defined(__cplusplus) || defined(__STDC__)
void yyunput(int c)
#else
yyunput(c)
   int c; 
#endif
{
	unput(c);
	}
