#pragma once
#include <string>
#include <glm\glm.hpp>
#include <vector>
#include "../../gl_core_3_2.hpp"
#include "../../utils/file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../Resource.h"

class Texture
{
	std::string name;
	GLuint tex;
	unsigned int width;
	unsigned int height;
public:
	fileInfo info;
	std::string filename;
	Texture( std::string name );
	~Texture();

	std::string getName() { return name; }

	bool load();

	GLuint get();
	bool use();
};