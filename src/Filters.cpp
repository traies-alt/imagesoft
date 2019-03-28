#include "Filters.h"
#include <iostream>
#include <imgui.h>
#include <SOIL.h>
#include "UIComponents.h"
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
	_subtract = false;
	_factor = glGetUniformLocation(_programID, "factor");
}

void SubstractionFilter::RenderUI()
{
	ImGui::Checkbox("Subtract?", &_subtract);

	if (ImGui::Button("Choose image")) {
		ImGui::OpenPopup("Choose image");
	}
	if (ImGui::BeginPopup("Choose image")) {
		std::filesystem::path texFile;
		if (SimpleFileNavigation(_path, texFile)) {
			GLuint _secondTex = SOIL_load_OGL_texture(
				texFile.string().c_str(),
				SOIL_LOAD_AUTO,
				SOIL_CREATE_NEW_ID,
				SOIL_FLAG_MIPMAPS
			);
			if (_secondTex != 0) {
				glUseProgram(_programID);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, _secondTex);
				glUniform1i(_secondSampler, 1);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
}

GLuint SubstractionFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);

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

void ScalarFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Scalar.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_factorGl = glGetUniformLocation(_programID, "factor");
	glUseProgram(_programID);
	glUniform1f(_factorGl, _factor);
}

void ScalarFilter::RenderUI()
{
	if(ImGui::InputFloat("Factor", &_factor, 0.1, 1, 2)) {
		glUseProgram(_programID);
		glUniform1f(_factorGl, _factor);
	}
}

GLuint ScalarFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void DynamicRangeCompressionFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/DynamicRangeCompression.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glC = glGetUniformLocation(_programID, "c");
}

void DynamicRangeCompressionFilter::RenderUI()
{
	ImGui::Checkbox("Calc. min/maxes every frame?", &_calcToggle);
}

GLuint DynamicRangeCompressionFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	if (_calcToggle) {
		GetMinMaxRGB(prevTexture, _width, _height, _minr, _ming, _minb, _maxr, _maxg, _maxb);
		float cr = 1.0 / (log(1.0 + _maxr / 256.0)),
					cg = 1.0 / (log(1.0 + _maxg / 256.0)),
					cb = 1.0 / (log(1.0 + _maxb / 256.0));
		
		glUniform3f(_glC, cr, cg, cb);
	}

	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void ThresholdFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Threshold.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glThreshold = glGetUniformLocation(_programID, "threshold");
	glUseProgram(_programID);
	glUniform3f(_glThreshold, _threshold[0], _threshold[1], _threshold[2]);
}

void ThresholdFilter::RenderUI()
{
	if (ImGui::ColorEdit3("Threshold", _threshold)) {
		glUseProgram(_programID);
		glUniform3f(_glThreshold, _threshold[0], _threshold[1], _threshold[2]);
	}
}

GLuint ThresholdFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void ContrastFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Contrast.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	glUseProgram(_programID);
	_glLower = glGetUniformLocation(_programID, "lower");
	glUniform3f(_glLower, _lower[0], _lower[1], _lower[2]);
	_glLowerTo = glGetUniformLocation(_programID, "lowerTo");
	glUniform3f(_glLowerTo, _lowerTo[0], _lowerTo[1], _lowerTo[2]);
	_glHigher = glGetUniformLocation(_programID, "higher");
	glUniform3f(_glHigher, _higher[0], _higher[1], _higher[2]);
	_glHigherTo = glGetUniformLocation(_programID, "higherTo");
	glUniform3f(_glHigherTo, _higherTo[0], _higherTo[1], _higherTo[2]);
}

void ContrastFilter::RenderUI()
{
	if (ImGui::ColorEdit3("Lower", _lower)) {
		glUseProgram(_programID);
		glUniform3f(_glLower, _lower[0], _lower[1], _lower[2]);
	}
	if (ImGui::ColorEdit3("Lower to", _lowerTo)) {
		glUseProgram(_programID);
		glUniform3f(_glLowerTo, _lowerTo[0], _lowerTo[1], _lowerTo[2]);
	}
	if (ImGui::ColorEdit3("Higher", _higher)) {
		glUseProgram(_programID);
		glUniform3f(_glHigher, _higher[0], _higher[1], _higher[2]);
	}
	if (ImGui::ColorEdit3("Higher to", _higherTo)) {
		glUseProgram(_programID);
		glUniform3f(_glHigherTo, _higherTo[0], _higherTo[1], _higherTo[2]);
	}
}

