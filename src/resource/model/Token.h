#pragma once
#include <string>

enum class TokenType
{
	Unknown = 0, // No token
	Invalid, // eg. number with char inside, too big number
	Separator, // Separator
	Empty, // End of line, no data
	String,
	Int,
	Float
};

class Token
{
public:
	Token()
	{
		comma = false;
		negative = false;
		commaDigit = 1;
		type = TokenType::Unknown;
		i = 0;
		f = 0;
	}
	std::string s;
	int i;
	float f;
	bool comma;
	bool negative;
	int commaDigit;
	TokenType type;

	// Metadata
	std::string filename;
	int line;
};