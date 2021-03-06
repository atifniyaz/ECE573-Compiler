#include<string>
#include "BisonUtility.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"
#include "LLString.hpp"
#include "CodeObject.hpp"

extern SymbolTableStack * stackTable;
extern int blockCnt;

string to_string(int val);

Identifier * buildNumberId(string type) {
	if (!type.compare("FLOAT")) {
		return new IdFloat();
	} else {
		return new IdInteger();
	}
}

SymbolTable * bu::addControlDecl(Identifier * id) {
	SymbolTable * table = new SymbolTable("BLOCK " + to_string(blockCnt), id, st::Type::CONTROL_STMT);
	stackTable->enqueue(table);
	blockCnt++;
	return table;
}

SymbolTable * bu::addFuncDecl(Identifier * bodyDecl, Identifier * argsDecl, string name) {
	Identifier * decl = bodyDecl;
	if(argsDecl != NULL && bodyDecl != NULL) {
		bodyDecl->getLast()->next = argsDecl;
	} else if(argsDecl != NULL) {
		decl = argsDecl;
	}
	SymbolTable * table = new SymbolTable(name, decl, st::Type::FUNC);
	table->reassignArgs(argsDecl);
	stackTable->enqueue(table);
	return table;
}

Identifier * bu::buildDecl(Identifier * parent, Identifier * child) {
	parent->getLast()->next = child;
	return parent;
}

IdString * bu::buildString(LLString * name, LLString * value) {
	IdString * str = new IdString();
	str->name = name->value;
	str->value = value->value;
	return str;
}

Identifier * bu::buildDeclFromList(LLString * type, LLString * list) {
	Identifier * head = buildNumberId(type->value);
	Identifier * buildId = head;

	while(list != NULL) {
		string idName = list->value;
		
		buildId->name = idName;
		list = list->next;

		if (list != NULL) {
			buildId->next = buildNumberId(type->value);
			buildId = buildId->next;
		}
	}
	return head;
}

void bu::findControl(tac::CodeObject * list, tac::CodeLine * contLine, tac::CodeLine * outLine) {
	if(list == NULL) { return; }
	for(int i = 0; i < list->codeList.size(); i++) {
		tac::CodeLine * line = list->codeList[i];
		if(!line->arg1.compare(";CONTINUE")) {
			list->codeList[i] = contLine;
		} else if(!line->arg1.compare(";BREAK")) {
			list->codeList[i] = outLine;
		}
	}
}
