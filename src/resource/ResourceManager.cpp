#include "ResourceManager.h"
#include "../Logger.h"
#include <cstdio>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


ResourceManager::ResourceManager()
{
}
ResourceManager::~ResourceManager()
{
}

bool ResourceManager::checkFile(const std::string file)
{
	FILE *f = fopen(file.c_str(), "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool ResourceManager::loadProgram(std::string name)
{
	std::shared_ptr<Program> program(new Program(dataFolder + "/" + programFolder + "/" + name));
	if (program->load()) 
	{
		programs.emplace(name, program);
		return true;
	}
	logger.Error("%s", program->getError().c_str());
	return false;
}

bool ResourceManager::loadModel(std::string name)
{
	std::shared_ptr<Model> model(new Model(dataFolder + "/" + modelFolder + "/" + name));
	if (model->load())
	{
		models.emplace(name, model);
		return true;
	}
	logger.Error("Cannot load %s", name.c_str());
	return false;
}

bool ResourceManager::loadTexture(std::string name)
{
	std::shared_ptr<Texture> texture(new Texture(dataFolder + "/" + textureFolder + "/" + name));
	if (texture->load())
	{
		textures.emplace(name, texture);
		return true;
	}
	logger.Error("Cannot load %s", name.c_str());
	return false;
}

void ResourceManager::scanAndReload()
{
	time_t currentTime = time(NULL);
	struct _stat newinfo;
	for (auto p : programs)
	{
		for (auto i : p.second->info)
		{
			_stat(i.name.c_str(), &newinfo);
			if (newinfo.st_mtime != i.info.st_mtime)
			{
				if (p.second->load()) logger.Success("%s reload", p.second->getName().c_str());
				else logger.Error("%s", p.second->getError().c_str());
				break;
			}
		}

	}
	for (auto m : models)
	{
		_stat(m.second->info.name.c_str(), &newinfo);
		if (newinfo.st_mtime != m.second->info.info.st_mtime && currentTime != newinfo.st_mtime)
		{
			if (m.second->load()) logger.Success("%s reload", m.second->info.name.c_str());
		}

	}
	for (auto t : textures)
	{
		_stat(t.second->info.name.c_str(), &newinfo);
		if (newinfo.st_mtime != t.second->info.info.st_mtime && currentTime != newinfo.st_mtime)
		{
			if (t.second->load()) logger.Success("%s reload", t.second->info.name.c_str());
		}

	}
	return;
}