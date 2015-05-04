#include "Texture.h"
#include "../../lodepng.h"
#include <vector>
#include <sstream>

Texture::Texture( std::string name )
{
	this->name = name;
}


Texture::~Texture()
{

}

bool Texture::load()
{
	_stat((name + ".png").c_str(), &info.info);
	info.name = name + ".png";

	std::vector<unsigned char> image;
	lodepng::decode(image, width, height, info.name, LCT_RGBA, 8U);
	if (image.empty()) return false;

	gl::GenTextures(1, &tex);
	gl::BindTexture(gl::TEXTURE_2D, tex);

	gl::TexImage2D(gl::TEXTURE_2D, 0, gl::RGBA, width, height, 0, gl::RGBA, gl::UNSIGNED_BYTE, &image[0]);

	gl::TexParameterf(gl::TEXTURE_2D, gl::TEXTURE_MIN_FILTER, gl::LINEAR_MIPMAP_LINEAR);
	gl::TexParameterf(gl::TEXTURE_2D, gl::TEXTURE_MAG_FILTER, gl::LINEAR_MIPMAP_LINEAR);

	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_S, gl::REPEAT);
	gl::TexParameteri(gl::TEXTURE_2D, gl::TEXTURE_WRAP_T, gl::REPEAT);

	gl::GenerateMipmap(gl::TEXTURE_2D);

	return true;
}

GLuint Texture::get()
{
	return tex;
}

bool Texture::use()
{
	gl::BindTexture(gl::TEXTURE_2D, tex);
	return true;
}