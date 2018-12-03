#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include<string>
#include<iostream>

#include "Identifier.hpp"

using namespace std;

enum class Type { GLOBAL, FUNC, CONTROL_STMT };

class SymbolTable {
public:
	string name;
	Identifier * ids;
	Type type;

	SymbolTable(string name, Identifier * ids, Type type);

	bool isLegal();
	void print();
};

#endif