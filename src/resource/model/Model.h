#pragma once
#include <string>
#include <glm\glm.hpp>
#include <vector>
#include "../../gl_core_3_2.hpp"
#include "../../utils/file.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "../Resource.h"
#include <unordered_map>
#include <memory>

struct modelVertice
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texcoord;
};

struct Material
{
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	std::string texture;
};

struct ModelObject
{
	GLuint vbo; // Buffer
	GLuint vao; // Metadata 
	int size;
	std::vector<modelVertice> data;
	Material material;

	bool use()
	{
		gl::BindVertexArray(vao);
		return true;
	}

	bool render()
	{
		use();
		gl::DrawArrays(gl::TRIANGLES, 0, size);
		return true;
	}
};

class Model
{
	std::string name;
	bool initialized;

	std::unordered_map<std::string, Material> materials;
	std::string materialFile;

	void commit(std::string objectName, std::string materialName, std::vector<modelVertice> &data);

	std::unordered_map<std::string, Material> parseMaterialFile(std::string filename);
public:
	fileInfo info;
	std::unordered_map<std::string, std::shared_ptr<ModelObject>> objects;

	Model(std::string name);
	~Model();

	std::string getName() { return name; }

	bool load();
	bool render();
};