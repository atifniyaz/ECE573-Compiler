#ifndef CODE_OBJECT_HPP
#define CODE_OBJECT_HPP

#include<vector>
#include<string>

#include "AST.hpp"

using namespace std;
using namespace ast;

namespace tac {

	class CodeObject {
	public:
		vector<string> codeList;
		int temporary;
		ast::Type type; // more specifically for value type (int, float, str)

		void addLine(string line);
		string getType();
	};

	CodeObject * merge(CodeObject * left, CodeObject * right);
	CodeObject * buildTAC(ASTNode * node);
}

#endif