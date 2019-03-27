#include "Filters.h"
#include <iostream>
#include <imgui.h>
#include "ShaderLayer.h"

static GLuint vertexbuffer;
static GLuint uvbuffer;

void InitVertexBuffer()
{
	const GLfloat g_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	static const GLfloat g_uv_buffer_data[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f,  1.0f,
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
}

void DrawTexturedTriangles(GLuint ogTexture, GLuint nextShaderTextureSampler)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ogTexture);

	// Texture Sampler
	glUniform1i(nextShaderTextureSampler, 0);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(
		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
		2,                                // size : U+V => 2
		GL_FLOAT,                         // type
		GL_FALSE,                         // normalized?
		0,                                // stride
		(void*)0                          // array buffer offset
	);
	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFlush();
}


void MainFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/SimpleFragmentShader.fragmentshader");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");

	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
}

void MainFilter::RenderUI()
{

}

GLuint MainFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}


void SubstractionFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Substract.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_secondTex = RandomTexture(100, _width, _height);
	_secondSampler = glGetUniformLocation(_programID, "secondSampler");
	_factor = glGetUniformLocation(_programID, "factor");
	_subtract = false;
}

void SubstractionFilter::RenderUI()
{
	ImGui::Checkbox("Subtract", &_subtract);
}

GLuint SubstractionFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _secondTex);
	glUniform1i(_secondSampler, 1);

	glUniform1i(_factor, _subtract ? -1 : 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void NegativeFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Negative.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
}

void NegativeFilter::RenderUI()
{
}

GLuint NegativeFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}


