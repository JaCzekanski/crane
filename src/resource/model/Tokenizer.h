#pragma once
#include <string>
#include "Token.h"

class Tokenizer
{
private:
	int pos;
	int len;
	int currentLine = 1;
	bool available = true;
	std::string &line;

	Token token;
public:
	Tokenizer( std::string &s );
	~Tokenizer();

	bool tokenAvailable() { return available; }
	Token getToken();
	int getPosition();
	int getLine() { return currentLine; }
	void nextLine();
};

