#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include<string>
#include<iostream>
#include<map>

#include "Identifier.hpp"

using namespace std;

namespace st {
	enum class Type { GLOBAL, FUNC, CONTROL_STMT };
}

class SymbolTable {
public:
	string name;
	Identifier * ids;
	st::Type type;
	int count;

	map<string, Identifier *> declMap;
	string isLegalStr;

	SymbolTable();
	SymbolTable(string name, Identifier * ids, st::Type type);

	bool isLegal();
	void print();
	void reassignArgs(Identifier * ids);
	Identifier * findIdentifier(string name);

private:
	void buildDeclMap(Identifier * ids);
};

#endif