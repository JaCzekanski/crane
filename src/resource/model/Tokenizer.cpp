#include "Tokenizer.h"
#include <cctype>
#include <cmath>
#include <cstdlib>

Tokenizer::Tokenizer(std::string s)
{
	line = s;
	pos = -1;
	len = line.length();
	line += ' ';
}


Tokenizer::~Tokenizer()
{
}

Token Tokenizer::getToken()
{
	Token token;
	string word;
	pos++;
	bool comment = false;
	bool lineEnd = false;
	for (bool found = false; !found; pos++)
	{
		if (pos >= len) break;

		if (line[pos] == ' ' || line[pos] == '\t')
		{
			if (word.empty()) continue; 
			else break; // Read only one word
		}
		else if (line[pos] == '\n' || (line[pos] == '\r' && line[pos+1] == '\n')) 
		{
			lineEnd = true;
			if (word.empty()) token.type = TokenType::Empty;
			else pos--;
			break;
		}
		else if (line[pos] == '#') comment = true;
		else if (line[pos] == '/') 
		{
			if (word.empty()) token.type = TokenType::Separator;
			else pos--;
			break; // Face Separator
		}
		if (!comment) word += line[pos];
	}
	
	if (pos >= len) available = false;
	if (word.empty())
	{
		if (lineEnd) token.type = TokenType::Empty;
		if (pos >= len) token.type = TokenType::Empty;
		return token;
	}

	bool number = false;
	if (isdigit(word[0]) || word[0] == '-')
	{
		number = true;
		token.type = TokenType::Int;
	}
	else if (word[0] == '.')
	{
		number = true;
		token.type = TokenType::Float;
	}
	else token.type = TokenType::String;

	// Analyze token
	token.i = 0;
	for (int i = 0; i < word.length(); i++)
	{
		if (number)
		{
			if (word[i] == '.')
			{
				if (token.type == TokenType::Float)
				{
					token.type = TokenType::Invalid; // Multiple commas
					break;
				}
				token.type = TokenType::Float;
			}
			else if (word[i] == '-')
			{
				if (token.negative)
				{
					token.type = TokenType::Invalid; // Multiple negation
					break;
				}
				token.negative = true;
			}
			// Add '+'
			else if (!isdigit(word[i]))
			{
				token.type = TokenType::Invalid;
				break;
			}

			if (token.type == TokenType::Int)
			{
				token.i *= 10;
				token.i += word[i] - '0';
			}
		}
		else token.s += word[i];
	}
	if (token.type == TokenType::Float) token.f = atof(word.c_str());
	if (token.negative && token.type == TokenType::Int) token.i = -token.i;

	return token;
}

int Tokenizer::getPosition()
{
	return pos;
}