#include <iostream>
#include "Identifier.hpp"

Identifier::Identifier() {
	this->next = NULL;
}

Identifier * Identifier::getLast() {
	Identifier * last = this;
	while (last->next != NULL) {
		last = last->next;
	}
	return last;
}

string IdInteger::getType() {
	return "INT";
}

string IdFloat::getType() {
	return "FLOAT";
}

string IdString::getType() {
	return "STRING";
}

void Identifier::print() {
	Identifier * curr = this;
	while (curr != NULL) {
		cout << curr->name << ": " << curr->getType() << endl;
		curr = curr->next;
	}
	cout << "____" << endl;
}