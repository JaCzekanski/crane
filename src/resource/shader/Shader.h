#pragma once
#include <string>
#include "../../gl_core_3_2.hpp"
#include <glm\glm.hpp>

enum class ShaderType
{
	Vertex,
	Fragment
};

class Shader
{
private:
	std::string error = "";
	std::string name;
	bool compiled = false;
	ShaderType type;
	GLuint shaderId = 0;

	void destroy();
public:
	Shader(std::string name, ShaderType shaderType);
	~Shader();

	bool compile();
	GLuint get();

	std::string getError();
	bool isCompiled();

	bool reload();
};

