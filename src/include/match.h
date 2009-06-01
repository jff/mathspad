#ifndef MP_MATCH_H
#define MP_MATCH_H
/*****************************************************************
**
** MathSpad 0.60
**
** Copyright 1996, Eindhoven University of Technology (EUT)
** 
********************************************************************/


#define INFIX 6
#define PREFIX 4
#define POSTFIX 2
#define EXPR 1
#define TEXT 8
#define IDOPEN 16
#define IDCLOSE 32
#define IDFORBID 64

#define ALLOPTREE 21
#define TEXTTREE 22
#define EXPRTREE 23
#define IDOPENTREE 24
#define IDFORBIDTREE 25
#define IDCLOSETREE 26
#define MAXTREES 27

typedef struct ParseRules {
    void *optree[MAXTREES];
    void *lextree;
    int height;
    int nextretval;
} PARSERULES;

extern int lex_add_string(char* item, int retval);
extern int lex_value(char *item);

extern int parse_add_rule(int group, Char *parsestring, int len, int template,
			  int prec);
extern int parse_text(char *text);
extern int parse_use_rules(PARSERULES *set);
extern int parse_save_rules(PARSERULES *set);
extern void parser_init(void);
extern void set_parser_case(int on);
extern void set_parser_space(int on);
#endif
