#include <stdio.h>
# define U(x) x
# define NLSTATE yyprevious=YYNEWLINE
# define BEGIN yybgin = yysvec + 1 +
# define INITIAL 0
# define YYLERR yysvec
# define YYSTATE (yyestate-yysvec-1)
# define YYOPTIM 1
# define YYLMAX BUFSIZ
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
 #define input() layout_input()
 #define unput(c) layout_unput(c)
 #define output(c) layout_output(c)
 #define yywrap() layout_wrapup()
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

# line 10 "parse.lex"
	return (INCLUDE);
break;
case 2:

# line 11 "parse.lex"
	return (INPUT);
break;
case 3:

# line 12 "parse.lex"
	return (CLEAR);
break;
case 4:

# line 13 "parse.lex"
	return (LAYOUT);
break;
case 5:

# line 14 "parse.lex"
	return (MENU);
break;
case 6:

# line 15 "parse.lex"
return (KEYBOARD);
break;
case 7:

# line 16 "parse.lex"
return (TRANSLATION);
break;
case 8:

# line 17 "parse.lex"
return (FUNCTION);
break;
case 9:

# line 18 "parse.lex"
	return (VARIABLE);
break;
case 10:

# line 19 "parse.lex"
	return (IF);
break;
case 11:

# line 20 "parse.lex"
	return (ELSEIF);
break;
case 12:

# line 21 "parse.lex"
	return (ELSE);
break;
case 13:

# line 22 "parse.lex"
	return (FI);
break;
case 14:

# line 23 "parse.lex"
	return (DO);
break;
case 15:

# line 24 "parse.lex"
	return (ELSEDO);
break;
case 16:

# line 25 "parse.lex"
	return (OD);
break;
case 17:

# line 26 "parse.lex"
	return (OPTIONS);
break;
case 18:

# line 27 "parse.lex"
	return (PIN);
break;
case 19:

# line 28 "parse.lex"
return (LEFTRIGHT);
break;
case 20:

# line 29 "parse.lex"
return (RIGHTLEFT);
break;
case 21:

# line 30 "parse.lex"
return (SEPARATOR);
break;
case 22:

# line 31 "parse.lex"
	return (TITLE);
break;
case 23:

# line 32 "parse.lex"
	return (DEFAULT);
break;
case 24:

# line 33 "parse.lex"
	return (BUTTON);
break;
case 25:

# line 34 "parse.lex"
	return (IMAGE);
break;
case 26:

# line 35 "parse.lex"
return (SCROLLBAR);
break;
case 27:

# line 36 "parse.lex"
	return (LEFT);
break;
case 28:

# line 37 "parse.lex"
	return (RIGHT);
break;
case 29:

# line 38 "parse.lex"
	return (BOTTOM);
break;
case 30:

# line 39 "parse.lex"
	return (TOP);
break;
case 31:

# line 40 "parse.lex"
return (GEOMETRY);
break;
case 32:

# line 41 "parse.lex"
	return (TYPE);
break;
case 33:

# line 42 "parse.lex"
	return (EDIT);
break;
case 34:

# line 43 "parse.lex"
	return (COMMENT);
break;
case 35:

# line 44 "parse.lex"
	return (PROGRAM);
break;
case 36:

# line 45 "parse.lex"
	return (CONSOLE);
break;
case 37:

# line 46 "parse.lex"
	return (BUFFER);
break;
case 38:

# line 47 "parse.lex"
	return (SYMBOL);
break;
case 39:

# line 48 "parse.lex"
	return (STENCIL);
break;
case 40:

# line 49 "parse.lex"
	return (DEFINE);
break;
case 41:

# line 50 "parse.lex"
return (FINDREPLACE);
break;
case 42:

# line 51 "parse.lex"
	return (ALL);
break;
case 43:

# line 52 "parse.lex"
	return (SHELL);
break;
case 44:

# line 53 "parse.lex"
return (FILESELECT);
break;
case 45:

# line 54 "parse.lex"
	return (REMARK);
break;
case 46:

# line 55 "parse.lex"
	return (NOTEQUAL);
break;
case 47:

# line 56 "parse.lex"
	return (LESSEQUAL);
break;
case 48:

# line 57 "parse.lex"
	return (GREATEREQUAL);
break;
case 49:

# line 58 "parse.lex"
	return (LOGICAND);
break;
case 50:

# line 59 "parse.lex"
	return (LOGICOR);
break;
case 51:

# line 60 "parse.lex"
	return (LOGICXOR);
break;
case 52:

# line 61 "parse.lex"
	return (ASSIGN);
break;
case 53:

# line 62 "parse.lex"
         return (RANGE);
break;
case 54:

# line 63 "parse.lex"
	return (MINUS);
break;
case 55:

# line 64 "parse.lex"
	return (ADD);
break;
case 56:

# line 65 "parse.lex"
	return (MULTIPLY);
break;
case 57:

# line 66 "parse.lex"
	return (DIVIDE);
break;
case 58:

# line 67 "parse.lex"
	return (REMAINDER);
