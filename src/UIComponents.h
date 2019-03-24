#pragma once

#include <string>
#include <functional>
#include <filesystem>
#include "glad/glad.h"
#include "ImageWindowState.h"

bool SimpleFileNavigation(std::string &path, std::filesystem::path &outFile);
bool ImageWindow(ImageWindowState &im, GLuint vertexBuffer, GLuint uvbuffer);
