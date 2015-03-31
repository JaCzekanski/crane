#pragma once
#include <string>
#include "Token.h"

using namespace std;

class Tokenizer
{
private:
	int pos;
	int len;
	bool available = true;
	std::string line;
public:
	Tokenizer( std::string );
	~Tokenizer();

	bool tokenAvailable() { return available; }
	Token getToken();
	int getPosition();
};

