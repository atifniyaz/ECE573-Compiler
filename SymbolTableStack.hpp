#ifndef SYMBOL_TABLE_STACK_HPP
#define SYMBOL_TABLE_STACK_HPP

#include<string>
#include "SymbolTable.hpp"

class SymbolTableHolder {

public:

	SymbolTableHolder * next;
	SymbolTable * value;

	SymbolTableHolder(SymbolTable * value);
};

class SymbolTableStack {

public:
	SymbolTableHolder * head;
	SymbolTableHolder * rear;

	SymbolTableStack();


	SymbolTable * peek();

	SymbolTable * pop();
	void push(SymbolTable * value);

	void enqueue(SymbolTable * value);
	SymbolTable * poll();

	void print();
	Identifier * findIdentifier(string name);
	bool isEmpty();
};

#endif
