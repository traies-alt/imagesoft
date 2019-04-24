#include "Filters.h"
#include <iostream>
#include <imgui.h>
#include <SOIL.h>
#include <string>
#include "UIComponents.h"
#include "ShaderLayer.h"
#define _USE_MATH_DEFINES
#include <math.h>

static GLuint vertexbuffer;
static GLuint uvbuffer;

using namespace std;
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

void ApplyTexture(GLuint programId, GLuint textureId, GLuint samplerId) 
{
	glUseProgram(programId);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glUniform1i(samplerId, 1);
}

void MainFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/SimpleFragmentShader.fragmentshader");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");

	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
}

void MainFilter::RenderUI(){}

void MainFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void SingleBandFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/SingleBand.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");

	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glBand = glGetUniformLocation(_programID, "band");
}

void SingleBandFilter::RenderUI(){
	if(ImGui::Selectable("Greyscale", _band == 0)) {
		_band = 0;
	} else if (ImGui::Selectable("Red", _band == 1)) {
		_band = 1;
	} else if (ImGui::Selectable("Green", _band == 2)) {
		_band = 2;
	} else if (ImGui::Selectable("Blue", _band == 3)) {
		_band = 3;
	}
}

void SingleBandFilter::ApplyFilter(GLuint prevTexture)
{
	glUniform1i(_glBand, _band);
	DrawTexturedTriangles(prevTexture, _textureSampler);
}


void SubstractionFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Substract.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	GLuint textures[1];
	glGenTextures(1, textures);
	_secondTex = RandomTexture(100, _width, _height, textures[1]);
	_secondSampler = glGetUniformLocation(_programID, "secondSampler");
	_subtract = false;
	_factor = glGetUniformLocation(_programID, "factor");
	
	_glMin = glGetUniformLocation(_programID, "min");
	glUniform3f(_glMin, _min[0], _min[1], _min[2]);
	_glMax = glGetUniformLocation(_programID, "max");
	glUniform3f(_glMax, _max[0], _max[1], _max[2]);
}

