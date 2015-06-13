#pragma once
#include <unordered_map>
#include <memory>
#include <SDL.h>
#include "texture/Texture.h"
#include "shader\Program.h"
#include "model\Model.h"
#include "../Logger.h"
#include "texture\Texture.h"
#include <sys/types.h>
#include <sys/stat.h>

class ResourceManager
{
private:
	const std::string dataFolder = "data";
	const std::string programFolder = "shader";
	const std::string modelFolder = "model";
	const std::string textureFolder = "texture";

	std::unordered_map<std::string, std::shared_ptr<Program>> programs;
	std::unordered_map<std::string, std::shared_ptr<Model>> models;
	std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

	bool checkFile(const std::string file);

	bool loadProgram(std::string name);
	bool loadModel(std::string name);
	bool loadTexture(std::string name);
public:
	ResourceManager();
	~ResourceManager();

	std::shared_ptr<Program> getProgram(const std::string& id)
	{
		auto p = programs.find(id);
		if (p == programs.end()) {
			if (!loadProgram(id)) {
				logger.Fatal("Cannot load program %s", id.c_str());
				return std::shared_ptr<Program>();
			}
			logger.Success("Program %s loaded", id.c_str());
			return programs.find(id)->second;
		}
		return p->second;
	}

	std::shared_ptr<Model> getModel(const std::string& id)
	{
		auto p = models.find(id);
		if (p == models.end()) {
			if (!loadModel(id)) {
				logger.Fatal("Cannot load model %s", id.c_str());
				return std::shared_ptr<Model>();
			}
			logger.Success("Model %s loaded", id.c_str());
			return models.find(id)->second;
		}
		return p->second;
	}

	std::shared_ptr<Texture> getTexture(const std::string& id)
	{
		auto p = textures.find(id);
		if (p == textures.end()) {

			if (!loadTexture(id)) {
				logger.Fatal("Cannot load texture %s", id.c_str());
				return std::shared_ptr<Texture>();
			}
			logger.Success("Texture %s loaded", id.c_str());
			return textures.find(id)->second;
		}
		return p->second;
	}

	void scanAndReload();
};

