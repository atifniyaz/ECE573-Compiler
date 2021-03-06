#include<string>
#include<vector>
#include<iterator>
#include<algorithm>
#include "CodeObject.hpp"
#include "Identifier.hpp"
#include "SymbolTable.hpp"
#include "SymbolTableStack.hpp"


using namespace std;
using namespace tac;

SymbolTable * global;

void spill();
int ensure(string reg, CodeLine * prev);
void resetAllocation();
int allocate(string reg, CodeLine * prev);
void free(int i);
void spill(bool write);

void tac::CodeObject::optimizeTiny() {
	this->configureCFG();
	this->createGenKillSet();
	this->createInOutSet();
	this->createReset();
	this->allocateRegisters();
}

void tac::CodeObject::configureCFG() {
	int size = codeList.size();

	// Compute Successors
	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];
	
		if (!line->arg1.compare("jmp") || // Unconditional Jump
			!line->arg1.compare("jge") || // Conditional Jumps
			!line->arg1.compare("jle") ||
			!line->arg1.compare("jne") ||
			!line->arg1.compare("jeq") ||
			!line->arg1.compare("jgt") ||
			!line->arg1.compare("jlt")) {

			for (int j = 0; j < size; j++) {
				CodeLine * label = codeList[j];

				if (!label->arg1.compare("label") &&
					!label->arg2.compare(line->arg2)) {
					line->successors.insert(label);
					break;
				}
			}
		}

		// Return doesn't have a successor
		// Jump has one sucessor, but that was already found
		if (i + 1 < size && (line->arg1.compare("jmp") && line->arg1.compare("ret"))) {
			line->successors.insert(codeList[i + 1]);
		}
	}

	// Compute Predecessors
	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];
		set<CodeLine *> successors = line->successors;
		for (set<CodeLine *>::iterator it = successors.begin();
			it != successors.end(); it++) {
			CodeLine * successor = *it;
			successor->predecessors.insert(line);
		}
	}
}

void tac::CodeObject::createGenKillSet() {
	int size = codeList.size();
	SymbolTable * globalVar = global;

	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];
		string op = line->arg1;
		string arg1 = line->arg2;
		string arg2 = line->arg3;

		if (op.find("sys read") != string::npos ||
			!op.compare("pop")) {
			if (!arg1.empty()) {
				line->genSet.insert(arg1);
			}
		} else if (op.find("sys write") != string::npos ||
			!op.compare("push")) {
			if (!arg1.empty()) {
				line->killSet.insert(arg1);
			}
		} else if (!op.compare("cmpi")) {
			line->genSet.insert(arg1);
			line->genSet.insert(arg2);
		} else if (!op.compare("jsr")) {
			for (map<string, Identifier *>::iterator it = globalVar->declMap.begin();
				it != globalVar->declMap.end(); it++) {
				Identifier * id = it->second;
				line->genSet.insert(id->name);
			}
		} else if (!op.compare("move") ||
			op.find("mul") != string::npos ||
			op.find("div") != string::npos ||
			op.find("add") != string::npos ||
			op.find("sub") != string::npos) {
			line->genSet.insert(arg1);
			line->killSet.insert(arg2);
		}
	}
}

bool containsAll(set<string> s1, set<string> s2) {
	for(set<string>::iterator it = s1.begin(); it != s1.end(); it++) {
		if (s2.find(*it) == s2.end()) {
			return false;
		}
	}
	for(set<string>::iterator it = s2.begin(); it != s2.end(); it++) {
		if (s1.find(*it) == s1.end()) {
			return false;
		}
	}
	return true;
}

