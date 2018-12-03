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

void tac::CodeObject::addLine(string line) {
	this->codeList.push_back(line);
}

string tac::CodeObject::getType() {
	if (this->type == ast::Type::INT_VAL) {
		return "I ";
	} else if (this->type == ast::Type::FLOAT_VAL) {
		return "F ";
	} else {
		return "S ";
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
		merged->addLine(node->getTAC() + to_string(temporaryCnt));
		merged->temporary = temporaryCnt;
		merged->type = node->type;

		temporaryCnt++;
	} else if (node->type == ast::Type::ASSIGNMENT) {
		// left is var assign
		// right is expr
		ASTNode_Identifier * leftNode = (ASTNode_Identifier *) node->left;
		string tac = "move" " r" + to_string(right->temporary) + " " + leftNode->idName;
		merged->addLine(tac);
		merged->type = node->left->type;

	} else if (node->type == ast::Type::MUL_EXPR || 
		node->type == ast::Type::ADD_EXPR) {
		// left / right could be identifiers
		string leftStorage;
		string rightStorage;
		string tacPrefix;
		ast::Type valType;

		if (node->left->type == ast::Type::ID_FIER) {
			ASTNode_Identifier * leftNode = (ASTNode_Identifier *) node->left;
			leftStorage = leftNode->idName;
		} else {
			leftStorage = "!T" + to_string(left->temporary);
		}

		if (node->right->type == ast::Type::ID_FIER) {
			ASTNode_Identifier * rightNode = (ASTNode_Identifier *) node->right;
			rightStorage = rightNode->idName;
		} else {
			rightStorage = "!T" + to_string(right->temporary);
		}

		if (node->type == ast::Type::MUL_EXPR) {
			ASTNode_MulExpr * exprNode = (ASTNode_MulExpr *) node;
			tacPrefix = exprNode->isMultiplication ? ";MUL" : ";DIV";
		} else {
			ASTNode_AddExpr * exprNode = (ASTNode_AddExpr *) node;
			tacPrefix = exprNode->isAddition ? ";ADD" : ";SUB";
		}
		if (left->type != right->type) {
			valType = ast::Type::FLOAT_VAL;
		} else {
			valType = left->type;
		}

		merged->type = valType;
		string tac = tacPrefix + merged->getType() + leftStorage + " " + rightStorage + " !T" + to_string(temporaryCnt);
		merged->addLine(tac);
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
	}
	return merged;
}