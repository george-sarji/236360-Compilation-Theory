%{

/* Declarations section */
#include <stdio.h>
#include "tokens.hpp"

void addWord(const char *word);
void convertEscape(const char *escape);
void illegalEscape(const char *escape);
%}

%option yylineno
%option noyywrap

digit       ([0-9])
letter      ([a-zA-Z])
char        ([^\n\r\\"])
whitespace  ([\t\n\r ])
quote       ([\"])
escape      \\(n|r|t|0|\\|\"|x([0-7][0-9A-F]))
word        ([ !#-\[\]-~])

%x INPUT_STRING
%%
(void)                                              printf("%d VOID %s\n", yylineno, yytext);
(int)                                               printf("%d INT %s\n", yylineno, yytext);
(byte)                                              printf("%d BYTE %s\n", yylineno, yytext);
(b)                                                 printf("%d B %s\n", yylineno, yytext);
(bool)                                              printf("%d BOOL %s\n", yylineno, yytext);
(and)                                               printf("%d AND %s\n", yylineno, yytext);
(or)                                                printf("%d OR %s\n", yylineno, yytext);
(not)                                               printf("%d NOT %s\n", yylineno, yytext);
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
((\/\/)[^\r\n]*)                                   printf("%d COMMENT //\n", yylineno);
({letter}({letter}|{digit})*)                       printf("%d ID %s\n", yylineno, yytext);
(0|[1-9]{digit}*)                                   printf("%d NUM %s\n", yylineno, yytext);
{quote}                                             BEGIN(INPUT_STRING);
<INPUT_STRING>{word}*                               addWord(yytext);
<INPUT_STRING>{escape}                              convertEscape(yytext);
<INPUT_STRING>\\x[^\r\n\t \"]{1,2}                  illegalEscape(yytext);
<INPUT_STRING>\\.                                   illegalEscape(yytext);
<INPUT_STRING>[\n\r]                                printf("Error unclosed string\n"); exit(0);
<INPUT_STRING><<EOF>>                               printf("Error unclosed string\n"); exit(0);
<INPUT_STRING>{quote}                               BEGIN(INITIAL); return STRING;
{whitespace}                                        ;
.		                                            printf("Error %s\n", yytext);

%%