#include "ImageWindowState.h"
#include <SOIL.h>
#include <optional>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <memory>
#include <utility>

using namespace std;

static int imageID = 0;

optional<ImageWindowState> LoadImageFile(const char * filepath)
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

		unsigned char * pixels = new unsigned char[w * h * 3];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		auto v = vector<IFilter*>();
		MainFilter * fil = new MainFilter(w, h);
		fil->InitShader();
		v.push_back(fil);

		ImageWindowState im = {
			tex_2d,
			w,
			h,
			1.0f,
			string(filepath).c_str(),
			imageID++,
			pixels,
			GL_RGB,
			filesystem::current_path().string(),
			move(v)
		};
		return make_optional(im);
	}
}

// optional<ImageWindowState> LoadImageRawFile(const char * filepath, int width, int height) {
// 	ifstream input(p.string().c_str(), ios::binary);
// 	std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(input), {});
// 	GLuint texture_map;
// 	glGenTextures(1, &texture_map);
// 	glBindTexture(GL_TEXTURE_2D, texture_map);

// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

// 	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, buffer.data());
// 	glBindTexture(GL_TEXTURE_2D, 0);

// 	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
// 	GLuint FramebufferName = 0;
// 	glGenFramebuffers(1, &FramebufferName);
// 	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

// 	// The texture we're going to render to
// 	GLuint renderedTexture;
// 	glGenTextures(1, &renderedTexture);

// 	// "Bind" the newly created texture : all future texture functions will modify this texture
// 	glBindTexture(GL_TEXTURE_2D, renderedTexture);

// 	// Give an empty image to OpenGL ( the last "0" )
// 	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

// 	// Poor filtering. Needed !
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

// 	// Set "renderedTexture" as our colour attachement #0
// 	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

// 	// Set the list of draw buffers.
// 	GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
// 	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

// 	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
// 		cout << "Error rendering to texture." << endl;

// 	glBindTexture(GL_TEXTURE_2D, 0);

// 	unsigned char *b = new unsigned char[buffer.size()];
// 	std::copy(buffer.begin(), buffer.end(), b);
// 	ImageWindowState i = {
// 		texture_map,
// 		width,
// 		height,
// 		1.0f,
// 		p.filename().string().c_str(),
// 		imageID++,
// 		b,
// 		GL_LUMINANCE,
// 		FramebufferName,
// 		renderedTexture,
// 		filesystem::current_path().string(),
// 		// programID,
// 		// TextureID,
// 	};
// }



bool SaveImageFile(const char * filepath, ImageWindowState * image)
{
	glBindTexture(GL_TEXTURE_2D, image->filters[image->filters.size() - 1]->_outputTexture);

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

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, image->colorFormat, GL_UNSIGNED_BYTE, image->data);
	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

