#include "Program.h"

Program::Program(std::string name)
{
	this->name = name;
}

Program::~Program()
{
	destroy();
}

void Program::destroy()
{
	if (programId == 0) return;
	gl::DeleteProgram(programId);
	shaders.clear();
}

bool Program::load()
{
	info.clear();
	struct _stat newinfo;

	_stat((name + ".frag").c_str(), &newinfo);
	info.push_back({ name + ".frag", newinfo });

	_stat((name + ".vert").c_str(), &newinfo);
	info.push_back({ name + ".vert", newinfo });

	if (!fileExists(name + ".frag")) return false;
	if (!fileExists(name + ".vert")) return false;

	std::vector<Shader> newShaders;

	newShaders.push_back(Shader(name + ".frag", ShaderType::Fragment));
	newShaders.push_back(Shader(name + ".vert", ShaderType::Vertex));

	GLuint id = link(newShaders);
	if (id == 0) return false;


	shaders = std::move(newShaders);
	programId = id;
	return true;
}


GLuint Program::link(std::vector<Shader> &shaders)
{
	GLuint id = gl::CreateProgram();
	if (id == 0) return 0;

	for (Shader s : shaders)
	{
		if (s.isCompiled()) continue;
		if (!s.compile())
		{
			error = s.getError();
			return false;
		}
		gl::AttachShader(id, s.get());
	}
	gl::LinkProgram(id);

	GLint status;
	gl::GetProgramiv(id, gl::LINK_STATUS, &status);
	if (status == false)
	{
		std::string buffer;
		buffer.resize(1024);
		gl::GetProgramInfoLog(id, 1024, NULL, &buffer[0]);
		error = "Linking error: " + buffer;

		gl::DeleteProgram(id);
		return 0;
	}
	return id;
}

GLuint Program::get()
{
	return programId;
}

std::string Program::getError()
{
	return error;
}

bool Program::use()
{
	if (programId == 0)
	{
		error = std::string("Program not linked.");
		return false;
	}
	gl::UseProgram( programId );
	return true;
}

GLint Program::getAttrib(const GLchar *name)
{
	return gl::GetAttribLocation(programId, name);
}

GLint Program::getUniform(const GLchar *name)
{
	return gl::GetUniformLocation(programId, name);
}