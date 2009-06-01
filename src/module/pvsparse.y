%{
#include <stdlib.h>
#include <stdio.h>
#include "mathpad.h"
#include "editor.h"

static unsigned long
  subscript_uid=3407435781UL,
  compose_uid=3406469154UL,
  times_uid=3407510728UL,
  plus_uid=3407510727UL,
  equal_uid=3383944400UL,
  apply_uid=2946556517UL,
  and_uid=2666975502UL,
  id_uid=3368830881UL;


void MakeBinOpNode(void)
{
  Char buffer[4];
  buffer[0]=buffer[2]=MP_Expr;
  buffer[1]=MP_Op;
  buffer[3]=0;
  make_node(MP_Expr, buffer, 3,0,0x3fff);
}
void MakeSubscriptNode(void)
{
   Char buffer[3];
   buffer[0]=PhNum2Char(MP_Expr,1);
   buffer[1]=PhNum2Char(MP_Expr,2);
   buffer[2]=0;
   make_node(MP_Op, buffer, 2, notation_with_number(subscript_uid), 0);
}
#ifdef RUBBISH
#undef yylook
#define yylook pvs_yylook
#undef yyextra
#define yyextra  pvs_yyextra
#undef yyvstop
#define yyvstop  pvs_yyvstop
#undef yycrank
#define yycrank  pvs_yycrank
#undef yysvec
#define yysvec  pvs_yysvec
#endif
int yyerror(char *c)
{
  fprintf(stderr, "%s\n", c);
  return 0;
}

%}
%left AND
%left EQUAL
%left PLUS
%left TIMES
%left COMPOSE
%left APPLY
%left SUBSCRIPT
%token IDENTIFIER FUNCTION
%%
expression  : expression PLUS
               {  Char buffer[1];
                  make_node(MP_Op,buffer,0,notation_with_number(plus_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | expression TIMES
               {  Char buffer[1];
                  make_node(MP_Op,buffer,0,notation_with_number(times_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | expression COMPOSE
               { Char buffer[1];
                 make_node(MP_Op,buffer,0,notation_with_number(compose_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | expression EQUAL
               {  Char buffer[1];
                  make_node(MP_Op,buffer,0,notation_with_number(equal_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | expression AND
               {  Char buffer[1];
                  make_node(MP_Op,buffer,0,notation_with_number(and_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | expression APPLY
               {  Char buffer[1];
                  make_node(MP_Op,buffer,0,notation_with_number(apply_uid),0);
               }
              expression
               {  MakeBinOpNode(); }
            | FUNCTION '(' expression ')'
               {  MakeSubscriptNode(); }
            | FUNCTION
               {  /* identifier or operator is set in lexical analyser. */ }
            | '(' expression ')'
               {  /* parenthesis are added automatically.
                     no notation needed */ ; }
            | IDENTIFIER
               {  /* identifier is set in lexical analyser */ ; }
            ;
%%
static int lastchar=' ';
static char *input_string;
static char unput_buffer[100];
static int unput_pos=0;
static int is_pos=0;
#if YYDEBUG
extern int yydebug;
#endif

static int str_input(void)
{
  if (input_string) {
    if (unput_pos)
      return unput_buffer[--unput_pos];
    else 
      return input_string[is_pos++];
  } else {
    return 0;
  }
}

static void str_unput(char c)
{
  if (input_string) {
    unput_buffer[unput_pos++]=c;
  }
}

static void str_output(char c)
{
  putc(c,stdout);
}

static int str_wrapup(void)
{
  lastchar=' ';
  return 1;
}

int pvs_parse_string(char *str)
{
  input_string=str;
  is_pos=0;
  unput_pos=0;
  lastchar=' ';
#if YYDEBUG
  yydebug=1;
#endif
  yyparse();
  return 1;
}

#include "pvsparselex.c"
            
