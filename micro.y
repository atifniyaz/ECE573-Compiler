%{
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "data.h"

void addChild(struct SymbolTable * parent, struct SymbolTable * child);
struct SymbolTable * peekStack(struct SymbolTableStack * stack);

struct SymbolTableStack * stack = NULL;
int blockCnt = 1;
%}

%union {
	struct Identifier * identifier;
	char * strData;
	int integer;
}

%type <identifier> string_decl var_decl id_list id_tail decl func_body pgm_body param_decl param_decl_list param_decl_tail
%type <strData> id str
%type <integer> var_type 

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
	{
		struct SymbolTable * globalScope = malloc(sizeof(*globalScope));
		globalScope->name = "GLOBAL";
		globalScope->ids = $4;
		globalScope->childLen = 0;
		pushStack(&stack, globalScope);
	}
id: IDENTIFIER 
	{}
pgm_body: decl func_declarations 
	{
		$$ = $1;
	}
decl: { $$ = NULL } | string_decl decl 
	{
		$$ = $1;
		struct Identifier * data = $1;
		while(data->next != NULL) {
			data = data->next;
		}
		data->next = $2;
	}
	  | var_decl decl
	{
		$$ = $1;
		struct Identifier * data = $1;
		while(data->next != NULL) {
			data = data->next;
		}
		data->next = $2;
	}

string_decl: STRING id ASSIGN_OP str ';'
	{ 
		$$ = malloc(sizeof(struct Identifier));
		$$->id = $2;
		$$->type = 0;
		$$->valStr = $4;
	}
str: STRINGLITERAL
	{}

var_decl: var_type id_list ';' 
	{
		struct Identifier * data = $2;
		$$ = data;
		while (data != NULL) {
			data->type = $1;
			data = data->next;
		}
	}
var_type: FLOAT { $$ = 2; } | 
		  INT   { $$ = 1; }
any_type: var_type | VOID 
	{}
id_list: id id_tail 
	{
		$$ = malloc(sizeof(struct Identifier));
		$$->id = $1;
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
		struct Identifier * data = malloc(sizeof(*data));
		$$ = data;
		$$->id = $2;
		$$->type = $1;
		$$->next = NULL;
	}
param_decl_tail: { $$ = NULL; } | ',' param_decl param_decl_tail
	{
		$$ = $2;
		$$->next = $3;
	}

func_declarations: | func_decl func_declarations
	{

	}
func_decl: FUNCTION any_type id '(' param_decl_list ')' _BEGIN func_body END
	{
		struct Identifier * parms = $5;
		if($5 != NULL) {
			while(parms->next != NULL) {
				parms = parms->next;
			}
			parms->next = $8;
			parms = $5;
		} else {
			parms = $8;
		}
		struct SymbolTable * parent = malloc(sizeof(*parent));
		parent->name = $3;
		parent->ids = parms;
		parent->childLen = 0;
		pushStack(&stack, parent);
	}
func_body: decl stmt_list
	{
		$$ = $1;
	}

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

if_stmt: IF '(' cond ')' decl stmt_list else_part ENDIF
	{
		struct SymbolTable * globalScope = malloc(sizeof(*globalScope));
		globalScope->name = "IF";
		globalScope->ids = $5;
		globalScope->childLen = 1;

		pushStack(&stack, globalScope);
	}
else_part: | ELSE decl stmt_list
	{
		struct SymbolTable * globalScope = malloc(sizeof(*globalScope));
		globalScope->name = "ELSE";
		globalScope->ids = $2;
		globalScope->childLen = 1;

		pushStack(&stack, globalScope);
	}
cond: expr compop expr | TRUE | FALSE
	{}
compop: '<' | '>' | '=' | '!''=' | '<''=' | '>''='
	{}
while_stmt: WHILE '(' cond ')' decl stmt_list ENDWHILE
	{
		struct SymbolTable * globalScope = malloc(sizeof(*globalScope));
		globalScope->name = "WHILE";
		globalScope->ids = $5;
		globalScope->childLen = 1;

		pushStack(&stack, globalScope);
	}

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

void pushStack(struct SymbolTableStack ** stk, struct SymbolTable * table) {
	struct SymbolTableStack * head = malloc(sizeof(*head));
	head->value = table;
	head->next = *stk;
	*stk = head;
}

void addChild(struct SymbolTable * parent, struct SymbolTable * child) {
	parent->childLen++;

/*	if (parent->children == NULL) {
		parent->children = malloc(sizeof(*(parent->children)));
	} else {
		realloc(parent->children, sizeof(*(parent->children)) * parent->childLen);
	}
	(parent->children)[parent->childLen - 1] = child; */
}

int isLegalSymbolTable(struct SymbolTable * table) {
	struct Identifier * id = table->ids;
	struct Identifier * idCpy;
	if (id == NULL) { return 1; }
	while (id->next != NULL) {
		idCpy = id->next;
		if (!strcmp(idCpy->id, id->id)) {
			printf("DECLARATION ERROR %s\n", id->id);
			return 0;
		}
		id = id->next;
	}
	return 1;
}

void printSymbolTable(struct SymbolTable * table) {
	struct Identifier * id = table->ids;
	char identifier [7];
	if (table->childLen == 1) {
		printf("Symbol table BLOCK %d\n", blockCnt);
		blockCnt++;
	} else {
		printf("Symbol table %s\n", table->name);
	}
	while (id != NULL) {
		if (id->type != -1) {
			if (id->type == 0) {
				strcpy(identifier, "STRING");
			} else if (id->type == 1) {
				strcpy(identifier, "INT");
			} else if (id->type == 2) {
				strcpy(identifier, "FLOAT");
			}
		}

		if (id->type == 0) {
			printf("name %s type STRING value %s\n", id->id, id->valStr);
		} else {
			printf("name %s type %s\n", id->id, identifier);
		} 

		id = id->next;
	}
	printf("\n");
}

void printStack() {
	struct SymbolTableStack * top = stack;
	while(top != NULL) {
		printf("%s\n", top->value->name);
		top = top->next;
	}
	printf("\n");
}

struct SymbolTable * popStack(struct SymbolTableStack ** stk) {
	struct SymbolTable * head = (*stk)->value;
	(*stk) = (*stk)->next;
	return head;
}

struct SymbolTable * peekStack(struct SymbolTableStack * stack) {
	return stack != NULL ? stack->value : NULL;
}

