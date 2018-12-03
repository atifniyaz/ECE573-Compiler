#ifndef AST_HPP
#define AST_HPP

#include<string>

using namespace std;

namespace ast {

	enum class Type {
		ADD_EXPR, MUL_EXPR, INT_VAL, FLOAT_VAL, ID_FIER
	};

	class ASTNode {
	public:
		ASTNode();
		ASTNode * left;
		ASTNode * right;
		Type type;

		void print(int depth);
		virtual void onPrint() = 0;
	};

	class ASTNode_AddExpr : public ASTNode {
		public:
			ASTNode_AddExpr(bool isAddition);
			bool isAddition;

			void onPrint();
	};

	class ASTNode_MulExpr : public ASTNode {
		public:
			ASTNode_MulExpr(bool isMultiplication);
			bool isMultiplication;

			void onPrint();
	};

	class ASTNode_INT : public ASTNode {
		public:
			ASTNode_INT(int value);
			int value;

			void onPrint();
	};

	class ASTNode_FLOAT : public ASTNode {
		public:
			ASTNode_FLOAT(float value);
			float value;

			void onPrint();
	};

	class ASTNode_Identifier : public ASTNode {
		public:
			ASTNode_Identifier(string idName);
			string idName;

			void onPrint();
	};

	class ASTNode_Assignment : public ASTNode {
		public:
			void onPrint();
	};
}

#endif