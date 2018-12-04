#include<iostream>

#include "AST.hpp"
#include "CodeObject.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"

#include "micro.h"

SymbolTableStack * stackTable = new SymbolTableStack();
tac::CodeObject * masterCode = new tac::CodeObject();
tac::CodeObject * tinyCode = new tac::CodeObject();

extern FILE * yyin;

extern "C" int yyparse();
void parseSymbolTable();
void parseSymbolTable();

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

	if (yyparse()) {
		cout << "Not Accepted";
	} else {
		parseSymbolTable();

		vector<tac::CodeLine *> lines = masterCode->codeList;

		for(int i = 0; i < lines.size(); i++) {
			cout << lines[i]->stringify() << endl;
		}
	}
	return 0;
}

void yyerror(char *s) { 
	cout << s << endl;
}

void parseSymbolTable() {
	SymbolTableStack * holder = new SymbolTableStack();
	while (!stackTable->isEmpty()) {
		SymbolTable * data = stackTable->poll();
		if (data->isLegal()) {
			holder->enqueue(data);
		} else {
			return;
		}
	}

	while (!holder->isEmpty()) {
		holder->poll()->printVar();
	}
}