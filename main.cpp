#include<iostream>

#include "AST.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"

#include "micro.h"

SymbolTableStack * stackTable = new SymbolTableStack();
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
		holder->poll()->print();
	}
}