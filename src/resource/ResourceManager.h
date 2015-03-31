#pragma once
#include <unordered_map>
#include <memory>
#include <SDL.h>
#include "texture/Texture.h"
#include "shader\Program.h"
#include "model\Model.h"
#include <sys/types.h>
#include <sys/stat.h>

class ResourceManager
{
private:
	const std::string dataFolder = "data";
	const std::string programFolder = "shader";
	const std::string modelFolder = "model";

	std::unordered_map<std::string, std::shared_ptr<Program>> programs;
	std::unordered_map<std::string, std::shared_ptr<Model>> models;


	bool checkFile(const std::string file);
public:
	ResourceManager();
	~ResourceManager();

	bool loadProgram(std::string name);
	bool loadModel(std::string name);

	std::shared_ptr<Program> getProgram(const std::string& id)
	{
		auto p = programs.find(id);
		if (p == programs.end()) return std::shared_ptr<Program>();
		return p->second;
	}

	std::shared_ptr<Model> getModel(const std::string& id)
	{
		auto p = models.find(id);
		if (p == models.end()) return std::shared_ptr<Model>();
		return p->second;
	}

	void scanAndReload();
};

