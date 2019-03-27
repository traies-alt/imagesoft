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

	char * _name;
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
		_name = "Main";
	}

	void InitShader() override;
	void RenderUI() override;
	GLuint ApplyFilter(GLuint prevTexture) override;
};

struct SubstractionFilter: IFilter {
	GLuint _textureSampler;
	GLuint _secondTex;
	GLuint _secondSampler;
	GLuint _factor;
	bool _subtract;
	
	SubstractionFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Substraction";
	}

	void InitShader() override;
	void RenderUI() override;
	GLuint ApplyFilter(GLuint prevTexture) override;
};

struct NegativeFilter: IFilter {
	GLuint _textureSampler;
	NegativeFilter(int w, int h) {
		_width = w;
		_height = h;
		_name = "Negative";
	}

	void InitShader() override;
	void RenderUI() override;
	GLuint ApplyFilter(GLuint prevTexture) override;
};