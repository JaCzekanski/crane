#include "Resource.h"


Resource::Resource(std::string name)
{
	this->name = name;
	loaded = false;
}


Resource::~Resource()
{
}
