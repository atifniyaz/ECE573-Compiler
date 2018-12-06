#include<vector>
#include<string>
#include<iostream>
#include<set>

#include "AST.hpp"
#include "Identifier.hpp"
#include "SymbolTableStack.hpp"
#include "CodeObject.hpp"

using namespace std;
using namespace tac;
using namespace ast;

int temporaryCnt = 0;

extern SymbolTableStack * stackTable;
extern SymbolTableStack * declStack;

tac::CodeLine::CodeLine(string arg1, string arg2, string arg3, string arg4) {
	this->arg1 = arg1;
	this->arg2 = arg2;
	this->arg3 = arg3;
	this->arg4 = arg4;
}

void tac::CodeObject::addLine(CodeLine * line) {
	this->codeList.push_back(line);
}

void tac::CodeObject::addRegister(string reg) {
	this->tempReg.insert(reg);
}

string tac::CodeObject::getType() {
	if (this->type == ast::Type::STR_VAL) {
		return "s";
	} else if (this->type == ast::Type::INT_VAL) {
		return "i";
	} else {
		return "r";
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
	
	merged->tempReg.insert(left->tempReg.begin(), left->tempReg.end());
	merged->tempReg.insert(right->tempReg.begin(), right->tempReg.end());
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
		merged->addRegister("r" + to_string(temporaryCnt));

		merged->temporary = temporaryCnt;
		merged->type = node->type;

		temporaryCnt++;
	} else if (node->type == ast::Type::ASSIGNMENT) {
		// left is var assign
		// right is expr
		ASTNode_Identifier * leftNode = (ASTNode_Identifier *) node->left;
		string rightLoc;
		if (node->right->type == ast::Type::ID_FIER) {
			// make a temporary for right var

			ASTNode_Identifier * nodeNode = (ASTNode_Identifier *) node->right;

			merged->addLine(new tac::CodeLine(
				"move",
				nodeNode->idName,
				"r" + to_string(temporaryCnt), ""
			));

			merged->addRegister("r" + to_string(temporaryCnt));
			merged->temporary = temporaryCnt;
			merged->type = node->type;

			rightLoc = "r" + to_string(temporaryCnt);
			temporaryCnt++;

		} else {
			rightLoc = "r" + to_string(right->temporary);
		}

		merged->addLine(new tac::CodeLine(
			"move",
			rightLoc,
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
		merged->addRegister("r" + to_string(temporaryCnt));

		temporaryCnt++;
	} else if (node->type == ast::Type::ID_FIER) {
		ASTNode_Identifier * idNode = (ASTNode_Identifier *) node;
		Identifier * id = declStack->findIdentifier(idNode->idName);	

		if (id != NULL) {
			string idType = id->getType();

			if (!idType.compare("INT")) {
				merged->type = ast::Type::INT_VAL;
			} else if (!idType.compare("FLOAT")) {
				merged->type = ast::Type::FLOAT_VAL;
			} else {
				merged->type = ast::Type::STR_VAL;
			}
			idNode->idName = id->name;
		} else {
			// Strange enough, this identifier doesn't exist???
		}
	} else if (node->type == ast::Type::COMPARATOR) {

		ASTNode_Comparator * astComp = (ASTNode_Comparator *) node;

		string leftStorage = getName(node->left, left);
		string rightStorage = getName(node->right, right);
		string cmpOpr;

		if (left->type != right->type) {
			cmpOpr = "r";
		} else {
			cmpOpr = left->type == ast::Type::FLOAT_VAL ? "r" : "i";
		}

		if (node->left->type == ast::Type::ID_FIER &&
			node->right->type == ast::Type::ID_FIER) {
			// We will create a temporary just for right

			ASTNode_Identifier * nodeNode = (ASTNode_Identifier *) node->right;

			merged->addLine(new tac::CodeLine(
				"move",
				nodeNode->idName,
				"r" + to_string(temporaryCnt), ""
			));

			merged->addRegister("r" + to_string(temporaryCnt));
			merged->temporary = temporaryCnt;
			merged->type = node->type;

			rightStorage = "r" + to_string(temporaryCnt);
			temporaryCnt++;
		} else if(node->right->type == ast::Type::ID_FIER &&
			(node->left->type == ast::Type::INT_VAL ||
			 node->left->type == ast::Type::FLOAT_VAL)) {
			// Flip the two
			// Reverse the operator
			string tempStorage = leftStorage;
			leftStorage = rightStorage;
			rightStorage = tempStorage;

			astComp->comp = astComp->oppo;
		}

		merged->addLine(new tac::CodeLine(
			"cmp" + cmpOpr,
			leftStorage, rightStorage, ""
		));

	} else if (node->type == ast::Type::FUNC_CALL) {
		ASTNode_Expr_List * exprNode = (ASTNode_Expr_List *) node->right;
		ASTNode_Identifier * idNode = (ASTNode_Identifier *) node->left;

		CodeObject * exprCode = new CodeObject();
		CodeObject * regPushCode = new CodeObject();

		// Expression 3AC
		for(int i = 0; i < exprNode->exprList.size(); i++) {
			ASTNode * myExprNode = exprNode->exprList[i];
			CodeObject * exprIndCode = new CodeObject();

			if (myExprNode->type == ast::Type::ID_FIER) {

				ASTNode_Identifier * nodeNode = (ASTNode_Identifier *) myExprNode;
				exprIndCode->addLine(new tac::CodeLine(
					"move", 
					declStack->findIdentifier(nodeNode->idName)->name,
					"r" + to_string(temporaryCnt), ""
				));

				regPushCode->addRegister("r" + to_string(temporaryCnt));
				temporaryCnt++;
			} else {
				exprIndCode = tac::buildTAC(myExprNode);
				string regFinal = *(exprIndCode->tempReg.rbegin());
				exprIndCode->tempReg.clear();
				regPushCode->addRegister(regFinal);
			}
			
			//exprIndCode->print();
			exprCode = tac::merge(exprCode, exprIndCode);

			//cout << "------" << endl;
		}

		merged = tac::merge(merged, exprCode);

		set<string>::reverse_iterator rt;

		// Push Registers
		for(auto& reg : merged->tempReg) {
			merged->addLine(new tac::CodeLine("push", reg, "", ""));
		}

		// Push Args
		for(auto& reg : regPushCode->tempReg) {
			merged->addLine(new tac::CodeLine("push", reg, "", ""));
		}

		merged->addLine(new tac::CodeLine("push", "", "", ""));
		
		// Call Function
		merged->addLine(new tac::CodeLine("jsr", "FUNC_ID_" + idNode->idName, "", ""));
		merged->addLine(new tac::CodeLine("pop", "r" + to_string(temporaryCnt), "", ""));

		// Pop Args
		for(rt = regPushCode->tempReg.rbegin(); rt != regPushCode->tempReg.rend(); ++rt) {
			merged->addLine(new tac::CodeLine("pop", *rt, "", ""));
		}

		// Pop Registers
		for(rt = merged->tempReg.rbegin(); rt != merged->tempReg.rend(); ++rt) {
			merged->addLine(new tac::CodeLine("pop", *rt, "", ""));
		}
		
		// Add New Register for Return Value
		merged->addRegister("r" + to_string(temporaryCnt));
		merged->temporary = temporaryCnt;
		temporaryCnt++;
	}
	return merged;
}