void SubstractionFilter::RenderUI()
{
	ImGui::Checkbox("Subtract?", &_subtract);

	if (ImGui::Button("Choose image")) {
		ImGui::OpenPopup("Choose image");
	}
	if (ImGui::BeginPopup("Choose image")) {
		fs::path texFile;
		if (SimpleFileNavigation(_path, texFile)) {
			GLuint _secondTex = SOIL_load_OGL_texture(
				texFile.string().c_str(),
				SOIL_LOAD_AUTO,
				SOIL_CREATE_NEW_ID,
				SOIL_FLAG_MIPMAPS
			);
			if (_secondTex != 0) {
				ApplyTexture(_programID, _secondTex, _secondSampler);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
	if (ImGui::Button("Linear fitting")) {
		GetMinMaxForSum(_textureSampler, _secondSampler, _width, _height, _min, _max);
		glUseProgram(_programID);
		glUniform3f(_glMin, _min[0], _min[1], _min[2]);
		glUseProgram(_programID);
		glUniform3f(_glMax, _max[0], _max[1], _max[2]);			
	}
	ImGui::Text("Min: %f %f %f, Max: %f %f %f", _min[0], _min[1], _min[2], _max[0], _max[1], _max[2]);
}

void SubstractionFilter::ApplyFilter(GLuint prevTexture)
{
	glUniform1i(_factor, _subtract ? -1 : 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void NegativeFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Negative.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
}

void NegativeFilter::RenderUI()
{
}

void NegativeFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void ScalarFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Scalar.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_factorGl = glGetUniformLocation(_programID, "factor");
	_glC = glGetUniformLocation(_programID, "c");
	_glDynamicRange = glGetUniformLocation(_programID, "dynamicRange");
	
	glUseProgram(_programID);
	glUniform1f(_factorGl, _factor);
	glUniform3f(_glC, 1, 1, 1);
	glUniform1i(_glDynamicRange, 0);
}

void ScalarFilter::RenderUI()
{
	if(ImGui::InputFloat("Factor", &_factor, 0.1, 1, 2)) {
		glUseProgram(_programID);
		glUniform1f(_factorGl, _factor);
	}
	if (ImGui::Checkbox("Dynamic range adjust", &_dynamicRange)) {
		glUseProgram(_programID);		
		if (_dynamicRange) {
			glUniform1i(_glDynamicRange, 1);		
		} else {
			glUniform1i(_glDynamicRange, 0);
		}
	}
}

void ScalarFilter::ApplyFilter(GLuint prevTexture)
{
	if (_dynamicRange) {
		unsigned char _minr, _ming, _minb, _maxr, _maxg, _maxb;
		GetMinMaxRGB(prevTexture, _width, _height, _minr, _ming, _minb, _maxr, _maxg, _maxb);
		float cr = 1.0 / (log(1.0 + min(255.0f, _maxr * _factor) / 255.0)),
					cg = 1.0 / (log(1.0 + min(255.0f, _maxg * _factor) / 255.0)),
					cb = 1.0 / (log(1.0 + min(255.0f, _maxb * _factor) / 255.0));
		glUniform3f(_glC, cr, cg, cb);
	}
	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void DynamicRangeCompressionFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/DynamicRangeCompression.frag");
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

void DynamicRangeCompressionFilter::ApplyFilter(GLuint prevTexture)
{
	if (_calcToggle) {
		GetMinMaxRGB(prevTexture, _width, _height, _minr, _ming, _minb, _maxr, _maxg, _maxb);
		float cr = 1.0 / (log(1.0 + _maxr / 255.0)),
					cg = 1.0 / (log(1.0 + _maxg / 255.0)),
					cb = 1.0 / (log(1.0 + _maxb / 255.0));
		
		glUniform3f(_glC, cr, cg, cb);
	}

	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void GammaFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Gamma.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glGamma = glGetUniformLocation(_programID, "gamma");
	glUseProgram(_programID);
	glUniform1f(_glGamma, _gamma);
}

void GammaFilter::RenderUI()
{
	if (ImGui::InputFloat("Gamma", &_gamma)) {
		glUseProgram(_programID);
		glUniform1f(_glGamma, _gamma);
	}
}

void GammaFilter::ApplyFilter(GLuint prevTexture)
{

	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void ThresholdFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Threshold.frag");
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

void ThresholdFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void ContrastFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Contrast.frag");
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

void ContrastFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);

}


void EqualizationFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Equalization.frag");
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
	if (ImGui::Button("Equalize")) {
		_eqCalcTexture = true;
	}
}

void EqualizationFilter::ApplyFilter(GLuint prevTexture)
{
	
	if (_eqCalcTexture){
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
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_1D, _eqTexture);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB ,256, NULL, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		_eqCalcTexture = false;
	}
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_1D, _eqTexture);
	glUniform1i(_eqSampler, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void ExponentialNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/ExponentialNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	GLuint textures[1];
	glGenTextures(1, textures);
	_randomTex = RandomTexture(_seed, _width, _height, textures[1]);
	_glLambda = glGetUniformLocation(_programID, "lambda");
	_glContamination = glGetUniformLocation(_programID, "contamination");
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);
	glUniform1f(_glLambda, _lambda);
	glUniform1f(_glContamination, _glContamination);
}

void ExponentialNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed, 1, 10, 1)) {
		_randomTex = RandomTexture(_seed, _width, _height, _randomTex);
	}
	if(ImGui::InputFloat("Lambda", &_lambda, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glLambda, _lambda);
	}
	if(ImGui::InputFloat("contamination", &_contamination, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glContamination, _contamination);
	}

}

void ExponentialNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1f(_randomSampler, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void RayleighNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/RayleighNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	_glContamination = glGetUniformLocation(_programID, "contamination");
	GLuint textures[1];
	glGenTextures(1, textures);
	_randomTex = RandomTexture(_seed, _width, _height, textures[1]);
	_glXi = glGetUniformLocation(_programID, "xi");
	glUseProgram(_programID);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);
	glUniform1f(_glXi, _xi);
	glUniform1f(_glContamination, _contamination);
}

void RayleighNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed, 1, 10, 1)) {
		_randomTex = RandomTexture(_seed, _width, _height, _randomTex);
	}
	if(ImGui::InputFloat("Xi", &_xi, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glXi, _xi);
	}
	if(ImGui::InputFloat("Contamination", &_contamination, 0.01f, 0.1f,2)) {
		glUseProgram(_programID);
		glUniform1f(_glContamination, _contamination);
	}
}

void RayleighNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1f(_randomSampler, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void GaussianNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/GaussianNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	_randomSampler2 = glGetUniformLocation(_programID, "randomSampler2");
	
	GLuint textures[2];
	glGenTextures(2, textures);
	_randomTex = RandomTexture(_seed[0], _width, _height, textures[0]);
	_randomTex2 = RandomTexture(_seed[1], _width, _height, textures[1]);
	
	_glSigma = glGetUniformLocation(_programID, "sigma");
	_glMu = glGetUniformLocation(_programID, "mu");
	_glContamination = glGetUniformLocation(_programID, "contamination");
	glUseProgram(_programID);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _randomTex2);


	glUniform1i(_randomSampler, 1);
	glUniform1i(_randomSampler2, 2);

	glUniform1f(_glSigma, _sigma) ;
	glUniform1f(_glMu, _mu) ;
	glUniform1f(_glContamination, _contamination) ;
}

