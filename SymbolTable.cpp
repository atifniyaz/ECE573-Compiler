#include "SymbolTable.hpp"

SymbolTable::SymbolTable(string name, Identifier * ids, st::Type type) {
	this->name = name;
	this->ids = ids;
	this->type = type;
	this->isLegalStr = "";

	this->buildDeclMap(ids);
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
/*
bool SymbolTable::isLegal() {
	if (!this->isLegalStr.empty()) {
		cout << "DECLARATION ERROR " << this->isLegalStr << endl;
		return false;
	} else {
		return true;
	}
}
*/
/*
Identifier * SymbolTable::findIdentifier(string name) {
	if (this->declMap.find(name) != this->declMap.end()) {
		return this->declMap.find(name)->second;
	} else {
		return NULL;
	}
} 
*/
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

void SymbolTable::buildDeclMap(Identifier * ids) {
	this->declMap.clear();
	Identifier * currId = this->ids;
	bool caughtFaulty = false;

	while (currId != NULL) {

		Identifier * currIdCpy = currId;
		string idName = currIdCpy->name;

		if (!caughtFaulty && this->declMap.find(idName) != this->declMap.end()) {
			this->isLegalStr = idName;
			caughtFaulty = true;
		} else {
			this->declMap.insert(pair<string, Identifier *>(idName, currIdCpy));
		}

		currId = currId->next;
	}
}

void SymbolTable::print() {
	for (const auto& any : this->declMap) {
	    Identifier * id = any.second;
	   	if (!id->getType().compare("STRING")) {
			cout << "str " << id->name << " " << ((IdString *) id)->value << endl;
		} else {
			cout << "var " << id->name << endl;
		} 
	}
}