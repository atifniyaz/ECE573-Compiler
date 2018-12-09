%{

#include "AST.hpp"
#include "CodeObject.hpp"
#include "LLString.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"

#include "micro.h"

extern "C" int yylex();

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

{DIGIT}+                      	{ 
									yylval.intVal = atoi(yytext);
									return INTLITERAL; 
								}

{QUOTE}[^"]*{QUOTE}				{ 
									yylval.stringList = new LLString(strdup(yytext));
									return STRINGLITERAL; 
								}

{LETTER}({LETTER}|{DIGIT})*    	{ 
									yylval.stringList = new LLString(strdup(yytext));
									return IDENTIFIER; 
								}

{DIGIT}*"."{DIGIT}+				{ 
									yylval.flVal = atof(yytext);
									return FLOATLITERAL; 
								}

.|\n

%%
