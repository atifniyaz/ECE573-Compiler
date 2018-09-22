#include <stddef.h>

struct Identifier {
	const char * id; // Name of Identifier
	int type; // Type of Identifier

	// Identifier Values
	const char * valStr; // 0 Type
	int valInt; // 1 Type
	float valFloat; // 2 Type

	// Handle lists of identifiers
	struct Identifier * next;
};

struct SymbolTable {
	const char * name;

	struct SymbolTable ** children;
	struct Identifier * ids;

	int childLen;
};

struct SymbolTableStack {
	struct SymbolTable * value;
	struct SymbolTableStack * next;
};

extern struct SymbolTableStack * stack;

void printSymbolTable(struct SymbolTable * table);
struct SymbolTable * popStack(struct SymbolTableStack ** stk);
void printStack();
void pushStack(struct SymbolTableStack ** stk, struct SymbolTable * table);
int isLegalSymbolTable(struct SymbolTable * table);