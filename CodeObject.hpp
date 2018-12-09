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
		string arg1;
		string arg2;
		string arg3;
		string arg4;
		
		set<CodeLine *> predecessors;
		set<CodeLine *> successors;

		set<string> genSet;
		set<string> killSet;

		set<string>	inSet;
		set<string>	outSet;

		CodeLine(string arg1, string arg2, string arg3, string arg4);

		string stringify() {
			return arg1 + " " + arg2 + " " + arg3 + " " + arg4;
		}
	};

	class CodeObject {
	public:
		vector<CodeLine *> codeList;
		vector<bool> reset;
		set<string> tempReg;
		int temporary;
		ast::Type type; // more specifically for value type (int, float, str)

		void addLine(CodeLine * line);
		void addRegister(string reg);
		void print();

		void optimizeTiny();
		string getType();

	private:
		void configureCFG();
		void createGenKillSet();
		void createInOutSet();
		void allocateRegisters();
		void createReset();
	};

	CodeObject * merge(CodeObject * left, CodeObject * right);
	CodeObject * buildTAC(ASTNode * node);
}

#endif