#ifndef UTILITY_HPP
#define UTILITY_HPP

#include<string>

#include "Identifier.hpp"
#include "LLString.hpp"
#include "SymbolTable.hpp"

namespace bu {
	
	Identifier * buildDecl(Identifier * parent, Identifier * child);
	Identifier * buildDeclFromList(LLString * type, LLString * list);

	IdString * buildString(LLString * name, LLString * value);

	SymbolTable * addControlDecl(Identifier * id);
	SymbolTable * addFuncDecl(Identifier * bodyDecl, Identifier * argsDecl, string name);
}

#endif