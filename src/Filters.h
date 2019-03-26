#pragma once 
#include <glad/glad.h>  

void InitVertexBuffer();

/**
 * Every filter does the following:
 * * Initializes the shader.		
 * * Keeps track of necessary state for each shader.
 * * Render UI controls.
 * 
 **/
struct IFilter {
	int _width;
	int _height;
	
	GLuint _programID;
	GLuint _outputFramebuffer;
	GLuint _outputTexture;
	virtual void InitShader() = 0;
	virtual void RenderUI() = 0;
	virtual GLuint ApplyFilter(GLuint prevTexture) = 0;
};

struct MainFilter: IFilter {
	GLuint _textureSampler;

	MainFilter(int w, int h) {
		_width = w;
		_height = h;
	}

	void InitShader() override;
	void RenderUI() override;
	GLuint ApplyFilter(GLuint prevTexture) override;
};
