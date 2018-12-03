#include "LLString.hpp"

LLString::LLString(string value) {
	this->value = value;
	this->next = NULL;
}