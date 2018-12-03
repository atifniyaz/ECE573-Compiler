#ifndef IDENTIFIER_HPP
#define IDENTIFIER_HPP

#include<string>

using namespace std;

class Identifier {
public:
	string name;
	Identifier * next;

	Identifier();
	virtual string getType() = 0;
	Identifier * getLast();
	void print();
};

class IdInteger : public Identifier {
public:
	int value;
	string getType();
};

class IdFloat : public Identifier {
public:
	float value;
	string getType();
};

class IdString : public Identifier {
public:
	string value;
	string getType();
};

#endif