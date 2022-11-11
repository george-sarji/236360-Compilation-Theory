%{

/* Declarations section */
#include <stdio.h>
#include "tokens.hpp"

%}

%option yylineno
%option noyywrap

digit       ([0-9])
letter      ([a-zA-Z])
char        ([^\n\r\\"])
whitespace  ([\t\n\r ])
quote       ([\"])
escape      \\[nrt0x\\"]
illegal_escape  \\[^nrt0x\\"]
%x INPUT_STRING
%%
(void)                                              printf("%d VOID %s\n", yylineno, yytext);
(int)                                               printf("%d INT %s\n", yylineno, yytext);
(byte)                                              printf("%d BYTE %s\n", yylineno, yytext);
(b)                                                 printf("%d B %s\n", yylineno, yytext);
(bool)                                              printf("%d BOOL %s\n", yylineno, yytext);
(&&)                                                printf("%d AND %s\n", yylineno, yytext);
(\|\|)                                              printf("%d OR %s\n", yylineno, yytext);
(\!)                                                printf("%d NOT %s\n", yylineno, yytext);
(true)                                              printf("%d TRUE %s\n", yylineno, yytext);
(false)                                             printf("%d FALSE %s\n", yylineno, yytext);
(return)                                            printf("%d RETURN %s\n", yylineno, yytext);
(if)                                                printf("%d IF %s\n", yylineno, yytext);
(else)                                              printf("%d ELSE %s\n", yylineno, yytext);
(while)                                             printf("%d WHILE %s\n", yylineno, yytext);
(break)                                             printf("%d BREAK %s\n", yylineno, yytext);
(continue)                                          printf("%d CONTINUE %s\n", yylineno, yytext);
(;)                                                 printf("%d SC %s\n", yylineno, yytext);
(,)                                                 printf("%d COMMA %s\n", yylineno, yytext);
(\()                                                printf("%d LPAREN %s\n", yylineno, yytext);
(\))                                                printf("%d RPAREN %s\n", yylineno, yytext);
(\{)                                                printf("%d LBRACE %s\n", yylineno, yytext);
(\})                                                printf("%d RBRACE %s\n", yylineno, yytext);
(=)                                                 printf("%d ASSIGN %s\n", yylineno, yytext);
(>=|<=|==|!=|<|>)                                   printf("%d RELOP %s\n", yylineno, yytext);
(\+|\*|-|\/)                                        printf("%d BINOP %s\n", yylineno, yytext);
((\/\/)[^\n|\r]*)                                   printf("%d COMMENT //", yylineno);
({letter}({letter}|{digit})*)                       printf("%d ID %s\n", yylineno, yytext);
(0|[1-9]{digit}*)                                   printf("%d NUM %s\n", yylineno, yytext);
{whitespace}                                        ;
.		                                            printf("Error %s\n", yytext);

%%