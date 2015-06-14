#pragma once
#include <string>
#include <vector>
#include "Shader.h"
#include "../../gl_core_3_2.hpp"
#include "../../utils/file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../Resource.h"

class Program
{
	std::string name;
	std::string error = "";
	std::vector<Shader> shaders;
	GLuint programId = 0;

	void destroy();
	GLuint link(std::vector<Shader> &shaders);
	bool initialized;
public:
	std::vector<fileInfo> info;
	Program(std::string name);
	~Program();

	std::string getName() { return name; }

	bool Program::load();
	std::string getError();

	GLuint get();
	bool use();
	GLint getAttrib(const GLchar *name);
	GLint getUniform(const GLchar *name);
};

