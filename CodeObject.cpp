#include<vector>
#include<string>
#include<iostream>

#include "AST.hpp"
#include "Identifier.hpp"
#include "SymbolTableStack.hpp"
#include "CodeObject.hpp"

using namespace std;
using namespace tac;
using namespace ast;

int temporaryCnt = 0;

extern SymbolTableStack * stackTable;

tac::CodeLine::CodeLine(string arg1, string arg2, string arg3, string arg4) {
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->arg3 = arg3;
	this->arg4 = arg4;
}

void tac::CodeObject::addLine(CodeLine * line) {
	this->codeList.push_back(line);
}

string tac::CodeObject::getType() {
	if (this->type == ast::Type::INT_VAL) {
		return "i";
	} else if (this->type == ast::Type::FLOAT_VAL) {
		return "r";
	} else {
		return "s";
	}
}

void tac::CodeObject::print() {
	for(int i = 0; i < this->codeList.size(); i++) {
		cout << this->codeList[i]->stringify() << endl;
	}
}

string getName(ASTNode * node, CodeObject * code) {
	if (node->type == ast::Type::ID_FIER) {
		ASTNode_Identifier * nodeNode = (ASTNode_Identifier *) node;
		return nodeNode->idName;
	} else {
		return "r" + to_string(code->temporary);
	}
}

CodeObject * tac::merge(CodeObject * left, CodeObject * right) {
	if (left == NULL && right != NULL) {
		return right;
	}  
	if (right == NULL && left != NULL) {
		return left;
	}
	CodeObject * merged = new CodeObject();
	if (left == NULL && right == NULL) {
		return merged;
	}
	merged->codeList.reserve(left->codeList.size() + right->codeList.size());
	merged->codeList.insert(merged->codeList.end(), left->codeList.begin(), left->codeList.end());
	merged->codeList.insert(merged->codeList.end(), right->codeList.begin(), right->codeList.end());
	return merged;
}

CodeObject * tac::buildTAC(ASTNode * node) {
	CodeObject * left = NULL;
	CodeObject * right = NULL;

	if (node->left != NULL) {
		left = buildTAC(node->left);
	}
	if (node->right != NULL) {
		right = buildTAC(node->right);
	}

	CodeObject * merged = tac::merge(left, right);

	if (node->type == ast::Type::INT_VAL ||
		node->type == ast::Type::FLOAT_VAL) {

		merged->addLine(new tac::CodeLine(
			"move",
			node->getTAC(),
			"r" + to_string(temporaryCnt), ""
		));

		merged->temporary = temporaryCnt;
		merged->type = node->type;

		temporaryCnt++;
	} else if (node->type == ast::Type::ASSIGNMENT) {
		// left is var assign
		// right is expr
		ASTNode_Identifier * leftNode = (ASTNode_Identifier *) node->left;
		merged->addLine(new tac::CodeLine(
			"move",
			"r" + to_string(right->temporary),
			leftNode->idName,
			""
		));
		merged->type = node->left->type;

	} else if (node->type == ast::Type::MUL_EXPR || 
		node->type == ast::Type::ADD_EXPR) {
		// left / right could be identifiers
		string leftStorage = getName(node->left, left);
		string rightStorage = getName(node->right, right);
		string tacPrefix;
		ast::Type valType;

		if (node->type == ast::Type::MUL_EXPR) {
			ASTNode_MulExpr * exprNode = (ASTNode_MulExpr *) node;
			tacPrefix = exprNode->isMultiplication ? "mul" : "div";
		} else {
			ASTNode_AddExpr * exprNode = (ASTNode_AddExpr *) node;
			tacPrefix = exprNode->isAddition ? "add" : "sub";
		}
		if (left->type != right->type) {
			valType = ast::Type::FLOAT_VAL;
		} else {
			valType = left->type;
		}

		merged->type = valType;
		merged->addLine(new tac::CodeLine(
			"move",
			leftStorage,
			"r" + to_string(temporaryCnt), ""
		)); // MUL A B R0 (MOV A RN)
		merged->addLine(new tac::CodeLine(
			tacPrefix + merged->getType(),
			rightStorage,
			"r" + to_string(temporaryCnt), ""
		)); // MUL A B RO (MUL B RN)
		merged->temporary = temporaryCnt;

		temporaryCnt++;
	} else if (node->type == ast::Type::ID_FIER) {
		ASTNode_Identifier * idNode = (ASTNode_Identifier *) node;
		Identifier * id = stackTable->findIdentifier(idNode->idName);	

		if (id != NULL) {
			string idType = id->getType();

			if (!idType.compare("INT")) {
				merged->type = ast::Type::INT_VAL;
			} else if (!idType.compare("FLOAT")) {
				merged->type = ast::Type::FLOAT_VAL;
			} else {
				merged->type = ast::Type::STR_VAL;
			}
		} else {
			// Strange enough, this identifier doesn't exist???
		}
	} else if (node->type == ast::Type::COMPARATOR) {
		string leftStorage = getName(node->left, left);
		string rightStorage = getName(node->right, right);
		string cmpOpr = left->type != right->type ? "r" : "i";

		merged->addLine(new tac::CodeLine(
			"cmp" + cmpOpr,
			leftStorage, rightStorage, ""
		));

	}
	return merged;
}
