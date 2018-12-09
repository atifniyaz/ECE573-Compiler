#include<iostream>

#include "AST.hpp"
#include "CodeObject.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"

extern "C" {
#include "micro.h"
}

SymbolTableStack * stackTable = new SymbolTableStack();
tac::CodeObject * masterCode = new tac::CodeObject();
tac::CodeObject * tinyCode = new tac::CodeObject();

extern "C" FILE * yyin;

extern "C" int yyparse();
bool parseSymbolTable();

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
		if (parseSymbolTable()) {
			vector<tac::CodeLine *> lines = masterCode->codeList;

			for(int i = 0; i < lines.size(); i++) {
				cout << lines[i]->stringify() << endl;
			}
		}
	}
	return 0;
}

void yyerror(const char *s) { 
	cout << s << endl;
}

bool parseSymbolTable() {
	SymbolTableStack * holder = new SymbolTableStack();
	while (!stackTable->isEmpty()) {
		SymbolTable * data = stackTable->poll();
		if (data->isLegal()) {
			holder->enqueue(data);
		} else {
			return false;
		}
	}
	while(!holder->isEmpty()) {
		SymbolTable * data = holder->poll();
		data->print();
		stackTable->enqueue(data);
	}
	return true;
}
