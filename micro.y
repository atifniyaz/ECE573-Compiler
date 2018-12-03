%{
#include<string>

#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"

int yylex();
void yyerror(char *s);

extern "C" int yyparse();
extern "C" FILE *yyin;

void addControlDecl(Identifier * id);
 
using namespace std;

extern SymbolTableStack * stackTable;

int blockCnt = 1;
%}

%error-verbose

%union {
	Identifier * identifier;
	LLString * stringList;
}

%type <identifier> string_decl var_decl decl param_decl param_decl_list param_decl_tail
%type <stringList> id_list id_tail var_type id str

%token PROGRAM
%token _BEGIN
%token END
%token IDENTIFIER
%token STRING
%token STRINGLITERAL
%token FLOAT
%token INT
%token VOID
%token FUNCTION
%token READ
%token WRITE
%token RETURN
%token INTLITERAL
%token FLOATLITERAL
%token IF
%token ELSE
%token ENDIF
%token TRUE
%token FALSE
%token WHILE
%token ENDWHILE
%token CONTINUE
%token BREAK
%token FOR
%token ENDFOR
%token ASSIGN_OP

%%

program: PROGRAM id _BEGIN pgm_body END 

id: IDENTIFIER 
	{}
pgm_body: decl 
	{
		SymbolTable * table = new SymbolTable("GLOBAL", $1, Type::GLOBAL);
		stackTable->enqueue(table);
	}
	func_declarations 
	
decl: { $$ = NULL; } | string_decl decl 
	{
		Identifier * id = $1;
		$$ = $1;
		id->getLast()->next = $2;
	}
	  | var_decl decl
	{
		Identifier * id = $1;
		$$ = $1;
		id->getLast()->next = $2;
	}

string_decl: STRING id ASSIGN_OP str ';'
	{ 
		IdString * idString = new IdString();
		idString->name = ($2)->value;
		idString->value = ($4)->value;
		$$ = idString;
	}
str: STRINGLITERAL
	{}

var_decl: var_type id_list ';' 
	{
		Identifier * head;
		string type = ($1)->value;

		if (!type.compare("FLOAT")) {
			head = new IdFloat();
		} else {
			head = new IdInteger();
		}

		Identifier * id = head;
		LLString * stringList = $2;

		while(stringList != NULL) {
			string idName = stringList->value;
			
			id->name = idName;
			stringList = stringList->next;

			if (stringList != NULL) {
				if (!type.compare("FLOAT")) {
					id->next = new IdFloat();
				} else {
					id->next = new IdInteger();
				}
				id = id->next;
			}
		}
		$$ = head;
	}
var_type: FLOAT { $$ = new LLString("FLOAT"); } | 
		  INT   { $$ = new LLString("INT"); }
any_type: var_type | VOID 
	{}
id_list: id id_tail 
	{
		$$ = $1;
		$$->next = $2;
	}
id_tail: { $$ = NULL; } | ',' id_list
	{
		$$ = $2;
	}

param_decl_list: { $$ = NULL; } | param_decl param_decl_tail
	{
		$$ = $1;
		$$->next = $2;
	}
param_decl: var_type id
	{
		Identifier * id;
		if (!($1)->value.compare("FLOAT")) {
			id = new IdFloat();
		} else {
			id = new IdInteger();
		}
		id->name = ($2)->value;
		$$ = id;
	}
param_decl_tail: { $$ = NULL; } | ',' param_decl param_decl_tail
	{
		$$ = $2;
		$$->next = $3;
	}

func_declarations: | func_decl func_declarations
	{

	}
func_decl: FUNCTION any_type id '(' param_decl_list ')' _BEGIN decl 
	{
		Identifier * parms = $5;
		if($5 != NULL) {
			while(parms->next != NULL) {
				parms = parms->next;
			}
			parms->next = $8;
			parms = $5;
		} else {
			parms = $8;
		}
		stackTable->enqueue(new SymbolTable(($3)->value, parms, Type::FUNC));
	} stmt_list END

stmt_list: | stmt stmt_list
	{}
stmt: base_stmt | if_stmt | loop_stmt
	{}
base_stmt: assign_stmt | read_stmt | write_stmt | control_stmt
	{}

assign_stmt: assign_expr ';'
	{}
assign_expr: id ASSIGN_OP expr
	{}
read_stmt: READ '(' id_list ')'';'
	{}
write_stmt: WRITE '(' id_list ')'';'
	{}
return_stmt: RETURN expr ';'
	{}

expr: expr_prefix factor
	{}
expr_prefix: | expr_prefix factor addop
	{}
factor: factor_prefix post_fix_expr
	{}
factor_prefix: | factor_prefix post_fix_expr mulop
	{}
post_fix_expr: primary | call_expr
	{}
call_expr: id '(' expr_list ')'
	{}
expr_list: | expr expr_list_tail
	{}
expr_list_tail: | ',' expr expr_list_tail
	{}
primary: '(' expr ')' | id | INTLITERAL | FLOATLITERAL
	{}
addop: '+' | '-'
	{}
mulop: '*' | '/'
	{}

if_stmt: 
	IF '(' cond ')' decl { addControlDecl($5); } stmt_list else_part ENDIF

else_part: | ELSE decl { addControlDecl($2); } stmt_list
	
cond: expr compop expr | TRUE | FALSE
	{}
compop: '<' | '>' | '=' | '!''=' | '<''=' | '>''='
	{}
while_stmt: WHILE '(' cond ')' decl { addControlDecl($5); } stmt_list ENDWHILE

control_stmt: return_stmt | CONTINUE ';' | BREAK ';'
	{}
loop_stmt: while_stmt | for_stmt
	{}
init_stmt: | assign_expr
	{}
incr_stmt: | assign_expr
	{}
for_stmt: FOR '(' init_stmt ';' cond ';' incr_stmt ')' decl stmt_list ENDFOR
	{}

%%

void addControlDecl(Identifier * id) {
	stackTable->enqueue(new SymbolTable("BLOCK " + to_string(blockCnt), id, Type::CONTROL_STMT));
	blockCnt++;
}
