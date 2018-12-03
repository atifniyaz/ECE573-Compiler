#ifndef LL_STRING_HPP
#define LL_STRING_HPP

#include<string>

using namespace std;

class LLString {
public:
	string value;
	LLString * next;

	LLString(string value);
};

#endif