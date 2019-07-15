#pragma once
#include <optional>
#include <glad/glad.h>  // Initialize with gladLoadGL()
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <tuple>
#include <algorithm>
#include "Filters.h"

struct IFilter;

typedef unsigned char u_char;

struct ImageWindowState {
	GLuint _texture;
	int width, height;
	float zoom;
	const char * filename;
	int id;
	unsigned char * _data;
	GLenum colorFormat;
	std::string outputPath;
	std::vector<IFilter*> filters;
	int histogramBand = 0;
	bool _calcHistogram = false;
	float _hist[256] = {0};
	int _maxVal = 255;
	int _histStart = 0, _histEnd = 256;

	ImageWindowState(
			GLuint _texture,
			int width,
			int height,
			float zoom,
			const char *filename,
			int id,
			unsigned char *_data,
			GLenum colorFormat,
			std::string outputPath,
			std::vector<IFilter *> filters) :
			_texture(_texture), width(width), height(height), zoom(zoom), filename(filename), id(id), _data(_data),
			colorFormat(colorFormat), outputPath(std::move(outputPath)), filters(std::move(filters)) {}

	virtual GLuint texture() {
		return _texture;
	}

	virtual unsigned char * data() {
		return _data;
	}
};
struct ImageWindowStateVideo: ImageWindowState {
	GLuint* _textures;
	unsigned char ** _data;
	size_t currentFrame = 0;
	size_t frames = 0;

	ImageWindowStateVideo(
			GLuint _texture,
			int width,
			int height,
			float zoom,
			const char *filename,
			int id,
			unsigned char *_data,
			GLenum colorFormat,
			const std::string &outputPath,
			std::vector<IFilter *> &filters,
			GLuint *_textures,
			unsigned char **_data1,
			size_t frames)
			: ImageWindowState(_texture, width, height, zoom, filename, id, _data, colorFormat, outputPath, filters),
			  _textures(_textures), _data(_data1), frames(frames) {

		currentFrame = 0;
	}

	GLuint texture() override {
		return _textures[currentFrame];
	}

	unsigned char * data() override {
		return _data[currentFrame];
	}

	void nextFrame() {
		currentFrame = std::min(currentFrame + 1, frames - 1);
	}

	void prevFrame() {
		currentFrame = (currentFrame - 1)%frames;
	}
};

std::optional<ImageWindowStateVideo> LoadVideoFile(const char* path);
std::optional<ImageWindowState> LoadImageFile(const char * filepath);
std::optional<ImageWindowState> LoadImageFileRaw(const char * filepath, int width, int height);
bool SaveImageFile(const char * filepath, ImageWindowState * image);
bool ReloadImage(ImageWindowState * image);
std::optional<ImageWindowState> CreateImage(unsigned char * pixels, int w, int h);
void fillBuffer(unsigned char* buffer, int w, int h,
				const std::function<std::tuple<u_char, u_char, u_char >(int, int)> &value);
