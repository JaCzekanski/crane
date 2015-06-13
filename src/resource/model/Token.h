#pragma once
#include <string>

class Token
{
public:
	enum class Type
	{
		Unknown, // No token
		Invalid, // eg. number with char inside, too big number
		Separator, // Separator
		Empty, // End of line, no data
		String,
		Int,
		Float
	};

	std::string s;
	int i;
	float f;
	bool comma;
	bool negative;
	int commaDigit;
	Type type;
	
	Token()
	{
		comma = false;
		negative = false;
		commaDigit = 1;
		type = Type::Unknown;
		i = 0;
		f = 0;
	}
};