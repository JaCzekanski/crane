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

struct Segment
{
	glm::vec3 start;
	glm::vec3 end;

	float getAngle()
	{
		return atan2f(end.z - start.z, end.y - start.y);
	}
	
	// position from 0.f to 1.f
	glm::vec3 getPosition( float position )
	{
		position = fmod(position, 1.f);
		if (position < 0.f) position = 1.f + position;

		return ((end - start) * position) + start;
	}

	glm::vec3 getCenter()
	{
		return (start + end) * 0.5f;
	}
};

struct ModelObject
{
	GLuint vbo; // Buffer
	GLuint vao; // Metadata 
	int size;
	std::vector<modelVertice> data;
	std::vector<Segment> segments;
	Material material;

	bool use()
	{
		gl::BindVertexArray(vao);
		return true;
	}

	bool render()
	{
		if (data.empty()) return false;
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

	void commit(std::string objectName, std::string materialName, std::vector<modelVertice> &data, std::vector<Segment> &segments);

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