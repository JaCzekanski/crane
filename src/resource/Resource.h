#pragma once
#include <string>

struct fileInfo
{
	std::string name;
	struct _stat info;
};


class Resource
{
	bool loaded;
	std::string name;
public:
	Resource(std::string name);
	~Resource();

	virtual bool load() = 0;
	virtual bool destroy() = 0;
};

