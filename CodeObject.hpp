#ifndef CODE_OBJECT_HPP
#define CODE_OBJECT_HPP

#include<vector>
#include<string>
#include<set>

#include "AST.hpp"

using namespace std;
using namespace ast;

namespace tac {

	class CodeLine {
	public:
		CodeLine(string arg1, string arg2, string arg3, string arg4);
		string arg1;
		string arg2;
		string arg3;
		string arg4;
		string stringify() {
			return arg1 + " " + arg2 + " " + arg3 + " " + arg4;
		}
	};

	class CodeObject {
	public:
		vector<CodeLine *> codeList;
		set<string> tempReg;
		int temporary;
		ast::Type type; // more specifically for value type (int, float, str)

		void addLine(CodeLine * line);
		void addRegister(string reg);
		void print();
		string getType();
	};

	CodeObject * merge(CodeObject * left, CodeObject * right);
	CodeObject * buildTAC(ASTNode * node);
}

#endif