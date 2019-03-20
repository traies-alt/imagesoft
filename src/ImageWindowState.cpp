#include "ImageWindowState.h"
#include <SOIL.h>
#include <optional>
#include <string>

std::optional<ImageWindowState> LoadImageFile(const char * filepath) 
{	
	GLuint tex_2d = SOIL_load_OGL_texture(
			filepath,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if (tex_2d == 0) {
			return std::nullopt;
	} else {
		glBindTexture(GL_TEXTURE_2D, tex_2d);

		int w, h;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
		
		glBindTexture(GL_TEXTURE_2D, 0);

		ImageWindowState im = {
			tex_2d,
			w,
			h,
			1.0f,
			std::string(filepath).c_str()
		};
		return std::make_optional(im);
	}
}

bool SaveImageFile(const char * filepath, ImageWindowState * image)
{
	glBindTexture(GL_TEXTURE_2D, image->texture);

	auto buffer = new unsigned char [4 * image->height * image->width];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	SOIL_save_image(filepath, SOIL_SAVE_TYPE_TGA, image->width, image->height, 4, buffer);
	delete[] buffer;

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}