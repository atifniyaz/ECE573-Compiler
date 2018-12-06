#include <string>
#include <iostream>

#include "AST.hpp"

using namespace std;

ast::ASTNode::ASTNode() {
	this->left = NULL;
	this->right = NULL;
}

ast::ASTNode_AddExpr::ASTNode_AddExpr(bool isAddition) {
	this->isAddition = isAddition;
	this->type = ast::Type::ADD_EXPR;
}

ast::ASTNode_MulExpr::ASTNode_MulExpr(bool isMultiplication) {
	this->isMultiplication = isMultiplication;
	this->type = ast::Type::MUL_EXPR;
}

ast::ASTNode_INT::ASTNode_INT(int value) {
	this->value = value;
	this->type = ast::Type::INT_VAL;
}

ast::ASTNode_FLOAT::ASTNode_FLOAT(float value) {
	this->value = value;
	this->type = ast::Type::FLOAT_VAL;
}

ast::ASTNode_Identifier::ASTNode_Identifier(string idName) {
	this->idName = idName;
	this->type = ast::Type::ID_FIER;
}

ast::ASTNode_Assignment::ASTNode_Assignment() {
	this->type = ast::Type::ASSIGNMENT;
}

ast::ASTNode_Comparator::ASTNode_Comparator(string comp, string oppo) {
	this->type = ast::Type::COMPARATOR;
	this->comp = comp;
	this->oppo = oppo;
}

ast::ASTNode_Boolean::ASTNode_Boolean(bool isTrue) {
	this->type = ast::Type::BOOLEAN;
	this->isTrue = true;
}

ast::ASTNode_Function_Call::ASTNode_Function_Call() {
	this->type = ast::Type::FUNC_CALL;
}

ast::ASTNode_Expr_List::ASTNode_Expr_List() {
	this->type = ast::Type::EXPR_LIST;
}

void ast::ASTNode_Expr_List::add(ASTNode * node) {
	this->exprList.push_back(node);
}

