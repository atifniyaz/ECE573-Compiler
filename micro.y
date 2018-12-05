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
int labelCnt = 0;
int outCnt = 0;
int whileCnt = 0;

%}

%error-verbose

%union {
	Identifier * identifier;
	LLString * stringList;
	ast::ASTNode * astNode;
	tac::CodeObject * codeObject;
	int intVal;
	float flVal;
}

%type <identifier> string_decl var_decl decl param_decl param_decl_list param_decl_tail
%type <stringList> id_list id_tail var_type id str
%type <astNode> addop mulop expr expr_prefix factor factor_prefix post_fix_expr primary compop cond 
%type <codeObject> stmt stmt_list base_stmt if_stmt loop_stmt assign_stmt read_stmt write_stmt control_stmt assign_expr else_part while_stmt
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
	masterCode->addLine(new tac::CodeLine("sys halt", "", "", ""));
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
	stmt_list END {
		masterCode = tac::merge(masterCode, $10);
	}

stmt_list: { $$ = NULL; } | stmt stmt_list
	{
		$$ = tac::merge($1, $2);
	}
stmt: base_stmt {
		$$ = $1;
	} | if_stmt {
		$$ = $1;
	} | loop_stmt {
		$$ = $1;
	}
base_stmt: assign_stmt { $$ = $1; } | 
		   read_stmt { $$ = $1; } | 
		   write_stmt { $$ = $1; } | 
		   control_stmt { $$ = $1; }

assign_stmt: assign_expr ';'
	{
		$$ = $1;
	}
assign_expr: id ASSIGN_OP expr
	{
		ASTNode * node = new ast::ASTNode_Assignment();
		node->left = new ast::ASTNode_Identifier($1->value);
		node->right = $3;

		$$ = tac::buildTAC(node);
	}
read_stmt: READ '(' id_list ')'';'
	{
		LLString * list = $3;
		tac::CodeObject * readCode = new tac::CodeObject();

		while (list != NULL) {
			string idName = list->value;
			Identifier * id = stackTable->findIdentifier(idName);
			string prefix;

			if (!id->getType().compare("INT")) {
				prefix = "i";
			} else if(!id->getType().compare("FLOAT")) {
				prefix = "r";
			} else {
				prefix = "s";
			}

			readCode->addLine(new tac::CodeLine(
				"sys read" + prefix,
				idName, "", ""));
			list = list->next;
		}
		$$ = readCode;
	}
