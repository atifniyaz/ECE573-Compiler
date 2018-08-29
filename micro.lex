%{
	void printData(char * type);
%}

LETTER  	[A-Za-z]
DIGIT      	[0-9]
KEYWORD    	PROGRAM|BEGIN|END|FUNCTION|READ|WRITE|IF|ELSE|ENDIF|WHILE|ENDWHILE|RETURN|INT|VOID|STRING|FLOAT|TRUE|FALSE|FOR|ENDFOR|CONTINUE|BREAK
QUOTE		\"
OPERATOR    ":="|"+"|"-"|"*"|"/"|"="|"!="|"<"|">"|"("|")"|";"|","|"<="|">="
COMMENT		--
%%

{COMMENT}[^\n]*

{OPERATOR}    					{ printData("OPERATOR"); }

{KEYWORD}                      	{ printData("KEYWORD"); }

{DIGIT}+                      	{ printData("INTLITERAL"); }

{QUOTE}[^"]*{QUOTE}				{ printData("STRINGLITERAL"); }

{LETTER}({LETTER}|{DIGIT})*    	{ printData("IDENTIFIER"); }

{DIGIT}*"."{DIGIT}+				{ printData("FLOATLITERAL"); }

.|\n

%%

int main(int argc, char ** argv) {
	if (argc >= 2) {
		yyin = fopen(argv[1], "r");
	} else {
		return -1;
	}
	yylex();
	return 0;
}

void printData(char * type) {
	printf("Token Type: %s\nValue: %s\n", type, yytext);
}
