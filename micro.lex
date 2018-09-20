%{
#include "micro.h"
%}

LETTER  	[A-Za-z]
DIGIT      	[0-9]
QUOTE		\"
OPERATOR    "+"|"-"|"*"|"/"|"="|"!"|"<"|">"|"("|")"|";"|","|"<"|">"
AS_OP		":="
COMMENT		--
%%

{COMMENT}[^\n]*

{OPERATOR}    					{ return yytext[0]; }
{AS_OP}							{ return ASSIGN_OP; }

PROGRAM							{ return PROGRAM; }
BEGIN							{ return _BEGIN; }
END								{ return END; }
FUNCTION						{ return FUNCTION; }
READ							{ return READ; }
WRITE							{ return WRITE; }
IF								{ return IF; }
ELSE							{ return ELSE; }
ENDIF							{ return ENDIF; }
WHILE							{ return WHILE; }
ENDWHILE						{ return ENDWHILE; }
RETURN							{ return RETURN; }
INT								{ return INT; }
VOID							{ return VOID; }
STRING							{ return STRING; }
FLOAT							{ return FLOAT; }
TRUE							{ return TRUE; }
FALSE							{ return FALSE; }
FOR								{ return FOR; }
ENDFOR							{ return ENDFOR; }
CONTINUE						{ return CONTINUE; }
BREAK							{ return BREAK; }

{DIGIT}+                      	{ return INTLITERAL; }

{QUOTE}[^"]*{QUOTE}				{ return STRINGLITERAL; }

{LETTER}({LETTER}|{DIGIT})*    	{ return IDENTIFIER; }

{DIGIT}*"."{DIGIT}+				{ return FLOATLITERAL; }

.|\n

%%

int main(int argc, char ** argv) {
	if (argc >= 2) {
		FILE * fp = fopen(argv[1], "r");
		if (fp == NULL) {
			return -1;
		} else {
			yyin = fp;
		}
	} else {
		return -1;
	}
	
	if(yyparse()) {
		printf("Not Accepted");
	} else {
		printf("Accepted");
	}
	return 0;
}

void yyerror(char const * s) { }