void GaussianNoiseFilter::RenderUI()
{
	if(ImGui::InputInt2("Seed", _seed)) {
		_randomTex = RandomTexture(_seed[0], _width, _height, _randomTex);
		_randomTex2 = RandomTexture(_seed[1], _width, _height, _randomTex2);
	}
	if(ImGui::InputFloat("Sigma", &_sigma, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glSigma, _sigma);
	}
	if(ImGui::InputFloat("Mu", &_mu, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glMu, _mu);
	}
	if(ImGui::InputFloat("Contamination", &_contamination, 0.01f, 0.1f, 2 )) {
		glUseProgram(_programID);
		glUniform1f(_glContamination, _contamination);
	}
}

void GaussianNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, _randomTex2);
	glUniform1i(_randomSampler2, 2);

	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void SaltAndPepperNoiseFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/SaltAndPepperNoise.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_randomSampler = glGetUniformLocation(_programID, "randomSampler");
	GLuint textures[1];
	glGenTextures(1, textures);
	_randomTex = RandomTexture(_seed, _width, _height, textures[0]);
	
	_glContamination1 = glGetUniformLocation(_programID, "contamination1");
	_glContamination2 = glGetUniformLocation(_programID, "contamination2");

	glUseProgram(_programID);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);

	glUniform1f(_glContamination1, _contamination1) ;
	glUniform1f(_glContamination2, _contamination2) ;
}

void SaltAndPepperNoiseFilter::RenderUI()
{
	if(ImGui::InputInt("Seed", &_seed)) {
		_randomTex = RandomTexture(_seed, _width, _height, _randomTex);
	}
	if(ImGui::InputFloat("P1", &_contamination1, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glContamination1, _contamination1);
	}

	if(ImGui::InputFloat("P2", &_contamination2, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glContamination2, _contamination2);
	}
}

void SaltAndPepperNoiseFilter::ApplyFilter(GLuint prevTexture)
{
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _randomTex);
	glUniform1i(_randomSampler, 1);

	DrawTexturedTriangles(prevTexture, _textureSampler);

}

void MaskFilter::InitMask()
{
	_maskWeightsTexture = WeightedTexture(_maskSize, _weights, _maskWeightsTexture);
	glUseProgram(_programID);
	glUniform1f(_glMaskSize, _maskSize);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
	glUniform1f(_glMaskDivision, _maskDivision);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _maskWeightsTexture);
	glUniform1i(_glMaskSampler, 1);
}

void MaskFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Mask.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glMaskSize = glGetUniformLocation(_programID, "maskSize");	
	_glWidth = glGetUniformLocation(_programID, "width");	
	_glHeight = glGetUniformLocation(_programID, "height");
	_glMaskSampler = glGetUniformLocation(_programID, "maskWeights");
	_glMaskDivision = glGetUniformLocation(_programID, "maskDivision");
	
	glGenTextures(1, &_maskWeightsTexture);
	InitMask();
}

float GaussianWeight(float x, float y, float sqrSigma)
{
	return exp(-(x * x + y * y) / 2 * (sqrSigma)) / (1.0 / (2 * M_PI * sqrSigma));
}

void MaskFilter::RenderUI()
{
	bool maskChanged = false;
	maskChanged |= ImGui::InputInt("Mask Size", &_maskSize);
	maskChanged |= ImGui::InputFloat("Mask division", &_maskDivision);
	ImGui::PushItemWidth(50);
	for (int i = 0; i < _maskSize; i++) {
		for (int j = 0; j < _maskSize; j++) {
			maskChanged |= ImGui::InputFloat(string("Mask Row").append(to_string(i * _maskSize + j)).c_str(), &_weights[i * _maskSize + j]);
			ImGui::SameLine();
		}
		ImGui::Text("");
	}
	ImGui::PopItemWidth();

	if (maskChanged){
		InitMask();
	}
}

