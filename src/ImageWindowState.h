#pragma once
#include <optional>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <memory>
#include <string>

struct ImageWindowState {
	GLuint texture;
	int width, height;
	float zoom;
	const char * filename;
	int id;
	unsigned char * data;
	GLenum colorFormat;
	GLuint outputName;
	GLuint outputTexture;
	std::string outputPath;
	GLuint mainShader;
	GLuint textureSampler;
};

std::optional<ImageWindowState> LoadImageFile(const char * filepath, GLuint programID, GLuint textureSampler);
bool SaveImageFile(const char * filepath, ImageWindowState * image);
bool ReloadImage(ImageWindowState * image);