write_stmt: WRITE '(' id_list ')'';'
	{
		LLString * list = $3;
		tac::CodeObject * writeCode = new tac::CodeObject();

		while (list != NULL) {
			string idName = list->value;
			Identifier * id = stackTable->findIdentifier(idName);
			string prefix;

			if (!id->getType().compare("INT")) {
				prefix = "i";
			} else if(!id->getType().compare("FLOAT")) {
				prefix = "r";
			} else {
				prefix = "s";
			}

			writeCode->addLine(new tac::CodeLine(
				"sys write" + prefix, 
				idName, "", ""));
			list = list->next;
		}
		$$ = writeCode;
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
	IF '(' cond ')' decl { addControlDecl($5); } stmt_list else_part ENDIF {
		if ($3->type == ast::Type::BOOLEAN) {
			ASTNode_Boolean * boolNode = (ASTNode_Boolean *) $3;
			if (boolNode->isTrue) {
				$$ = $7;
			} else {
				$$ = $8;
			}
		} else {
			// conditionCode, comparatorCode, codeTrue, jumpCode, codeFalse, outCode
			tac::CodeObject * codeTrue = $7;
			tac::CodeObject * codeFalse = $8;
			tac::CodeObject * conditionCode = tac::buildTAC($3);

			tac::CodeObject * comparator = new tac::CodeObject();
			tac::CodeObject * jump = new tac::CodeObject();
			tac::CodeObject * out = new tac::CodeObject();

			ASTNode_Comparator * _compExpr = (ASTNode_Comparator *) $3;

			comparator->addLine(new tac::CodeLine(
				_compExpr->comp, "ELSE_" + to_string(labelCnt), "", ""
			)); // Comparator code
			jump->addLine(new tac::CodeLine(
				"jmp", "OUT_" + to_string(outCnt), "", ""
			));
			jump->addLine(new tac::CodeLine(
				"label", "ELSE_" + to_string(labelCnt), "", ""
			)); // Jump code
			out->addLine(new tac::CodeLine(
				"label", "OUT_" + to_string(outCnt), "", ""
			));
			outCnt++;
			labelCnt++;

			tac::CodeObject * merged = tac::merge(conditionCode, comparator);
			merged = tac::merge(merged, codeTrue);
			merged = tac::merge(merged, jump);
			merged = tac::merge(merged, codeFalse);
			merged = tac::merge(merged, out);

			$$ = merged;
		}
	}

else_part: { $$ = NULL; } | ELSE decl { addControlDecl($2); } stmt_list {
	$$ = $4;
}
	
cond: expr compop expr {
		$2->left = $1;
		$2->right = $3;
		$$ = $2;
	} | TRUE { 
		$$ = new ASTNode_Boolean(true);
	} | FALSE {
		$$ = new ASTNode_Boolean(false);
	}

/* Comparators use the OPPOSITE tiny command */
compop: '<' { $$ = new ast::ASTNode_Comparator("jge"); } | 
  		'>' { $$ = new ast::ASTNode_Comparator("jle"); } | 
  		'=' { $$ = new ast::ASTNode_Comparator("jne"); } | 
  		'!''=' { $$ = new ast::ASTNode_Comparator("jeq"); } | 
  		'<''=' { $$ = new ast::ASTNode_Comparator("jlt"); } | 
  		'>''=' { $$ = new ast::ASTNode_Comparator("jgt"); }
while_stmt: WHILE '(' cond ')' decl { addControlDecl($5); } stmt_list ENDWHILE {
	
	tac::CodeObject * loopCode = $7;
	tac::CodeObject * conditionCode;
	tac::CodeObject * labelWhile = new tac::CodeObject();
	tac::CodeObject * comparator = new tac::CodeObject();
	tac::CodeObject * jumpOut = new tac::CodeObject();

	labelWhile->addLine(new tac::CodeLine(
		"label", "WHILE_" + to_string(whileCnt), "", ""
	)); // Comparator code
	jumpOut->addLine(new tac::CodeLine(
		"jmp", "WHILE_" + to_string(whileCnt), "", ""
	));
	jumpOut->addLine(new tac::CodeLine(
		"label", "OUT_" + to_string(outCnt), "", ""
	)); // Loop back code & Exit

	if ($3->type == ast::Type::BOOLEAN) {
		ASTNode_Boolean * boolNode = (ASTNode_Boolean *) $3;
		if (boolNode->isTrue) {
			tac::CodeObject * allTrue = tac::merge(labelWhile, loopCode);
			$$ = tac::merge(allTrue, jumpOut);
		} else {
			$$ = NULL;
		}
	} else {
		conditionCode = tac::buildTAC($3);
		ASTNode_Comparator * _compExpr = (ASTNode_Comparator *) $3;
		
		comparator->addLine(new tac::CodeLine(
			_compExpr->comp, "OUT_" + to_string(outCnt), "", ""
		));

		tac::CodeObject * merged = tac::merge(labelWhile, conditionCode);
		merged = tac::merge(merged, comparator);
		merged = tac::merge(merged, loopCode);
		merged = tac::merge(merged, jumpOut);

		$$ = merged;
	}
}

control_stmt: return_stmt {
		$$ = NULL;
	} | CONTINUE ';' {
		$$ = NULL;
	} | BREAK ';' {
		$$ = NULL;
	}
loop_stmt: while_stmt { $$ = $1; } | for_stmt { $$ = NULL; }
init_stmt: | assign_expr
	{}
incr_stmt: | assign_expr
	{}
for_stmt: FOR '(' init_stmt ';' cond ';' incr_stmt ')' decl stmt_list ENDFOR
	{}

%%
