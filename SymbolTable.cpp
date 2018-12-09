#include "SymbolTable.hpp"

SymbolTable::SymbolTable(string name, Identifier * ids, st::Type type) {
	this->name = name;
	this->ids = ids;
	this->type = type;
	this->isLegalStr = "";
	this->count = 1;

	this->buildDeclMap(ids);
}

SymbolTable::SymbolTable() {
	this->name = "GLOBAL";
	this->type = st::Type::GLOBAL;
}

bool SymbolTable::isLegal() {
	if (!this->isLegalStr.empty()) {
		cout << "DECLARATION ERROR " << this->isLegalStr << endl;
		return false;
	} else {
		return true;
	}
}

Identifier * SymbolTable::findIdentifier(string name) {
	if (this->declMap.find(name) != this->declMap.end()) {
		return this->declMap.find(name)->second;
	} else {
		return NULL;
	}
} 

void SymbolTable::reassignArgs(Identifier * ids) {
	int counter = 3;
	/****
	Stack Design
	args    <-- $7
	ret val <-- $6
	r3 <-- $5
	r2 <-- $4
	r1 <-- $3
	r0 <-- $2
	fp
	*/
	while(ids != NULL) {
		ids->name = "$" + to_string(counter++);
		ids = ids->next;
	}
}

void SymbolTable::buildDeclMap(Identifier * ids) {
	this->declMap.clear();
	bool caughtFaulty = false;

	while (ids != NULL) {

		Identifier * currIdCpy = ids;
		string idName = currIdCpy->name;

		if (!currIdCpy->getType().compare("STRING")) {
			currIdCpy->name = this->name + "_" + currIdCpy->name;
		} else if (type != st::Type::GLOBAL) {
			// not a global identifier, thus generate a variable name on the stack
			currIdCpy->name = "$-" + to_string(this->count);
			this->count++;
		}

		if (!caughtFaulty && this->declMap.find(idName) != this->declMap.end()) {
			this->isLegalStr = idName;
			caughtFaulty = true;
		} else {
			this->declMap.insert(pair<string, Identifier *>(idName, currIdCpy));
		}

		ids = ids->next;
	}
}

void SymbolTable::print() {
	for (const auto& any : this->declMap) {
	    Identifier * id = any.second;
	   	if (!id->getType().compare("STRING")) {
			cout << "str " << id->name << " " << ((IdString *) id)->value << endl;
		} else if(this->type == st::Type::GLOBAL) {
			cout << "var " << id->name << endl;
		} 
	}
}