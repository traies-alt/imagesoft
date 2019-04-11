#pragma once
#include <optional>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <tuple>

#include "Filters.h"

typedef unsigned char u_char;

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
	int histogramBand = 0;
	bool _calcHistogram = true;
	float _hist[256] = {0};
	int _maxVal = 255;
	int _histStart = 0, _histEnd = 256;
};

std::optional<ImageWindowState> LoadImageFile(const char * filepath);
std::optional<ImageWindowState> LoadImageFileRaw(const char * filepath, int width, int height);
bool SaveImageFile(const char * filepath, ImageWindowState * image);
bool ReloadImage(ImageWindowState * image);
std::optional<ImageWindowState> CreateImage(unsigned char * pixels, int w, int h);
void fillBuffer(unsigned char* buffer, int w, int h,
				const std::function<std::tuple<u_char, u_char, u_char >(int, int)> &value);
