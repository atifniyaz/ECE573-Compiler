#ifndef AST_HPP
#define AST_HPP

#include<string>
#include<vector>

using namespace std;

namespace ast {

	enum class Type {
		ADD_EXPR, MUL_EXPR, INT_VAL, FLOAT_VAL, 
		ID_FIER, ASSIGNMENT, STR_VAL, COMPARATOR, BOOLEAN,
		FUNC_CALL, EXPR_LIST
	};

	class ASTNode {
	public:
		ASTNode();
		ASTNode * left;
		ASTNode * right;
		ASTNode * leftCenter;
		ASTNode * rightCenter;

		Type type;

		virtual string getTAC() { return ""; }
	};

	class ASTNode_AddExpr : public ASTNode {
		public:
			ASTNode_AddExpr(bool isAddition);
			bool isAddition;
	};

	class ASTNode_MulExpr : public ASTNode {
		public:
			ASTNode_MulExpr(bool isMultiplication);
			bool isMultiplication;
	};

	class ASTNode_INT : public ASTNode {
		public:
			ASTNode_INT(int value);
			int value;
			string getTAC() {
				return to_string(static_cast<long long>(value));
			}
	};

	class ASTNode_FLOAT : public ASTNode {
		public:
			ASTNode_FLOAT(float value);
			float value;
			string getTAC() {
				return to_string(static_cast<long double>(value));
			}
	};

	class ASTNode_Identifier : public ASTNode {
		public:
			ASTNode_Identifier(string idName);
			string idName;
	};

	class ASTNode_Assignment : public ASTNode {
		public:
			ASTNode_Assignment();
	};

	class ASTNode_Comparator : public ASTNode {
		public:
			ASTNode_Comparator(string comp, string oppo);
			string comp;
			string oppo;
	};

	class ASTNode_Boolean : public ASTNode {
		public:
			ASTNode_Boolean(bool isTrue);
			bool isTrue;
	};

	class ASTNode_Function_Call : public ASTNode {
		public:
			ASTNode_Function_Call();
	};

	class ASTNode_Expr_List : public ASTNode {
		public:
			vector<ASTNode *> exprList;
			ASTNode_Expr_List();
		
			void add(ASTNode * node);
	};
}

#endif
