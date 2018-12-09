#include<string>
#include<vector>
#include<iterator>
#include<regex>
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

void printInOut(CodeLine * line, int index) {
	cout << ";" << index << ": " << line->stringify() << endl;
	cout << "; outSet:";
	for (set<string>::iterator it = line->outSet.begin(); it != line->outSet.end(); it++) {
		cout << " " << *it;
	}
	cout << endl;
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
			for (const auto& any : globalVar->declMap) {
			 	Identifier * id = any.second;
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
			for (const auto& any : globalVar->declMap) {
			 	Identifier * id = any.second;
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

	for(int i = 0; i < size; i++) {
		printInOut(codeList[i], i);
	} 
}

CodeObject * optimized = new CodeObject();
string register4R [] = {"", "", "", ""};
bool dirty4R [] = {false, false, false, false};
vector<string> usedList;

string printSet(set<string> aSet) {
	string s;
	for (auto const& e : aSet)
	{
	    s += e;
	    s += ',';
	}
	return s;
}

void tac::CodeObject::createReset() {
	int size = codeList.size();
	for (int i = 0; i < size; i++) {
		CodeLine * line = codeList[i];
		if (!line->predecessors.size()) {
			reset.push_back(true);
		} else {
			bool predFlag = false;
			for(auto predecessor : line->predecessors) {
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

void tac::CodeObject::allocateRegisters() {
	int size = codeList.size();
	regex validReg ("r[0-9]+");
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

			if (std::regex_match(arg1, validReg)) {
				if (dictReg.find(arg1) != dictReg.end()) {
					indxR1 = dictReg.find(arg1)->second;
					line->arg2 = "r" + to_string(indxR1);
				} else {
					indxR1 = ensure(arg1, codeList[i - 1]);
					line->arg2 = "r" + to_string(indxR1);
					dictReg.insert(pair<string, int>(arg1, indxR1));
				}
			}

			if (line->outSet.find(arg1) == line->outSet.end()) {
				free(indxR1);
			}

			if (std::regex_match(arg2, validReg)) {
				if (dictReg.find(arg2) != dictReg.end()) {
					indxR2 = dictReg.find(arg2)->second;
					line->arg3 = "r" + to_string(indxR2);
				} else {
					indxR2 = allocate(arg2, codeList[i - 1]);
					line->arg3 = "r" + to_string(indxR2);
					dictReg.insert(pair<string, int>(arg2, indxR2));
					dirty4R[indxR2] = true;
				}
			}

		} else if(!op.compare("ret") || this->reset[i]) {
			spill(true);
		} else {
			if (std::regex_match(arg1, validReg)) {
				if (dictReg.find(arg1) != dictReg.end()) {
					indxR1 = dictReg.find(arg1)->second;
					line->arg2 = "r" + to_string(indxR1);
				}
			}
			if (std::regex_match(arg2, validReg)) {
				if (dictReg.find(arg2) != dictReg.end()) {
					indxR2 = dictReg.find(arg2)->second;
					line->arg3 = "r" + to_string(indxR2);
				}
			}
		} // 8, 17

		optimized->addLine(line);
	}

	cout << "; " << codeList[0]->stringify() << endl;
	for(auto elem : dictReg)
	{
   		cout << ";" << elem.first << " " << elem.second << endl;
	}
	codeList = optimized->codeList;
}

int ensure(string reg, CodeLine * prev) {
	// see if register is already there
	regex validReg ("r[0-9]+");
	for (int i = 3; i >= 0; i--) {
		if (!reg.compare(register4R[i])) {
			return i;
		}
	}

	int regIndex = allocate(reg, prev);
	if(!std::regex_match(reg, validReg) || 
		find(usedList.begin(), usedList.end(), reg) != usedList.end()) {

	}
	if(std::regex_match(reg, validReg) && 
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
	register4R[freeIndex] = reg;
	return freeIndex;
}

void free(int i) {
	regex validReg ("r[0-9]+");
	if (dirty4R[i] && !regex_match(register4R[i], validReg)) {
		optimized->addLine(new CodeLine(
			"move", "r" + to_string(i), register4R[i], ""	
		));
	}
	register4R[i] = "";
	dirty4R[i] = false;
}

void spill(bool write) {
	regex validReg ("r[0-9]+");
	for (int i = 3; i >= 0; i--) {
		if (write && register4R[i].compare("") &&
			!regex_match(register4R[i], validReg)) {
			optimized->addLine(new CodeLine(
				"move", "r" + to_string(i), register4R[i], ""	
			));
		}
		register4R[i] = "";
		dirty4R[i] = false;
	}
}