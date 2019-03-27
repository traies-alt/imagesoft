#pragma once
#include <glad/glad.h>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
GLuint RandomTexture(size_t seed, int width, int height);