void MeanFilter::RenderUI()
{
	MaskFilter::RenderUI();
	bool maskChanged = false;

	if (ImGui::Button("High pass")) {
		maskChanged = true;
		_maskDivision = _maskSize * _maskSize;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if (i == _maskSize / 2 && j == _maskSize / 2) {
					_weights[i * _maskSize + j] = _maskDivision;
				} else {
					_weights[i * _maskSize + j] = -1;
				}
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Mean")) {
		maskChanged = true;
		_maskDivision = _maskSize * _maskSize;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				_weights[i * _maskSize + j] = 1;
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Gaussian")) {
		maskChanged = true;
		float sqrSigma = _sigma * _sigma;
		_maskDivision = 0;
		for (float i = 0; i < _maskSize; i++) {
			for (float j = 0; j < _maskSize; j++) {
				float normalX = i / (_maskSize - 1) - 0.5;
				float normalY = j / (_maskSize - 1) - 0.5;
				float w = GaussianWeight(normalX, normalY, sqrSigma);
				_maskDivision += w;
				_weights[(int)(i * _maskSize + j)] = w;
			}
		}
	}
	ImGui::SameLine();
	ImGui::InputFloat("Sigma", &_sigma);

	if (maskChanged){
		InitMask();
	}
}

void BorderFilter::RenderUI()
{
	MaskFilter::RenderUI();
	bool maskChanged = false;

	if (ImGui::Button("Roberts Horizontal")) {
		maskChanged = true;
		_maskSize = 2;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if(j!=i) {
					_weights[i * _maskSize + j] = 0;
				} else if(i==0){
					_weights[i * _maskSize + j] = 1;
				} else {
					_weights[i * _maskSize + j] = -1;
				}
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Roberts Vertical")) {
		maskChanged = true;
		_maskSize = 2;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if(j==i) {
					_weights[i * _maskSize + j] = 0;
				} else if(i==0){
					_weights[i * _maskSize + j] = 1;
				} else {
					_weights[i * _maskSize + j] = -1;
				}
			}
		}
	}

	if (ImGui::Button("Prewitt Horizontal")) {
		maskChanged = true;
		_maskSize = 3;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				_weights[i * _maskSize + j] = i - 1;
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Prewitt Vertical")) {
		maskChanged = true;
		_maskSize = 3;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				_weights[i * _maskSize + j] = j - 1;
			}
		}
	}

	if (ImGui::Button("Sobel Horizontal")) {
		maskChanged = true;
		_maskSize = 3;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if(j==1) {
					_weights[i * _maskSize + j] = (i - 1)*2;
				} else {
					_weights[i * _maskSize + j] = i - 1;
				}
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Sobel Vertical")) {
		maskChanged = true;
		_maskSize = 3;
		_maskDivision = 1;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if(i==1) {
					_weights[i * _maskSize + j] = (j - 1)*2;
				} else {
					_weights[i * _maskSize + j] = j - 1;
				}
			}
		}
	}


	if (maskChanged){
		InitMask();
	}
}

void MaskFilter::ApplyFilter(GLuint prevTexture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _maskWeightsTexture);
	glUniform1i(_glMaskSampler, 1);

	glUseProgram(_programID);
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void MedianFilter::InitMask()
{
	_maskDivision = (float)_maskSize * _maskSize;
	_maskWeightsTexture = WeightedTexture(_maskSize, _weights, _maskWeightsTexture);
	
	glUseProgram(_programID);
	glUniform1f(_glMaskSize, _maskSize);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
	glUniform1f(_glMaskDivision, _maskDivision);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _maskWeightsTexture);
	glUniform1i(_glMaskSampler, 1);
}

void MedianFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/MedianMask.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glMaskSize = glGetUniformLocation(_programID, "maskSize");	
	_glWidth = glGetUniformLocation(_programID, "width");	
	_glHeight = glGetUniformLocation(_programID, "height");
	_glMaskSampler = glGetUniformLocation(_programID, "maskWeights");
	_glMaskDivision = glGetUniformLocation(_programID, "maskDivision");
	
	glGenTextures(1, &_maskWeightsTexture);
	InitMask();
}

void MedianFilter::RenderUI()
{
	bool maskChanged = false;
	maskChanged |= ImGui::InputInt("Mask Size", &_maskSize);
	ImGui::PushItemWidth(50);
	for (int i = 0; i < _maskSize; i++) {
		for (int j = 0; j < _maskSize; j++) {
			maskChanged |= ImGui::InputFloat(string("Mask Row").append(to_string(i * _maskSize + j)).c_str(), &_weights[i * _maskSize + j]);
			ImGui::SameLine();
		}
		ImGui::Text("");
	}
	ImGui::PopItemWidth();
	if (maskChanged){
		InitMask();
	}
}

void MedianFilter::ApplyFilter(GLuint prevTexture)
{
	ApplyTexture(_programID, _maskWeightsTexture, _glMaskSampler);
	DrawTexturedTriangles(prevTexture, _textureSampler);
}
