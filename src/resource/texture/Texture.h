#pragma once
#include <SDL.h>
#include <string>
class Texture
{
private:
	SDL_Texture *texture;
public:
	std::string filename;
	_stat info;
	Texture( SDL_Texture *texture );
	~Texture();
	SDL_Texture* Get();
};

