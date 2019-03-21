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
			SOIL_FLAG_MIPMAPS
	);
	if (tex_2d == 0) {
			return std::nullopt;
	} else {
		glBindTexture(GL_TEXTURE_2D, tex_2d);

		int w, h;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
		

		unsigned char * pixels = new unsigned char[w * h * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		ImageWindowState im = {
			tex_2d,
			w,
			h,
			1.0f,
			std::string(filepath).c_str(),
			0,
			pixels,
			GL_RGBA,
		};
		glBindTexture(GL_TEXTURE_2D, 0);
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

bool ReloadImage(ImageWindowState * image) {
	glBindTexture(GL_TEXTURE_2D, image->texture);

	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, image->colorFormat, GL_UNSIGNED_BYTE, image->data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
