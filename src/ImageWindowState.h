#pragma once
#include <optional>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <memory>
#include <string>
#include <vector>

#include "Filters.h"

struct ImageWindowState {
	GLuint texture;
	int width, height;
	float zoom;
	const char * filename;
	int id;
	unsigned char * data;
	GLenum colorFormat;
	std::string outputPath;
	std::vector<IFilter*> filters;
};

std::optional<ImageWindowState> LoadImageFile(const char * filepath);
// std::optional<ImageWindowState> LoadImageFileRaw(const char * filepath);
bool SaveImageFile(const char * filepath, ImageWindowState * image);
bool ReloadImage(ImageWindowState * image);
