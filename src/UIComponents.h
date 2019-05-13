#pragma once

#include <string>
#include <functional>
#include "glad/glad.h"
#include "ImageWindowState.h"
#include "FilesystemAdapter.h"

bool SimpleFileNavigation(std::string &path, fs::path &outFile, bool directory);
bool ImageWindow(ImageWindowState &im, GLuint vertexBuffer, GLuint uvbuffer);
