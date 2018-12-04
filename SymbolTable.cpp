#include "SymbolTable.hpp"

SymbolTable::SymbolTable(string name, Identifier * ids, st::Type type) {
	this->name = name;
	this->ids = ids;
	this->type = type;
}

bool SymbolTable::isLegal() {
	Identifier * currId = this->ids;
	Identifier * iterId;

	if (currId == NULL) { 
		return true; 
	}


	while (currId->next != NULL) {
		iterId = currId->next;
		while (iterId != NULL) {
			if (!currId->name.compare(iterId->name)) {
				cout << "DECLARATION ERROR " << currId->name << endl;
				return false;
			}
			iterId = iterId->next;
		}
		currId = currId->next;
	}
	return true;
}

Identifier * SymbolTable::findIdentifier(string name) {
	Identifier * currId = this->ids;
	while (currId != NULL) {
		if(!currId->name.compare(name)) {
			return currId;
			cout << endl;
		}
		currId = currId->next;
	}
	return NULL;
}

void SymbolTable::print() {
	Identifier * currId = this->ids;
	cout << "Symbol table " << this->name << endl;

	while (currId != NULL) {
		cout << "name " << currId->name << " type " << currId->getType();

		if (!currId->getType().compare("STRING")) {
			cout << " value " << ((IdString *) currId)->value << endl;
		} else {
			cout << endl;
		} 
		currId = currId->next;
	}
	cout << endl;
}

void SymbolTable::printVar() {
	Identifier * currId = this->ids;
	while (currId != NULL) {

		if (!currId->getType().compare("STRING")) {
			cout << "str " << currId->name << " " << ((IdString *) currId)->value << endl;
		} else {
			cout << "var " << currId->name << endl;
		} 
		currId = currId->next;
	}
}