%{

/* Declarations section */
#include <stdio.h>
#include "tokens.hpp"

%}

%option yylineno
%option noyywrap

%%
.		printf("Lex doesn't know what that is!\n");

%%