break;
case 59:

# line 68 "parse.lex"
	return (LOGICNOT);
break;
case 60:

# line 69 "parse.lex"
	return (BITNOT);
break;
case 61:

# line 70 "parse.lex"
	return (LAZYREF);
break;
case 62:

# line 71 "parse.lex"
	return (EQUAL);
break;
case 63:

# line 72 "parse.lex"
	return (LESS);
break;
case 64:

# line 73 "parse.lex"
	return (GREATER);
break;
case 65:

# line 74 "parse.lex"
	return (BITOR);
break;
case 66:

# line 75 "parse.lex"
	return (BITAND);
break;
case 67:

# line 76 "parse.lex"
	return (BITXOR);
break;
case 68:

# line 77 "parse.lex"
       { yylval.ival=strtol(yytext,NULL,8); return (INTEGER); }
break;
case 69:

# line 78 "parse.lex"
{ yylval.ival=strtol(yytext,NULL,16); return (INTEGER); }
break;
case 70:

# line 79 "parse.lex"
        { yylval.ival=strtol(yytext,NULL,10); return (INTEGER); }
break;
case 71:

# line 80 "parse.lex"
case 72:

# line 81 "parse.lex"
case 73:

# line 82 "parse.lex"
   case 74:

# line 83 "parse.lex"
case 75:

# line 84 "parse.lex"
{ sscanf(yytext, "%lf", &yylval.rval); return (REAL); }
break;
case 76:

