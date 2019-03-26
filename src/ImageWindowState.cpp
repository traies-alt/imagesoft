#include "ImageWindowState.h"
#include <SOIL.h>
#include <optional>
#include <filesystem>
#include <iostream>

using namespace std;

optional<ImageWindowState> LoadImageFile(const char * filepath, GLuint programID, GLuint textureSampler)
{
	GLuint tex_2d = SOIL_load_OGL_texture(
			filepath,
			SOIL_LOAD_AUTO,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_MIPMAPS
	);
	if (tex_2d == 0) {
			return nullopt;
	} else {
		glBindTexture(GL_TEXTURE_2D, tex_2d);

		int w, h;
    int miplevel = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

		// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
		GLuint FramebufferName = 0;
		glGenFramebuffers(1, &FramebufferName);
		glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

		// The texture we're going to render to
		GLuint renderedTexture;
		glGenTextures(1, &renderedTexture);

		// "Bind" the newly created texture : all future texture functions will modify this texture
		glBindTexture(GL_TEXTURE_2D, renderedTexture);

		// Give an empty image to OpenGL ( the last "0" )
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, w, h, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

		// Poor filtering. Needed !
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// Set "renderedTexture" as our colour attachement #0
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

		// Set the list of draw buffers.
		GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
		glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			cout << "Error rendering to texture." << endl;

		unsigned char * pixels = new unsigned char[w * h * 4];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		ImageWindowState im = {
			tex_2d,
			w,
			h,
			1.0f,
			string(filepath).c_str(),
			0,
			pixels,
			GL_RGB,
			FramebufferName,
			renderedTexture,
			filesystem::current_path().string(),
			programID,
			textureSampler,
		};
		glBindTexture(GL_TEXTURE_2D, 0);
		return make_optional(im);
	}
}

bool SaveImageFile(const char * filepath, ImageWindowState * image)
{
	glBindTexture(GL_TEXTURE_2D, image->outputTexture);

	auto buffer = new unsigned char [4 * image->height * image->width];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	SOIL_save_image(filepath, SOIL_SAVE_TYPE_TGA, image->width, image->height, 4, buffer);
	delete[] buffer;

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

bool ReloadImage(ImageWindowState * image) {
	glBindTexture(GL_TEXTURE_2D, image->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, image->colorFormat, GL_UNSIGNED_BYTE, image->data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