void tac::CodeObject::createInOutSet() {
	int size = codeList.size();
	vector<int> indexList;
	SymbolTable * globalVar = global;

	for (int i = 0; i < size; i++) {
		indexList.push_back(i);
		CodeLine * line = codeList[i];

		if (!line->arg1.compare("ret")) {
			for (map<string, Identifier *>::iterator it = globalVar->declMap.begin();
				it != globalVar->declMap.end(); it++) {
				Identifier * id = it->second;
				line->outSet.insert(id->name);
			}
		}
	}

	int k = 0;

	while (k != indexList.size()) {
		int index = indexList[k++];
		CodeLine * line = codeList[index];

		set<string> prevInSet = line->inSet;
		set<string> inSet;
		set<string> outSet = line->outSet;

		if (line->arg1.compare("ret")) {
			outSet.clear();
			for (set<CodeLine *>::iterator it = line->successors.begin();
				it != line->successors.end(); it++) {
				CodeLine * successor = *it;
				outSet.insert(successor->inSet.begin(), successor->inSet.end());
			}
			line->outSet = outSet;
		}

		// Update inset
		inSet = outSet;
		for (set<string>::iterator it = line->killSet.begin(); 
			it != line->killSet.end(); it++) {
			if (inSet.find(*it) != inSet.end()) {
				inSet.erase(*it);
			}
		}
		inSet.insert(line->genSet.begin(), line->genSet.end());

		line->inSet = inSet;

		if (!containsAll(inSet, prevInSet)) {
			for (set<CodeLine *>::iterator it = line->predecessors.begin();
				it != line->predecessors.end(); it++) {
				CodeLine * predecessor = *it;
				
				auto _it = std::find(codeList.begin(), codeList.end(), predecessor);
				int dex = std::distance(codeList.begin(), _it);
				indexList.insert(indexList.end(), dex);
			}
		}
	}
}

CodeObject * optimized = new CodeObject();
string register4R [] = {"", "", "", ""};
bool dirty4R [] = {false, false, false, false};
vector<string> usedList;

void tac::CodeObject::createReset() {
	int size = codeList.size();
	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];
		if (!line->predecessors.size()) {
			reset.push_back(true);
		} else {
			bool predFlag = false;
			for(set<CodeLine *>::iterator it = line->predecessors.begin();
				it != line->predecessors.end(); it++) {
				CodeLine * predecessor = *it;
				string op = predecessor->arg1;
				if(	!op.compare("jmp") ||
					!op.compare("jeq") ||
					!op.compare("jne") ||
					!op.compare("jlt") ||
					!op.compare("jle") ||
					!op.compare("jgt") ||
					!op.compare("jge")) {
					reset.push_back(true);
					predFlag = true;
					break;
				}
			}
			if (!predFlag) {
				reset.push_back(false);
			}
		}
	}
}

bool matchReg(string reg) {
	if (reg.size() < 2) {
		return false;
	}
	for (int i = 0; i < reg.size(); i++) {
		char c = reg[i];
		if (i == 0 && c == 'r') {
			continue;
		} else if(i != 0 && c >= '0' && c <= '9') {
			continue;
		} else {
			return false;
		}
	}
	return true;
}

