#include <string>
#include <iostream>

#include "AST.hpp"

using namespace std;

ast::ASTNode::ASTNode() {
	this->left = NULL;
	this->right = NULL;
}

void printDepth(int depth) {
	for(int i = 0; i < depth; i++) {
		cout << ".";
	}
}

void ast::ASTNode::print(int depth) {
	if (this->left != NULL) {
		printDepth(depth + 1);
		cout << "LEFT" << endl;
		this->left->print(depth + 1);
	}
	if (this->right != NULL) {
		printDepth(depth + 1);
		cout << "RIGHT" << endl;
		this->right->print(depth + 1);
	}

	printDepth(depth);
	this->onPrint();
	cout << endl;

}

ast::ASTNode_AddExpr::ASTNode_AddExpr(bool isAddition) {
	this->isAddition = isAddition;
	this->type = ast::Type::ADD_EXPR;
}

void ast::ASTNode_AddExpr::onPrint() {
	cout << (this->isAddition ? "+" : "-") << " ";
}

ast::ASTNode_MulExpr::ASTNode_MulExpr(bool isMultiplication) {
	this->isMultiplication = isMultiplication;
	this->type = ast::Type::MUL_EXPR;
}

void ast::ASTNode_MulExpr::onPrint() {
	cout << (this->isMultiplication ? "*" : "/") << " ";
}

ast::ASTNode_INT::ASTNode_INT(int value) {
	this->value = value;
	this->type = ast::Type::INT_VAL;
}

void ast::ASTNode_INT::onPrint() {
	cout << this->value << " ";
}

ast::ASTNode_FLOAT::ASTNode_FLOAT(float value) {
	this->value = value;
	this->type = ast::Type::FLOAT_VAL;
}

void ast::ASTNode_FLOAT::onPrint() {
	cout << this->value << " ";
}

ast::ASTNode_Identifier::ASTNode_Identifier(string idName) {
	this->idName = idName;
	this->type = ast::Type::ID_FIER;
}

void ast::ASTNode_Identifier::onPrint() {
	cout << this->idName << " ";
}

void ast::ASTNode_Assignment::onPrint() {
	cout << "=" << " ";
}