GLuint ContrastFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}


void EqualizationFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/Equalization.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	glUseProgram(_programID);
	_eqSampler = glGetUniformLocation(_programID, "eqSampler");
	glGenTextures(1, &_eqTexture);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, _eqTexture);
	glUniform1i(_eqSampler, 1);
}

static void Eq(float * h, unsigned char * ha, int total) 
{
	float temp[256] = { 0 };
	int acum = 0;
	for (int i = 0; i < 256; i++) {
		acum += h[i];
		temp[i] = acum / (float) total;
	}

	for (int i = 0; i < 256; i++) {
		ha[i] = (unsigned char)((temp[i] - temp[0] ) / (1 - temp[0]) * 255);
	}
}

void EqualizationFilter::RenderUI()
{
}

GLuint EqualizationFilter::ApplyFilter(GLuint prevTexture)
{
	{
		float hr[256] = { 0 }, hg[256] = { 0 }, hb[256] = { 0 } ;
		unsigned char har[256], hag[256], hab[256] ;

		int total = _width * _height;
		GetHistogramAll(prevTexture, _width, _height, hr, hg, hb);
		Eq(hr, har, total); Eq(hg, hag, total); Eq(hb, hab, total);

		unsigned char pixels[256 * 3] = { 0 };
		for (int i = 0; i < 256; i++) {
			pixels[i * 3] = har[i];
			pixels[i * 3 + 1] = hag[i];
			pixels[i * 3 + 2] = hab[i];
		}
		glBindTexture(GL_TEXTURE_1D, _eqTexture);

		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB ,256, NULL, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		glBindTexture(GL_TEXTURE_1D, 0);
	}	
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void ExponentialNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/ExponentialNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	_randomTex = RandomTexture(_seed, _width, _height);
	_glLambda = glGetUniformLocation(_programID, "lambda");
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);
	glUniform1f(_glLambda, _lambda);
}

void ExponentialNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed)) {
		_randomTex = RandomTexture(_seed, _width, _height);
	}
	if(ImGui::InputFloat("Lambda", &_lambda)) {
		glUseProgram(_programID);
		glUniform1f(_glLambda, _lambda);
	}
}

GLuint ExponentialNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1f(_randomSampler, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void RayleighNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/RayleighNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	_randomTex = RandomTexture(_seed, _width, _height);
	_glXi = glGetUniformLocation(_programID, "xi");
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);
	glUniform1f(_glXi, _xi);
}

void RayleighNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed)) {
		_randomTex = RandomTexture(_seed, _width, _height);
	}
	if(ImGui::InputFloat("Xi", &_xi)) {
		glUseProgram(_programID);
		glUniform1f(_glXi, _xi);
	}
}

GLuint RayleighNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1f(_randomSampler, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}

void GaussianNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/Passthrough.vert", "./src/GaussianNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	_randomTex = RandomTexture(_seed, _width, _height);
	_randomSampler2 = glGetUniformLocation(_programID, "randomSampler2");
	_randomTex2 = RandomTexture(_seed2, _width, _height);
	_glSigma = glGetUniformLocation(_programID, "sigma");
	_glMu = glGetUniformLocation(_programID, "mu");
	glUseProgram(_programID);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _randomTex2);
	glUniform1i(_randomSampler2, 2);

	glUniform1f(_glSigma, _sigma) ;
	glUniform1f(_glMu, _mu) ;
}

void GaussianNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed)) {
		_randomTex = RandomTexture(_seed, _width, _height);
	}
	if(ImGui::InputFloat("Sigma", &_sigma)) {
		glUseProgram(_programID);
		glUniform1f(_glSigma, _sigma);
	}
	if(ImGui::InputFloat("Mu", &_mu)) {
		glUseProgram(_programID);
		glUniform1f(_glMu, _mu);
	}
}

GLuint GaussianNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glViewport(0,0,_width,_height);
	glUseProgram(_programID);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _randomTex2);
	glUniform1i(_randomSampler2, 2);

	DrawTexturedTriangles(prevTexture, _textureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return _outputTexture;
}