void tac::CodeObject::allocateRegisters() {
	int size = codeList.size();
	map<string, int> dictReg;
	resetAllocation();

	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];

		string op = line->arg1;
		string arg1 = line->arg2;
		string arg2 = line->arg3;

		int indxR1 = -1;
		int indxR2 = -1;

		if (!op.compare("move") ||
			op.find("mul") != string::npos ||
			op.find("div") != string::npos ||
			op.find("add") != string::npos ||
			op.find("sub") != string::npos ||
			!op.compare("pop")) {

			if (matchReg(arg1)) {
				if (dictReg.find(arg1) != dictReg.end()) {
					indxR1 = dictReg.find(arg1)->second;
					line->arg2 = "r" + to_string(static_cast<long long>(indxR1));
				} else {
					indxR1 = ensure(arg1, codeList[i - 1]);
					line->arg2 = "r" + to_string(static_cast<long long>(indxR1));
					dictReg.insert(pair<string, int>(arg1, indxR1));
				}
			}

			if (indxR1 != -1 && line->outSet.find(arg1) == line->outSet.end()) {
				free(indxR1);
			}

			if (matchReg(arg2)) {
				if (dictReg.find(arg2) != dictReg.end()) {
					indxR2 = dictReg.find(arg2)->second;
					line->arg3 = "r" + to_string(static_cast<long long>(indxR2));
				} else {
					indxR2 = allocate(arg2, codeList[i - 1]);
					line->arg3 = "r" + to_string(static_cast<long long>(indxR2));
					dictReg.insert(pair<string, int>(arg2, indxR2));
					dirty4R[indxR2] = true;
				}
			}

		} else if(!op.compare("ret") || this->reset[i]) {
			spill(true);
		} else {
			if (matchReg(arg1)) {
				if (dictReg.find(arg1) != dictReg.end()) {
					indxR1 = dictReg.find(arg1)->second;
					line->arg2 = "r" + to_string(static_cast<long long>(indxR1));
				}
			}
			if (matchReg(arg2)) {
				if (dictReg.find(arg2) != dictReg.end()) {
					indxR2 = dictReg.find(arg2)->second;
					line->arg3 = "r" + to_string(static_cast<long long>(indxR2));
				}
			}
		} // 17

		optimized->addLine(line);
	}

	CodeObject * obj = new CodeObject();

	for(int i = 0; i < optimized->codeList.size(); i++) {
		CodeLine * elem = optimized->codeList[i];
		
		if (!elem->arg1.compare("move") && !elem->arg2.compare(elem->arg3)) {
			continue;
		} else if (!elem->arg1.compare("jsr")) {
			for(int i = 0; i < 4; i++) {
				obj->addLine(new CodeLine("move", "r" + to_string(static_cast<long long>(i)),
				"$-" + to_string(static_cast<long long>(i + 1)), ""));
			}
			obj->addLine(elem);
			for(int i = 0; i < 4; i++) {
				obj->addLine(new CodeLine("move", "$-" + to_string(static_cast<long long>(i + 1)),
				"r" + to_string(static_cast<long long>(i)), ""));
			}
		} else {
			obj->addLine(elem);
		}
	} 
	codeList = obj->codeList;
}

int ensure(string reg, CodeLine * prev) {
	// see if register is already there
	//regex validReg ("r[0-9]+");
	for (int i = 3; i >= 0; i--) {
		if (!reg.compare(register4R[i])) {
			return i;
		}
	}

	int regIndex = allocate(reg, prev);
	if(!matchReg(reg) || 
		find(usedList.begin(), usedList.end(), reg) != usedList.end()) {

	}
	if(matchReg(reg) && 
		find(usedList.begin(), usedList.end(), reg) == usedList.end()) {
		usedList.push_back(reg);
	}
	return regIndex;
}

void resetAllocation() {
	optimized = new CodeObject();
	spill(false);
	usedList.clear();
}

int allocate(string reg, CodeLine * prev) {
	// check if there is a empty spot
	for (int i = 3; i >= 0; i--) {
		if (register4R[i].empty()) {
			register4R[i] = reg;
			return i;
		}
	}

	// free one of these spots
	int freeIndex = 0;
	for (int i = 0; i < 4; i++) {
		string reg = register4R[i];
		if (reg.compare(prev->arg2) && reg.compare(prev->arg1)) {
			freeIndex = i;
		}
	}

	free(freeIndex);
	register4R[freeIndex] = reg;;
	return freeIndex;
}

void free(int i) {
	//regex validReg ("r[0-9]+");
	if (dirty4R[i] && !matchReg(register4R[i])) {
		optimized->addLine(new CodeLine(
			"move", "r" + to_string(static_cast<long long>(i)), register4R[i], ""	
		));
	}
	register4R[i] = "";
	dirty4R[i] = false;
}

void spill(bool write) {
	//regex validReg ("r[0-9]+");
	for (int i = 3; i >= 0; i--) {
		if (write && register4R[i].compare("") &&
			!matchReg(register4R[i])) {
			optimized->addLine(new CodeLine(
				"move", "r" + to_string(static_cast<long long>(i)), register4R[i], ""	
			));
		}
		register4R[i] = "";
		dirty4R[i] = false;
	}
}
