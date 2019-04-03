#pragma once
#include <glad/glad.h>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);
GLuint RandomTexture(size_t seed, int width, int height, GLuint old);
bool InitOutputTexture(int w, int h, GLuint& oFrameBuffer, GLuint& oTexture);
void GetMinMaxRGB(GLuint texture, int w, int h, unsigned char& minr, unsigned char& ming, unsigned char& minb, unsigned char& maxr, unsigned char& maxg, unsigned char& maxb);
int GetHistogram(GLuint texture, int w, int h, int band, float * hist);
void GetHistogramAll(GLuint texture, int w, int h, float * hr, float * hg, float * hb);
void GetMinMaxForSum(GLuint texture1, GLuint texture2,  int w, int h, float * min, float * max);
GLuint WeightedTexture(int maskSize, float * weights, GLuint texture);
