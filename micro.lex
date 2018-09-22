%{
#include "data.h"
#include "micro.h"
%}

LETTER  	[A-Za-z]
DIGIT      	[0-9]
QUOTE		\"
OPERATOR    "+"|"-"|"*"|"/"|"="|"!"|"<"|">"|"("|")"|";"|","|"<"|">"
ASSIGN_OP	":="
COMMENT		--
%%

{COMMENT}[^\n]*

{OPERATOR}    					{ return yytext[0]; }
{ASSIGN_OP}						{ return ASSIGN_OP; }

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

{QUOTE}[^"]*{QUOTE}				{ 
									yylval.strData = strdup(yytext);
									return STRINGLITERAL; 
								}

{LETTER}({LETTER}|{DIGIT})*    	{ 
									yylval.strData = strdup(yytext);
									return IDENTIFIER; 
								}

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
		struct SymbolTableStack * sk = NULL;
		struct SymbolTableStack * funcStk = NULL;

		while(stack != NULL) {
			pushStack(&sk, popStack(&stack));
		}
		funcStk = sk;
		
		while(funcStk->next != NULL) {
			funcStk = funcStk->next;
		}

		printSymbolTable(funcStk->value);
		funcStk = NULL;

		while(sk->next != NULL) {
			struct SymbolTable * data = popStack(&sk);
			pushStack(&funcStk, data);
			
			if (data->childLen == 0) {
				while (funcStk != NULL) {
					struct SymbolTable * elData = popStack(&funcStk);
					if (isLegalSymbolTable(elData)) {
						printSymbolTable(elData);
					} else {
						return 0;
					}
				}
			}
		}
	}
	return 0;
}

void yyerror(char const * s) { }