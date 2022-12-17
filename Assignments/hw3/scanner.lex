%{
    #include "Semantics.h"
    #include "hw3_output.hpp"
    #include "parser.tab.hpp"
    using namespace std;
%}

%option yylineno
%option noyywrap

%%

void                                yylval = new Node(yytext); return VOID;
int                                 yylval = new Node(yytext); return INT;
byte                                yylval = new Node(yytext); return BYTE;
b                                   yylval = new Node(yytext); return B;
bool                                yylval = new Node(yytext); return BOOL;
and                                 yylval = new Node(yytext); return AND;
or                                  yylval = new Node(yytext); return OR;
not                                 yylval = new Node(yytext); return NOT;
true                                yylval = new Node(yytext); return TRUE;
false                               yylval = new Node(yytext); return FALSE;
return                              yylval = new Node(yytext); return RETURN;
if                                  yylval = new Node(yytext); return IF;
else                                yylval = new Node(yytext); return ELSE;
while                               yylval = new Node(yytext); return WHILE;
break                               yylval = new Node(yytext); return BREAK;
continue                            yylval = new Node(yytext); return CONTINUE;
(;)                                 yylval = new Node(yytext); return SC;
(,)                                 yylval = new Node(yytext); return COMMA;
(\()                                yylval = new Node(yytext); return LPAREN;
(\))                                yylval = new Node(yytext); return RPAREN;
(\{)                                yylval = new Node(yytext); return LBRACE;
(\})                                yylval = new Node(yytext); return RBRACE;
(=)                                 yylval = new Node(yytext); return ASSIGN;
(==|!=)                             yylval = new Node(yytext); return EQ_RELOP;
(<|>|<=|>=)                         yylval = new Node(yytext); return RELOP;
(\+|-)                              yylval = new Node(yytext); return ADD_BINOP;
(\/|\*)                             yylval = new Node(yytext); return BINOP;
[a-zA-Z][a-zA-Z0-9]*                yylval = new Node(yytext); return ID;
0|[1-9][0-9]*                       yylval = new Node(yytext); return NUM;
\"([^\n\r\"\\]|\\[rnt"\\])+\"       yylval = new Node(yytext); return STRING;
[\r\n\t ]                           ;
\/\/[^\r\n]*[ \r|\n|\r\n]?          ;
.                                   { output::errorLex(yylineno); exit(0); }

%%