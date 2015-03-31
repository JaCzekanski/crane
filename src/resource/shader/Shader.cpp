#include "Shader.h"
#include "../../utils/file.h"
#include <sstream>
#include <vector>

Shader::Shader(std::string name, ShaderType shaderType)
{
	this->name = name;
	this->type = shaderType;
}

Shader::~Shader()
{
	destroy();
}

void Shader::destroy()
{
	if (compiled && shaderId != 0) gl::DeleteShader(shaderId);
}

bool Shader::reload()
{
	destroy();
	return compile();
}

bool Shader::compile()
{
	error = name + ": ";
	auto data = getFileContents(name);
	data.push_back(0);

	GLchar const* lines[] = { (const GLchar*)&data[0], NULL };

	if (type == ShaderType::Vertex) shaderId = gl::CreateShader(gl::VERTEX_SHADER);
	else if (type == ShaderType::Fragment) shaderId = gl::CreateShader(gl::FRAGMENT_SHADER);
	else
	{
		error = std::string("Wrong shader type");
		return false;
	}
	if (shaderId == 0)
	{
		error = std::string("Cannot create shader");
		return false;
	}

	gl::ShaderSource(shaderId, 1, lines, NULL);
	gl::CompileShader(shaderId);

	GLint status;
	gl::GetShaderiv(shaderId, gl::COMPILE_STATUS, &status);
	if (!status)
	{
		std::string buffer;
		buffer.resize(1024);
		gl::GetShaderInfoLog(shaderId, 1024, NULL, &buffer[0]);
		error += buffer;
		return false;
	}
	error = std::string();
	compiled = true;
	return true;
}

GLuint Shader::get()
{
	return shaderId;
}

std::string Shader::getError()
{
	return error;
}

bool Shader::isCompiled()
{
	return compiled;
}