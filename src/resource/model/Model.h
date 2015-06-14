#pragma once
#include <string>
#include <glm\glm.hpp>
#include <vector>
#include "../../gl_core_3_2.hpp"
#include "../../utils/file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../Resource.h"


struct modelVertice
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

class Model
{
	std::string name;
	GLuint vbo; // Buffer
	GLuint vao; // Metadata 
	int size;
	bool initialized;
public:
	fileInfo info;
	Model(std::string name);
	~Model();

	std::vector<modelVertice> data;

	std::string getName() { return name; }
	int getSize() { return size; }

	bool load();

	GLuint get();
	bool use();
	bool render();
};