# line 85 "parse.lex"
	{ int i;
		for (i=yyleng-1; i>=0 && yytext[i]=='\\'; i--);
		if ((yyleng-i)%2) {
			char *conv;
			int i;
			Uchar *ucon,*slashconv;
			input();
                        conv=(char*) malloc((yyleng+1)*sizeof(char));
			for (i=0; i<yyleng-1;i++) {
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
break;
case 77:

# line 101 "parse.lex"
	{ int i;
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
break;
case 78:

# line 113 "parse.lex"
   case 79:

# line 114 "parse.lex"
case 80:

# line 115 "parse.lex"
{ Uchar b[40];
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
break;
case 81:

# line 120 "parse.lex"
{ Type t;
			  t = lookup_type(yytext);
			  if (t) {
			     yylval.ival=t;
			     return (TYPEVAL);
			  }
			  yylval.tval=ident_buffer[current_id++];
			  if (current_id==MAXIDENTIFIER) current_id=0;
                          strncpy(yylval.tval,yytext, IDENTLENGTH-1);
                          return (IDENTIFIER); }
break;
case 82:

# line 130 "parse.lex"
{ int i;
		  if (yytext[yyleng-1]=='*') {
		    input();
		  } else {
		    yymore();
		  }
		}
break;
case 83:

# line 137 "parse.lex"
	;
break;
case 84:

# line 138 "parse.lex"
	return yytext[0];
break;
case -1:
break;
default:
(void)fprintf(yyout,"bad switch yylook %d",nstr);
} return(0); }
/* end of yylex */
int yyvstop[] = {
0,

84,
0,

83,
84,
0,

83,
0,

59,
84,
0,

76,
84,
0,

58,
84,
0,

65,
84,
0,

84,
0,

56,
84,
0,

55,
84,
0,

54,
84,
0,

84,
0,

57,
84,
0,

70,
84,
0,

70,
84,
0,

84,
0,

63,
84,
0,

62,
84,
0,

64,
84,
0,

61,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

67,
84,
0,

77,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

81,
84,
0,

66,
84,
0,

60,
84,
0,

46,
0,

76,
0,

49,
0,

73,
0,

82,
0,

71,
0,

68,
70,
0,

70,
0,

52,
0,

47,
0,

48,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

51,
0,

77,
0,

14,
81,
0,

81,
0,

13,
81,
0,

10,
81,
0,

16,
81,
0,

50,
0,

78,
0,

78,
0,

53,
0,

72,
0,

69,
0,

42,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

18,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

30,
81,
0,

81,
0,

81,
0,

9,
81,
0,

81,
0,

78,
80,
0,

74,
0,

75,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

33,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

27,
81,
0,

5,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

32,
81,
0,

12,
81,
0,

80,
0,

79,
0,

81,
0,

81,
0,

81,
0,

3,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

25,
81,
0,

81,
0,

2,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

28,
81,
0,

81,
0,

81,
0,

43,
81,
0,

81,
0,

81,
0,

22,
81,
0,

81,
0,

81,
0,

81,
0,

29,
81,
0,

37,
81,
0,

24,
81,
0,

81,
0,

81,
0,

81,
0,

40,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

4,
81,
0,

81,
0,

81,
0,

81,
0,

45,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

38,
81,
0,

81,
0,

15,
81,
0,

11,
81,
0,

34,
81,
0,

36,
81,
0,

23,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

1,
81,
0,

81,
0,

81,
0,

17,
81,
0,

35,
81,
0,

81,
0,

81,
0,

81,
0,

39,
81,
0,

81,
0,

81,
0,

81,
0,

8,
81,
0,

31,
81,
0,

6,
81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

81,
0,

19,
81,
0,

20,
81,
0,

26,
81,
0,

21,
81,
0,

81,
0,

44,
81,
0,

81,
0,

81,
0,

41,
81,
0,

7,
81,
0,
0};
# define YYTYPE int
struct yywork { YYTYPE verify, advance; } yycrank[] = {
0,0,	0,0,	1,3,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,4,	1,5,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	0,0,	0,0,	
0,0,	0,0,	1,6,	1,7,	
0,0,	0,0,	1,8,	1,9,	
1,10,	9,52,	51,0,	1,11,	
1,12,	15,57,	1,13,	1,14,	
1,15,	1,16,	1,17,	1,17,	
1,17,	1,17,	1,17,	1,17,	
1,17,	1,17,	53,106,	1,18,	
55,110,	1,19,	1,20,	1,21,	
6,50,	1,22,	1,23,	1,24,	
1,25,	1,26,	1,27,	1,28,	
1,29,	1,30,	1,31,	1,30,	
1,32,	1,33,	1,34,	1,30,	
1,35,	1,36,	1,30,	1,37,	
1,38,	1,39,	1,30,	1,40,	
1,30,	1,30,	1,30,	1,30,	
18,63,	19,64,	21,65,	1,41,	
1,3,	1,42,	1,30,	1,30,	
1,30,	1,43,	1,44,	1,45,	
1,30,	1,30,	1,46,	1,30,	
1,30,	1,30,	1,30,	1,30,	
1,47,	1,30,	1,30,	1,30,	
1,30,	1,30,	1,30,	1,30,	
1,30,	30,66,	1,30,	1,30,	
2,6,	1,48,	41,98,	1,49,	
2,8,	2,9,	2,10,	25,70,	
26,72,	2,11,	25,71,	43,66,	
2,13,	2,14,	43,100,	26,66,	
2,17,	2,17,	2,17,	2,17,	
2,17,	2,17,	2,17,	24,66,	
44,101,	2,18,	24,68,	2,19,	
2,20,	2,21,	27,73,	2,22,	
24,69,	2,24,	2,25,	2,26,	
48,105,	2,28,	27,66,	2,30,	
2,31,	2,30,	2,32,	2,33,	
2,34,	2,30,	2,35,	2,36,	
2,30,	2,37,	2,38,	2,39,	
2,30,	2,40,	2,30,	29,76,	
2,30,	2,30,	31,66,	31,77,	
31,78,	2,41,	29,66,	66,66,	
2,30,	2,30,	2,30,	2,43,	
2,44,	2,45,	2,30,	2,30,	
2,46,	2,30,	2,30,	2,30,	
2,30,	2,30,	2,47,	2,30,	
2,30,	2,30,	2,30,	2,30,	
2,30,	2,30,	2,30,	7,51,	
2,30,	2,30,	33,80,	2,48,	
32,79,	2,49,	33,81,	7,51,	
7,51,	10,53,	35,66,	32,66,	
45,102,	33,66,	35,83,	45,66,	
67,116,	10,53,	10,0,	14,55,	
99,0,	14,56,	14,56,	14,56,	
14,56,	14,56,	14,56,	14,56,	
14,56,	14,56,	14,56,	28,74,	
7,0,	34,82,	28,66,	36,84,	
40,97,	74,125,	36,66,	74,126,	
34,66,	7,51,	10,53,	28,75,	
36,85,	7,51,	7,51,	40,66,	
71,66,	71,121,	71,122,	10,53,	
46,103,	37,86,	7,51,	10,53,	
10,53,	37,87,	46,66,	72,123,	
37,66,	100,66,	47,104,	7,51,	
10,53,	72,66,	38,88,	7,51,	
38,89,	7,51,	47,66,	38,90,	
68,66,	10,53,	75,66,	38,66,	
75,127,	10,53,	102,66,	10,53,	
68,117,	39,93,	70,120,	38,91,	
39,66,	69,118,	7,51,	39,94,	
38,92,	70,66,	39,95,	69,66,	
73,124,	7,51,	7,51,	73,66,	
10,53,	39,96,	76,66,	69,119,	
10,54,	76,128,	89,66,	10,53,	
10,53,	16,58,	89,142,	16,59,	
16,59,	16,59,	16,59,	16,59,	
16,59,	16,59,	16,59,	16,60,	
16,60,	59,59,	59,59,	59,59,	
59,59,	59,59,	59,59,	59,59,	
59,59,	86,66,	86,139,	17,58,	
16,61,	17,60,	17,60,	17,60,	
17,60,	17,60,	17,60,	17,60,	
17,60,	17,60,	17,60,	78,130,	
77,129,	79,66,	80,66,	81,134,	
82,66,	83,66,	82,135,	16,62,	
78,66,	81,66,	17,61,	77,66,	
78,131,	83,136,	79,132,	80,133,	
84,66,	88,66,	84,137,	85,66,	
16,61,	103,66,	85,138,	88,141,	
92,66,	92,145,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
87,140,	104,66,	116,66,	16,62,	
130,173,	87,66,	17,61,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	137,66,	94,66,	121,66,	
121,163,	23,66,	94,147,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,67,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	23,66,	23,66,	23,66,	
23,66,	42,99,	96,66,	93,66,	
54,53,	143,185,	96,149,	128,66,	
128,171,	42,99,	42,99,	93,146,	
54,53,	54,0,	56,56,	56,56,	
56,56,	56,56,	56,56,	56,56,	
56,56,	56,56,	56,56,	56,56,	
58,58,	58,58,	58,58,	58,58,	
58,58,	58,58,	58,58,	58,58,	
58,58,	58,58,	42,99,	56,111,	
117,66,	54,53,	95,148,	144,66,	
146,188,	144,186,	54,107,	42,99,	
117,159,	58,112,	54,53,	42,99,	
42,99,	95,66,	54,53,	54,108,	
97,66,	90,143,	91,144,	101,66,	
42,99,	118,160,	97,150,	54,53,	
90,66,	91,66,	101,151,	118,66,	
120,162,	42,99,	147,66,	56,111,	
54,53,	42,99,	57,57,	42,99,	
54,53,	119,66,	54,53,	120,66,	
122,66,	58,112,	57,57,	57,57,	
125,168,	119,161,	123,165,	122,164,	
124,66,	140,182,	126,169,	125,66,	
42,99,	140,66,	123,166,	54,53,	
124,167,	123,66,	126,66,	42,99,	
42,0,	127,170,	54,53,	54,53,	
129,172,	131,66,	139,181,	57,57,	
132,175,	129,66,	127,66,	134,66,	
133,66,	135,66,	131,174,	133,176,	
57,57,	139,66,	132,66,	134,177,	
57,0,	57,57,	135,178,	136,179,	
142,184,	138,180,	136,66,	54,109,	
141,66,	57,57,	138,66,	141,183,	
148,66,	149,190,	148,189,	142,66,	
150,66,	153,192,	57,57,	154,193,	
149,66,	61,113,	57,57,	61,113,	
57,57,	167,66,	61,114,	61,114,	
61,114,	61,114,	61,114,	61,114,	
61,114,	61,114,	61,114,	61,114,	
145,187,	151,191,	160,195,	159,66,	
161,66,	57,57,	159,194,	161,196,	
151,66,	160,66,	145,66,	178,66,	
57,57,	57,57,	62,115,	62,115,	
62,115,	62,115,	62,115,	62,115,	
62,115,	62,115,	62,115,	62,115,	
162,66,	166,66,	164,66,	166,201,	
108,152,	164,199,	162,197,	62,115,	
62,115,	62,115,	62,115,	62,115,	
62,115,	108,153,	108,153,	108,153,	
108,153,	108,153,	108,153,	108,153,	
108,153,	111,155,	175,66,	111,155,	
183,216,	175,209,	111,156,	111,156,	
111,156,	111,156,	111,156,	111,156,	
111,156,	111,156,	111,156,	111,156,	
109,106,	185,218,	190,66,	62,115,	
62,115,	62,115,	62,115,	62,115,	
62,115,	109,154,	109,154,	109,154,	
109,154,	109,154,	109,154,	109,154,	
109,154,	109,154,	109,154,	179,66,	
187,66,	197,66,	179,212,	187,220,	
165,66,	199,229,	109,154,	109,154,	
109,154,	109,154,	109,154,	109,154,	
112,157,	165,200,	112,157,	194,66,	
194,225,	112,158,	112,158,	112,158,	
112,158,	112,158,	112,158,	112,158,	
112,158,	112,158,	112,158,	113,114,	
113,114,	113,114,	113,114,	113,114,	
113,114,	113,114,	113,114,	113,114,	
113,114,	200,230,	109,154,	109,154,	
109,154,	109,154,	109,154,	109,154,	
155,156,	155,156,	155,156,	155,156,	
155,156,	155,156,	155,156,	155,156,	
155,156,	155,156,	157,158,	157,158,	
157,158,	157,158,	157,158,	157,158,	
157,158,	157,158,	157,158,	157,158,	
163,198,	168,202,	169,203,	170,66,	
177,211,	171,205,	172,206,	163,66,	
173,66,	180,66,	174,66,	170,204,	
171,66,	172,66,	176,66,	180,213,	
181,66,	173,207,	174,208,	182,66,	
186,219,	204,234,	181,214,	176,210,	
204,66,	184,66,	168,66,	182,215,	
169,66,	186,66,	177,66,	184,217,	
188,221,	189,66,	206,66,	201,231,	
191,223,	195,66,	208,66,	188,66,	
189,222,	191,224,	201,66,	195,226,	
191,66,	196,66,	198,66,	196,227,	
198,228,	202,232,	203,233,	205,66,	
207,236,	209,237,	210,66,	211,239,	
202,66,	203,66,	211,66,	205,235,	
207,66,	212,66,	210,238,	212,240,	
209,66,	213,241,	214,242,	214,66,	
215,243,	216,244,	217,245,	218,66,	
219,246,	220,247,	221,66,	219,66,	
213,66,	222,248,	224,250,	223,66,	
225,66,	217,66,	223,249,	226,66,	
224,66,	227,66,	228,66,	231,66,	
229,252,	230,66,	232,254,	238,66,	
233,66,	235,66,	228,251,	229,66,	
233,255,	230,253,	234,66,	235,257,	
215,66,	234,256,	236,258,	239,260,	
237,66,	240,66,	241,66,	241,262,	
239,66,	236,66,	237,259,	242,66,	
240,261,	243,263,	244,264,	245,66,	
246,266,	247,66,	248,267,	249,66,	
243,66,	250,66,	251,66,	245,265,	
244,66,	252,66,	253,66,	254,268,	
255,269,	248,66,	256,66,	257,66,	
256,270,	258,66,	254,66,	259,272,	
260,273,	261,66,	262,66,	263,274,	
260,66,	264,275,	266,66,	259,66,	
257,271,	263,66,	265,66,	267,66,	
268,278,	265,276,	269,279,	270,66,	
264,66,	271,66,	272,66,	267,277,	
273,66,	268,66,	274,66,	275,66,	
277,284,	269,66,	276,66,	277,66,	
273,280,	275,282,	274,281,	278,66,	
276,283,	279,286,	280,66,	281,66,	
282,66,	283,66,	284,66,	278,285,	
285,66,	284,287,	279,66,	286,288,	
287,66,	288,66,	287,289,	289,66,	
0,0,	0,0,	286,66,	0,0,	
0,0};
struct yysvf yysvec[] = {
0,	0,	0,
yycrank+-1,	0,		0,	
yycrank+-91,	yysvec+1,	0,	
yycrank+0,	0,		yyvstop+1,
yycrank+0,	0,		yyvstop+3,
yycrank+0,	0,		yyvstop+6,
yycrank+3,	0,		yyvstop+8,
yycrank+-210,	0,		yyvstop+11,
yycrank+0,	0,		yyvstop+14,
yycrank+3,	0,		yyvstop+17,
yycrank+-220,	0,		yyvstop+20,
yycrank+0,	0,		yyvstop+22,
yycrank+0,	0,		yyvstop+25,
yycrank+0,	0,		yyvstop+28,
yycrank+185,	0,		yyvstop+31,
yycrank+3,	0,		yyvstop+33,
yycrank+271,	0,		yyvstop+36,
yycrank+293,	0,		yyvstop+39,
yycrank+31,	0,		yyvstop+42,
yycrank+32,	0,		yyvstop+44,
yycrank+0,	0,		yyvstop+47,
yycrank+33,	0,		yyvstop+50,
yycrank+0,	0,		yyvstop+53,
yycrank+330,	0,		yyvstop+56,
yycrank+39,	yysvec+23,	yyvstop+59,
yycrank+23,	yysvec+23,	yyvstop+62,
yycrank+31,	yysvec+23,	yyvstop+65,
yycrank+54,	yysvec+23,	yyvstop+68,
yycrank+138,	yysvec+23,	yyvstop+71,
yycrank+78,	yysvec+23,	yyvstop+74,
yycrank+13,	yysvec+23,	yyvstop+77,
yycrank+74,	yysvec+23,	yyvstop+80,
yycrank+115,	yysvec+23,	yyvstop+83,
yycrank+117,	yysvec+23,	yyvstop+86,
yycrank+144,	yysvec+23,	yyvstop+89,
yycrank+114,	yysvec+23,	yyvstop+92,
yycrank+142,	yysvec+23,	yyvstop+95,
yycrank+164,	yysvec+23,	yyvstop+98,
yycrank+179,	yysvec+23,	yyvstop+101,
yycrank+188,	yysvec+23,	yyvstop+104,
yycrank+151,	yysvec+23,	yyvstop+107,
yycrank+32,	0,		yyvstop+110,
yycrank+-452,	0,		yyvstop+113,
yycrank+27,	yysvec+23,	yyvstop+116,
yycrank+40,	yysvec+23,	yyvstop+119,
yycrank+119,	yysvec+23,	yyvstop+122,
yycrank+162,	yysvec+23,	yyvstop+125,
yycrank+174,	yysvec+23,	yyvstop+128,
yycrank+36,	0,		yyvstop+131,
yycrank+0,	0,		yyvstop+134,
yycrank+0,	0,		yyvstop+137,
yycrank+-8,	yysvec+7,	yyvstop+139,
yycrank+0,	0,		yyvstop+141,
yycrank+19,	0,		0,	
yycrank+-455,	0,		0,	
yycrank+14,	0,		0,	
yycrank+418,	0,		yyvstop+143,
yycrank+-521,	0,		yyvstop+145,
yycrank+428,	0,		yyvstop+147,
yycrank+281,	yysvec+17,	yyvstop+149,
yycrank+0,	yysvec+17,	yyvstop+152,
yycrank+546,	0,		0,	
yycrank+570,	0,		0,	
yycrank+0,	0,		yyvstop+154,
yycrank+0,	0,		yyvstop+156,
yycrank+0,	0,		yyvstop+158,
yycrank+79,	yysvec+23,	yyvstop+160,
yycrank+120,	yysvec+23,	yyvstop+162,
yycrank+176,	yysvec+23,	yyvstop+164,
yycrank+195,	yysvec+23,	yyvstop+166,
yycrank+193,	yysvec+23,	yyvstop+168,
yycrank+152,	yysvec+23,	yyvstop+170,
yycrank+169,	yysvec+23,	yyvstop+172,
yycrank+199,	yysvec+23,	yyvstop+174,
yycrank+141,	yysvec+23,	yyvstop+176,
yycrank+178,	yysvec+23,	yyvstop+178,
yycrank+202,	yysvec+23,	yyvstop+180,
yycrank+255,	yysvec+23,	yyvstop+182,
yycrank+252,	yysvec+23,	yyvstop+184,
yycrank+245,	yysvec+23,	yyvstop+186,
yycrank+246,	yysvec+23,	yyvstop+188,
yycrank+253,	yysvec+23,	yyvstop+190,
yycrank+248,	yysvec+23,	yyvstop+192,
yycrank+249,	yysvec+23,	yyvstop+194,
yycrank+260,	yysvec+23,	yyvstop+196,
yycrank+263,	yysvec+23,	yyvstop+198,
yycrank+229,	yysvec+23,	yyvstop+200,
yycrank+285,	yysvec+23,	yyvstop+202,
yycrank+261,	yysvec+23,	yyvstop+204,
yycrank+206,	yysvec+23,	yyvstop+206,
yycrank+404,	yysvec+23,	yyvstop+208,
yycrank+405,	yysvec+23,	yyvstop+210,
yycrank+268,	yysvec+23,	yyvstop+212,
yycrank+347,	yysvec+23,	yyvstop+214,
yycrank+314,	yysvec+23,	yyvstop+216,
yycrank+393,	yysvec+23,	yyvstop+218,
yycrank+346,	yysvec+23,	yyvstop+220,
yycrank+396,	yysvec+23,	yyvstop+222,
yycrank+0,	0,		yyvstop+224,
yycrank+-136,	yysvec+42,	yyvstop+226,
yycrank+165,	yysvec+23,	yyvstop+228,
yycrank+399,	yysvec+23,	yyvstop+231,
yycrank+182,	yysvec+23,	yyvstop+233,
yycrank+265,	yysvec+23,	yyvstop+236,
yycrank+281,	yysvec+23,	yyvstop+239,
yycrank+0,	0,		yyvstop+242,
yycrank+0,	0,		yyvstop+244,
yycrank+0,	yysvec+53,	yyvstop+246,
yycrank+593,	0,		0,	
yycrank+625,	0,		0,	
yycrank+0,	0,		yyvstop+248,
yycrank+606,	0,		0,	
yycrank+653,	0,		0,	
yycrank+663,	0,		0,	
yycrank+0,	yysvec+113,	yyvstop+250,
yycrank+0,	yysvec+62,	yyvstop+252,
yycrank+282,	yysvec+23,	yyvstop+254,
yycrank+380,	yysvec+23,	yyvstop+257,
yycrank+407,	yysvec+23,	yyvstop+259,
yycrank+417,	yysvec+23,	yyvstop+261,
yycrank+419,	yysvec+23,	yyvstop+263,
yycrank+315,	yysvec+23,	yyvstop+265,
yycrank+420,	yysvec+23,	yyvstop+267,
yycrank+437,	yysvec+23,	yyvstop+269,
yycrank+428,	yysvec+23,	yyvstop+271,
yycrank+431,	yysvec+23,	yyvstop+273,
yycrank+438,	yysvec+23,	yyvstop+275,
yycrank+450,	yysvec+23,	yyvstop+277,
yycrank+351,	yysvec+23,	yyvstop+279,
yycrank+449,	yysvec+23,	yyvstop+281,
yycrank+284,	yysvec+23,	yyvstop+283,
yycrank+445,	yysvec+23,	yyvstop+285,
yycrank+458,	yysvec+23,	yyvstop+287,
yycrank+452,	yysvec+23,	yyvstop+289,
yycrank+451,	yysvec+23,	yyvstop+291,
yycrank+453,	yysvec+23,	yyvstop+293,
yycrank+466,	yysvec+23,	yyvstop+295,
yycrank+313,	yysvec+23,	yyvstop+297,
yycrank+470,	yysvec+23,	yyvstop+300,
yycrank+457,	yysvec+23,	yyvstop+302,
yycrank+433,	yysvec+23,	yyvstop+304,
yycrank+468,	yysvec+23,	yyvstop+306,
yycrank+475,	yysvec+23,	yyvstop+308,
yycrank+349,	yysvec+23,	yyvstop+310,
yycrank+383,	yysvec+23,	yyvstop+312,
yycrank+506,	yysvec+23,	yyvstop+314,
yycrank+384,	yysvec+23,	yyvstop+316,
yycrank+410,	yysvec+23,	yyvstop+318,
yycrank+472,	yysvec+23,	yyvstop+321,
yycrank+480,	yysvec+23,	yyvstop+323,
yycrank+476,	yysvec+23,	yyvstop+325,
yycrank+504,	yysvec+23,	yyvstop+328,
yycrank+0,	0,		yyvstop+330,
yycrank+546,	yysvec+108,	0,	
yycrank+548,	yysvec+109,	0,	
yycrank+680,	0,		0,	
yycrank+0,	yysvec+155,	yyvstop+333,
yycrank+690,	0,		0,	
yycrank+0,	yysvec+157,	yyvstop+335,
yycrank+499,	yysvec+23,	yyvstop+337,
yycrank+505,	yysvec+23,	yyvstop+339,
yycrank+500,	yysvec+23,	yyvstop+341,
yycrank+520,	yysvec+23,	yyvstop+343,
yycrank+647,	yysvec+23,	yyvstop+345,
yycrank+522,	yysvec+23,	yyvstop+347,
yycrank+580,	yysvec+23,	yyvstop+349,
yycrank+521,	yysvec+23,	yyvstop+351,
yycrank+485,	yysvec+23,	yyvstop+353,
yycrank+666,	yysvec+23,	yyvstop+356,
yycrank+668,	yysvec+23,	yyvstop+358,
yycrank+643,	yysvec+23,	yyvstop+360,
yycrank+652,	yysvec+23,	yyvstop+362,
yycrank+653,	yysvec+23,	yyvstop+364,
yycrank+648,	yysvec+23,	yyvstop+366,
yycrank+650,	yysvec+23,	yyvstop+368,
yycrank+542,	yysvec+23,	yyvstop+370,
yycrank+654,	yysvec+23,	yyvstop+372,
yycrank+670,	yysvec+23,	yyvstop+374,
yycrank+507,	yysvec+23,	yyvstop+377,
yycrank+575,	yysvec+23,	yyvstop+380,
yycrank+649,	yysvec+23,	yyvstop+382,
yycrank+656,	yysvec+23,	yyvstop+384,
yycrank+659,	yysvec+23,	yyvstop+386,
yycrank+544,	yysvec+23,	yyvstop+388,
yycrank+665,	yysvec+23,	yyvstop+390,
yycrank+557,	yysvec+23,	yyvstop+392,
yycrank+669,	yysvec+23,	yyvstop+394,
yycrank+576,	yysvec+23,	yyvstop+396,
yycrank+679,	yysvec+23,	yyvstop+398,
yycrank+673,	yysvec+23,	yyvstop+400,
yycrank+558,	yysvec+23,	yyvstop+402,
yycrank+684,	yysvec+23,	yyvstop+405,
yycrank+0,	0,		yyvstop+408,
yycrank+0,	0,		yyvstop+410,
yycrank+591,	yysvec+23,	yyvstop+412,
yycrank+677,	yysvec+23,	yyvstop+414,
yycrank+685,	yysvec+23,	yyvstop+416,
yycrank+577,	yysvec+23,	yyvstop+418,
yycrank+686,	yysvec+23,	yyvstop+421,
yycrank+581,	yysvec+23,	yyvstop+423,
yycrank+613,	yysvec+23,	yyvstop+425,
yycrank+682,	yysvec+23,	yyvstop+427,
yycrank+696,	yysvec+23,	yyvstop+429,
yycrank+697,	yysvec+23,	yyvstop+431,
yycrank+664,	yysvec+23,	yyvstop+433,
yycrank+691,	yysvec+23,	yyvstop+435,
yycrank+674,	yysvec+23,	yyvstop+437,
yycrank+700,	yysvec+23,	yyvstop+440,
yycrank+678,	yysvec+23,	yyvstop+442,
yycrank+704,	yysvec+23,	yyvstop+445,
yycrank+694,	yysvec+23,	yyvstop+447,
yycrank+698,	yysvec+23,	yyvstop+449,
yycrank+701,	yysvec+23,	yyvstop+451,
yycrank+716,	yysvec+23,	yyvstop+453,
yycrank+707,	yysvec+23,	yyvstop+455,
yycrank+740,	yysvec+23,	yyvstop+457,
yycrank+709,	yysvec+23,	yyvstop+460,
yycrank+721,	yysvec+23,	yyvstop+462,
yycrank+711,	yysvec+23,	yyvstop+464,
yycrank+715,	yysvec+23,	yyvstop+467,
yycrank+713,	yysvec+23,	yyvstop+469,
yycrank+714,	yysvec+23,	yyvstop+471,
yycrank+717,	yysvec+23,	yyvstop+474,
yycrank+719,	yysvec+23,	yyvstop+476,
yycrank+724,	yysvec+23,	yyvstop+478,
yycrank+720,	yysvec+23,	yyvstop+480,
yycrank+723,	yysvec+23,	yyvstop+483,
yycrank+725,	yysvec+23,	yyvstop+486,
yycrank+726,	yysvec+23,	yyvstop+489,
yycrank+735,	yysvec+23,	yyvstop+491,
yycrank+729,	yysvec+23,	yyvstop+493,
yycrank+727,	yysvec+23,	yyvstop+495,
yycrank+730,	yysvec+23,	yyvstop+498,
yycrank+732,	yysvec+23,	yyvstop+500,
yycrank+738,	yysvec+23,	yyvstop+502,
yycrank+733,	yysvec+23,	yyvstop+504,
yycrank+749,	yysvec+23,	yyvstop+506,
yycrank+744,	yysvec+23,	yyvstop+508,
yycrank+731,	yysvec+23,	yyvstop+510,
yycrank+748,	yysvec+23,	yyvstop+513,
yycrank+745,	yysvec+23,	yyvstop+515,
yycrank+746,	yysvec+23,	yyvstop+517,
yycrank+751,	yysvec+23,	yyvstop+519,
yycrank+760,	yysvec+23,	yyvstop+522,
yycrank+764,	yysvec+23,	yyvstop+524,
yycrank+755,	yysvec+23,	yyvstop+526,
yycrank+756,	yysvec+23,	yyvstop+528,
yycrank+757,	yysvec+23,	yyvstop+530,
yycrank+769,	yysvec+23,	yyvstop+533,
yycrank+759,	yysvec+23,	yyvstop+535,
yycrank+761,	yysvec+23,	yyvstop+538,
yycrank+762,	yysvec+23,	yyvstop+541,
yycrank+765,	yysvec+23,	yyvstop+544,
yycrank+766,	yysvec+23,	yyvstop+547,
yycrank+774,	yysvec+23,	yyvstop+550,
yycrank+768,	yysvec+23,	yyvstop+552,
yycrank+770,	yysvec+23,	yyvstop+554,
yycrank+771,	yysvec+23,	yyvstop+556,
yycrank+773,	yysvec+23,	yyvstop+558,
yycrank+783,	yysvec+23,	yyvstop+561,
yycrank+780,	yysvec+23,	yyvstop+563,
yycrank+777,	yysvec+23,	yyvstop+565,
yycrank+778,	yysvec+23,	yyvstop+568,
yycrank+785,	yysvec+23,	yyvstop+571,
yycrank+792,	yysvec+23,	yyvstop+573,
yycrank+786,	yysvec+23,	yyvstop+575,
yycrank+782,	yysvec+23,	yyvstop+577,
yycrank+787,	yysvec+23,	yyvstop+580,
yycrank+797,	yysvec+23,	yyvstop+582,
yycrank+801,	yysvec+23,	yyvstop+584,
yycrank+791,	yysvec+23,	yyvstop+586,
yycrank+793,	yysvec+23,	yyvstop+589,
yycrank+794,	yysvec+23,	yyvstop+592,
yycrank+796,	yysvec+23,	yyvstop+595,
yycrank+798,	yysvec+23,	yyvstop+597,
yycrank+799,	yysvec+23,	yyvstop+599,
yycrank+802,	yysvec+23,	yyvstop+601,
yycrank+803,	yysvec+23,	yyvstop+603,
yycrank+807,	yysvec+23,	yyvstop+605,
yycrank+818,	yysvec+23,	yyvstop+607,
yycrank+810,	yysvec+23,	yyvstop+609,
yycrank+811,	yysvec+23,	yyvstop+612,
yycrank+812,	yysvec+23,	yyvstop+615,
yycrank+813,	yysvec+23,	yyvstop+618,
yycrank+814,	yysvec+23,	yyvstop+621,
yycrank+816,	yysvec+23,	yyvstop+623,
yycrank+826,	yysvec+23,	yyvstop+626,
yycrank+820,	yysvec+23,	yyvstop+628,
yycrank+821,	yysvec+23,	yyvstop+630,
yycrank+823,	yysvec+23,	yyvstop+633,
0,	0,	0};
struct yywork *yytop = yycrank+934;
struct yysvf *yybgin = yysvec+1;
char yymatch[] = {
  0,   1,   1,   1,   1,   1,   1,   1, 
  1,   9,  10,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  1,   1,   1,   1,   1,   1,   1,   1, 
  9,   1,  34,   1,   1,   1,   1,   1, 
  1,   1,   1,  43,   1,  43,   1,  47, 
 48,  48,  48,  48,  48,  48,  48,  48, 
 56,  56,   1,   1,   1,   1,   1,   1, 
  1,  65,  65,  65,  65,  69,  65,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 88,  71,  71,   1,   1,   1,   1,  95, 
 96,  65,  65,  65,  65,  69,  65,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 71,  71,  71,  71,  71,  71,  71,  71, 
 88,  71,  71,   1,   1,   1,   1,   1, 
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
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};
/*	Copyright (c) 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#pragma ident	"@(#)ncform	6.8	95/02/11 SMI"

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
			if(yylastch > &yytext[YYLMAX]) {
				fprintf(yyout,"Input string too long, limit %d\n",YYLMAX);
				exit(1);
			}
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
