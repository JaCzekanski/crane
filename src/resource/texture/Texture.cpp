#include "Texture.h"


Texture::Texture( SDL_Texture *texture )
{
	this->texture = texture;
}


Texture::~Texture()
{
	if (texture != NULL) SDL_DestroyTexture(texture);
}

SDL_Texture* Texture::Get()
{
	return texture;
}