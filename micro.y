%{
#include<string>
#include<vector>

#include "AST.hpp"
#include "CodeObject.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"
#include "BisonUtility.hpp"

int yylex();
void yyerror(char *s);

extern "C" int yyparse();
extern "C" FILE *yyin;

using namespace std;
using namespace bu;

extern SymbolTableStack * stackTable;

extern tac::CodeObject * masterCode;

int blockCnt = 1;
%}

%error-verbose

%union {
	Identifier * identifier;
	LLString * stringList;
	ast::ASTNode * astNode;
	int intVal;
	float flVal;
}

%type <identifier> string_decl var_decl decl param_decl param_decl_list param_decl_tail
%type <stringList> id_list id_tail var_type id str
%type <astNode> addop mulop expr expr_prefix factor factor_prefix post_fix_expr primary assign_expr
%type <intVal> intval
%type <flVal> fltval

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

program: PROGRAM id _BEGIN pgm_body END {
	masterCode->addLine("sys halt");
}

id: IDENTIFIER 
	{}
pgm_body: decl 
	{
		stackTable->enqueue(new SymbolTable("GLOBAL", $1, st::Type::GLOBAL));
	}
	func_declarations 
	
decl: { $$ = NULL; } | string_decl decl 
	{
		$$ = buildDecl($1, $2);
	}
	  | var_decl decl
	{
		$$ = buildDecl($1, $2);
	}

string_decl: STRING id ASSIGN_OP str ';'
	{ 
		$$ = buildString($2, $4);
	}
str: STRINGLITERAL
	{}
intval: INTLITERAL
	{}
fltval: FLOATLITERAL
	{}
var_decl: var_type id_list ';' 
	{
		$$ = buildDeclFromList($1, $2);
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
func_decl: FUNCTION any_type id '(' param_decl_list ')' _BEGIN 
	decl { addFuncDecl($8, $5, $3->value); } 
	stmt_list END

stmt_list: | stmt stmt_list
	{}
stmt: base_stmt | if_stmt | loop_stmt
	{}
base_stmt: assign_stmt | read_stmt | write_stmt | control_stmt
	{}

assign_stmt: assign_expr ';'
	{}
assign_expr: id ASSIGN_OP expr
	{
		$$ = new ast::ASTNode_Assignment();
		$$->left = new ast::ASTNode_Identifier($1->value);
		$$->right = $3;

		masterCode = tac::merge(masterCode, tac::buildTAC($$));
	}
read_stmt: READ '(' id_list ')'';'
	{
		LLString * list = $3;
		while (list != NULL) {
			string idName = list->value;
			masterCode->addLine(";READI " + idName);
			list = list->next;
		}
	}
write_stmt: WRITE '(' id_list ')'';'
	{
		LLString * list = $3;
		while (list != NULL) {
			string idName = list->value;
			Identifier * id = stackTable->findIdentifier(idName);
			string prefix;

			if (!id->getType().compare("INT")) {
				prefix = "i ";
			} else if(!id->getType().compare("FLOAT")) {
				prefix = "r ";
			} else {
				prefix = "s ";
			}

			masterCode->addLine("sys write" + prefix + idName);
			list = list->next;
		}
	}
return_stmt: RETURN expr ';'
	{}

expr: expr_prefix factor
	{
		if ($1 == NULL) {
			$$ = $2;
		} else {
			$1->right = $2;
			$$ = $1;
		}
	}
expr_prefix: { $$ = NULL; } | expr_prefix factor addop
	{
		if($1 == NULL) {
			$3->left = $2;
		} else {
			$1->right = $2;
			$3->left = $1;
		}
		$$ = $3;
	}
factor: factor_prefix post_fix_expr
	{
		if ($1 == NULL) {
			$$ = $2;
		} else {
			$1->right = $2;
			$$ = $1;
		}
	}
factor_prefix: { $$ = NULL; } | factor_prefix post_fix_expr mulop
	{
		if ($1 == NULL) {
			$3->left = $2;
		} else {
			$1->right = $2;
			$3->left = $1;
		}
		$$ = $3;
	}
post_fix_expr: primary { $$ = $1; } | 
	call_expr { $$ = NULL; }
call_expr: id '(' expr_list ')'
	{}
expr_list: { } | expr expr_list_tail
	{}
expr_list_tail: | ',' expr expr_list_tail
	{}
primary: '(' expr ')' { $$ = $2; } | 
	id { 
		$$ = new ast::ASTNode_Identifier($1->value);
	} | intval {
		$$ = new ast::ASTNode_INT($1);
	} | fltval {
		$$ = new ast::ASTNode_FLOAT($1);
	}
addop: '+' { $$ = new ast::ASTNode_AddExpr(true); } | 
	   '-' { $$ = new ast::ASTNode_AddExpr(false); }
mulop: '*' { $$ = new ast::ASTNode_MulExpr(true); } | 
	   '/' { $$ = new ast::ASTNode_MulExpr(false); }

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
