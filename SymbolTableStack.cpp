#include "SymbolTableStack.hpp"

SymbolTableHolder::SymbolTableHolder(SymbolTable * value) {
	next = NULL;
	this->value = value;
}

SymbolTableStack::SymbolTableStack() {
	this->head = NULL;
}

SymbolTable * SymbolTableStack::peek() {
	return this->head->value;
}

// Treat as Stack
void SymbolTableStack::push(SymbolTable * value) {
	SymbolTableHolder * node = new SymbolTableHolder(value);
	node->next = this->head;
	this->head = node;
}

SymbolTable * SymbolTableStack::pop() {
	if (this->head == NULL) {
		return NULL;
	}

	SymbolTableHolder * head = this->head;
	this->head = head->next;
	return head->value;
}

// Treat as Queue
void SymbolTableStack::enqueue(SymbolTable * value) {
	SymbolTableHolder * node = new SymbolTableHolder(value);
	if (this->head == NULL) { // So will prev
		this->head = node;
	} else {
		this->rear->next = node;
	}
	this->rear = node;
}

SymbolTable * SymbolTableStack::poll() {
	SymbolTable * node = peek();
	if (this->head == this->rear) {
		this->head = NULL;
		this->rear = NULL;
		return node;
	} else {
		return pop();
	}
}

void SymbolTableStack::print() {
	SymbolTableHolder * head = this->head;
	while(head != NULL) {
		cout << head->value->name << endl;
		head = head->next;
	}
}

bool SymbolTableStack::isEmpty() {
	return this->head == NULL;
}

Identifier * SymbolTableStack::findIdentifier(string name) {
	Identifier * idId = NULL;
	SymbolTableHolder * head = this->head;
	while (idId == NULL && head != NULL) {
		idId = head->value->findIdentifier(name);
		head = head->next;
	}
	return idId;
}