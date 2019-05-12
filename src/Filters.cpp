#include "Filters.h"
#include <iostream>
#include <imgui.h>
#include <SOIL.h>
#include <string>
#include "UIComponents.h"
#include "ShaderLayer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "KnownMask.h"

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

void ApplyTextureNumber(GLuint programId, GLuint textureId, GLuint samplerId, GLenum textureEnum, int textureNumber)
{
	glUseProgram(programId);
	glActiveTexture(textureEnum);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glUniform1i(samplerId, textureNumber);
}

void ApplyTexture(GLuint programId, GLuint textureId, GLuint samplerId) {
	ApplyTextureNumber(programId, textureId, samplerId, GL_TEXTURE1, 1);
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
	_textureSampler = static_cast<GLuint>(glGetUniformLocation(_programID, "myTextureSampler"));
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

void GlobalThresholdFilter::InitShader()
{
    _programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Threshold.frag");
    _textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
    if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
        std::cout << "Error rendering to texture." << std::endl;
        return;
    }
    _glThreshold = glGetUniformLocation(_programID, "threshold");



    glUseProgram(_programID);
    glUniform3f(_glThreshold, _currentThreshold[0], _currentThreshold[1], _currentThreshold[2]);
}

void GlobalThresholdFilter::RenderUI()
{

    if (ImGui::ColorEdit3("Global Threshold", _initialThreshold)) {
		if(_thresholdError > 0
			&& _initialThreshold[0] > 0 && _initialThreshold[1] > 0 && _initialThreshold[2] > 0
		   	&& _initialThreshold[0] < 1 && _initialThreshold[1] < 1 && _initialThreshold[2]  < 1) {
			_thresholdChanged = true;
		}
    }

	if (ImGui::InputFloat("Global Threshold Error", &_thresholdError)) {
		if(_thresholdError > 0
		   && _initialThreshold[0] > 0 && _initialThreshold[1] > 0 && _initialThreshold[2] > 0
		   && _initialThreshold[0] < 1 && _initialThreshold[1] < 1 && _initialThreshold[2]  < 1) {
			_thresholdChanged = true;
		}
	}

	float temp[3] = {_currentThreshold[0], _currentThreshold[1], _currentThreshold[2]};
    ImGui::ColorEdit3("Threshold", _currentThreshold);
	for(int i=0; i<3; i++) {
		_currentThreshold[i] = temp[i];
	}


}

bool almostEqual(float a, float b, float error){
	return fabs(a - b) < error;
}

void GlobalThresholdFilter::ApplyFilter(GLuint prevTexture)
{
	if(_thresholdChanged) {
        for(int i=0; i<3; i++) {
            _currentThreshold[i] = _initialThreshold[i];
        }

		float temp[3] = {-1,-1,-1};
		auto prev_pixels = (unsigned char*)malloc( (size_t)3*_width*_height );
		auto cur_pixels = (unsigned char*)malloc( (size_t)3*_width*_height );


		while(!almostEqual(temp[0], _currentThreshold[0], _thresholdError)
					|| !almostEqual(temp[1], _currentThreshold[1], _thresholdError)
					|| !almostEqual(temp[2], _currentThreshold[2], _thresholdError) ){

			for(int i=0; i<3; i++) {
				temp[i] = _currentThreshold[i];
			}

			glUseProgram(_programID);
			glUniform3f(_glThreshold, _currentThreshold[0], _currentThreshold[1], _currentThreshold[2]);
			DrawTexturedTriangles(prevTexture, _textureSampler);

			//Calculate group values
			float acuWhites[3]  = {0,0,0};
			int countWhites[3]  = {0,0,0};

			float acuBlacks[3]  = {0,0,0};
			int countBlacks[3]  = {0,0,0};


			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, _outputTexture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, cur_pixels);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, prevTexture);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, prev_pixels);

			for(int x=0; x<_width; x++) {
				for(int y=0; y<_height; y++) {
					int pos = x * _width + y;
					for(int i=0; i<3; i++) {
						if(cur_pixels[pos + i] == 0) {
							acuBlacks[i] += prev_pixels[i];
							countBlacks[i] ++;
						} else {
							acuWhites[i] += prev_pixels[i];
							countWhites[i] ++;
						}
					}

				}
			}

			for(int i=0; i<3; i++) {
				if (countBlacks[i] == 0) {
					_currentThreshold[i] = 0.5f * acuWhites[i] / countWhites[i] / 255.0f;
				} else if (countWhites[i] == 0) {
					_currentThreshold[i] = 0.5f * acuBlacks[i] / countBlacks[i] / 255.0f;

				} else {
					_currentThreshold[i] = 0.5f * (acuWhites[i] / countWhites[i] + acuBlacks[i] / countBlacks[i]) / 256.0f;
				}
			}
		}

		free(prev_pixels);
		free(cur_pixels);

        glUseProgram(_programID);
        glUniform3f(_glThreshold, _currentThreshold[0], _currentThreshold[1], _currentThreshold[2]);
        DrawTexturedTriangles(prevTexture, _textureSampler);
		_thresholdChanged = false;
	}
}

void OtsuThresholdFilter::RenderUI() {

	if(ImGui::ColorEdit3("OtsuThreshold", _threshold)) {
		_thresholdChanged = true;
	}

	if(ImGui::Button("Recalculate")) {
		_thresholdChanged = true;
	}

}

void OtsuThresholdFilter::ApplyFilter(GLuint prevTexture)
{
	if(_thresholdChanged) {

		float histA[3][256] = { {0}, {0}, {0} };
		double histD[3][256] = { {0}, {0}, {0} };

		double pixelCount = _width * _height;
		for (int i = 0; i < 3; ++i) {
			GetHistogram(prevTexture, _width, _height, i, histA[i]);
			for (int j = 0; j < 256; ++j) {
				histD[i][j] = histA[i][j]/pixelCount;
			}
		}

		double p[3][256] = {{0}, {0}, {0}};
		for (int i = 0; i < 3; ++i) {
			for (int t = 0; t < 256; ++t) {
				for (int k = 0; k < t; ++k) {
					p[i][t] += histD[i][k];
				}
			}
		}

		double m[3][256] = {{0}, {0}, {0}};
		for (int i = 0; i < 3; ++i) {
			for (int t = 0; t < 256; ++t) {
				for (int k = 0; k < t; ++k) {
					m[i][t] += k*histD[i][k];
				}
			}
		}

		double mg[3] = {m[0][255], m[1][255], m[2][255]};

		for (int i = 0; i < 3; ++i) {
			double max = 0;
			int maxFound = 0;
			double maxSum = 0;

			if (p[i][255] == 0) {
				_threshold[i] = 0;
			} else {
				for (int j = 0; j < 256; ++j) {
					if (almostEqual((float) p[i][j], 0, 0.01) || almostEqual((float) p[i][j], 1, 0.01)) {
						continue;
					}

					double upper = mg[i] * p[i][j] - m[i][j];
					double sig = (upper * upper) / (p[i][j] * (1 - p[i][j]));

					if (sig > max) {
						max = sig;
						maxFound=1;
						maxSum=j;
					}

				}

				if (maxFound == 0) {
					_threshold[i] = 0;
				} else {
					_threshold[i] = (float) (maxSum / maxFound / 255.0);
				}
			}
		}

			glUseProgram(_programID);
		glUniform3f(_glThreshold, _threshold[0], _threshold[1], _threshold[2]);
		DrawTexturedTriangles(prevTexture, _textureSampler);
		_thresholdChanged = false;
	}
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
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

void CombinedMaskFilter::InitMask()
{
	_maskWeightsTexture1 = WeightedTexture(_maskSize1, _weights1, _maskWeightsTexture1);
	_maskWeightsTexture2 = WeightedTexture(_maskSize2, _weights2, _maskWeightsTexture2);
	glUseProgram(_programID);
	glUniform1f(_glMaskSize1, _maskSize1);
	glUniform1f(_glMaskSize2, _maskSize2);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
	glUniform1f(_glMaskDivision, _maskDivision);


	ApplyTextureNumber(_programID, _maskWeightsTexture1, _glMaskSampler1, GL_TEXTURE1, 1);
	ApplyTextureNumber(_programID, _maskWeightsTexture2, _glMaskSampler2, GL_TEXTURE2, 2);
}

void CombinedMaskFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/CombinedMask.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glMaskSize1 = glGetUniformLocation(_programID, "maskSize1");
	_glMaskSize2 = glGetUniformLocation(_programID, "maskSize2");
	_glWidth = glGetUniformLocation(_programID, "width");
	_glHeight = glGetUniformLocation(_programID, "height");
	_glMaskSampler1 = glGetUniformLocation(_programID, "maskWeights1");
	_glMaskSampler2 = glGetUniformLocation(_programID, "maskWeights2");
	_glMaskDivision = glGetUniformLocation(_programID, "maskDivision");

	glGenTextures(1, &_maskWeightsTexture1);
	glGenTextures(1, &_maskWeightsTexture2);
	InitMask();
}

void CombinedMaskFilter::ApplyFilter(GLuint prevTexture)
{
	ApplyTextureNumber(_programID, _maskWeightsTexture1, _glMaskSampler1, GL_TEXTURE1, 1);
	DrawTexturedTriangles(prevTexture, _textureSampler);

	ApplyTextureNumber(_programID, _maskWeightsTexture2, _glMaskSampler2, GL_TEXTURE2, 2);
	DrawTexturedTriangles(prevTexture, _textureSampler);
}


void BorderFilter::RenderUI()
{
	bool maskChanged = false;
	maskChanged |= ImGui::InputInt("Mask Size", &_maskSize1);
	maskChanged |= ImGui::InputFloat("Mask division", &_maskDivision);
	ImGui::PushItemWidth(50);
	for (int i = 0; i < _maskSize1; i++) {
		for (int j = 0; j < _maskSize1; j++) {
			maskChanged |= ImGui::InputFloat(string("Mask Row").append(to_string(i * _maskSize1 + j)).c_str(), &_weights1[i * _maskSize1 + j]);
			ImGui::SameLine();
		}
		ImGui::Text("");
	}

	if (ImGui::Button("Rotate Mask")) {
		maskChanged = true;
		RotateMask(_maskSize1, _weights1);
	}
	ImGui::PopItemWidth();
	ImGui::Text("");


	if (maskChanged){
		InitMask();
	}

	if(_showSecondMask) {
		maskChanged |= ImGui::InputInt("Mask Size", &_maskSize2);
		maskChanged |= ImGui::InputFloat("Mask division", &_maskDivision);
		ImGui::PushItemWidth(50);
		for (int i = 0; i < _maskSize2; i++) {
			for (int j = 0; j < _maskSize2; j++) {
				maskChanged |= ImGui::InputFloat(string("Mask Row").append(to_string(i * _maskSize2 + j)).c_str(),
												 &_weights2[i * _maskSize2 + j]);
				ImGui::SameLine();
			}
			ImGui::Text("");
		}

        if (ImGui::Button("Rotate Mask")) {
            maskChanged = true;
            RotateMask(_maskSize2, _weights2);
        }

        if (ImGui::Button("Rotate Both")) {
            maskChanged = true;
            RotateMask(_maskSize1, _weights1);
            RotateMask(_maskSize2, _weights2);
        }
		ImGui::PopItemWidth();

		if (maskChanged) {
			InitMask();
		}
		ImGui::Text("");
	}

	if (ImGui::Button("Roberts Horizontal")) {
		maskChanged = true;
		_showSecondMask = false;
		_maskDivision = 1;
		setupRobertsHorizontal(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

	ImGui::SameLine();

	if (ImGui::Button("Roberts Vertical")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupRobertsVertical(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

    ImGui::SameLine();

    if (ImGui::Button("Roberts Combined")) {
        maskChanged = true;
        _showSecondMask = true;
        _maskDivision = 1;
        setupRobertsHorizontal(&_maskSize1, _weights1);
        setupRobertsVertical(&_maskSize2, _weights2);
    }

    if (ImGui::Button("Prewitt Horizontal")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupPrewittHorizontal(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

    ImGui::SameLine();

    if (ImGui::Button("Prewitt Vertical")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupPrewittVertical(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

    ImGui::SameLine();

    if (ImGui::Button("Prewitt Combined")) {
        maskChanged = true;
        _showSecondMask = true;
        _maskDivision = 1;
        setupPrewittHorizontal(&_maskSize1, _weights1);
        setupPrewittVertical(&_maskSize2, _weights2);
    }

    if (ImGui::Button("Sobel Horizontal")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupSobelHorizontal(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

    ImGui::SameLine();

    if (ImGui::Button("Sobel Vertical")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupSobelVertical(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
	}

    ImGui::SameLine();

    if (ImGui::Button("Sobel Combined")) {
        maskChanged = true;
        _showSecondMask = true;
        _maskDivision = 1;
        setupSobelHorizontal(&_maskSize1, _weights1);
        setupSobelVertical(&_maskSize2, _weights2);
    }

    if (ImGui::Button("House Mask")) {
        maskChanged = true;
        _showSecondMask = false;
        _maskDivision = 1;
        setupHouseMask(&_maskSize1, _weights1);
        _maskSize2 = _maskSize1;
        clearMask(_maskSize1, _weights2);
    }

    ImGui::SameLine();
    if (ImGui::Button("Kirsh")) {
		maskChanged = true;
		_showSecondMask = false;
		_maskDivision = 1;
		setupKirsh(&_maskSize1, _weights1);
		_maskSize2 = _maskSize1;
		clearMask(_maskSize1, _weights2);
    }

    if (maskChanged){
		InitMask();
	}
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

void LaplaceFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/LaplaceSecondPassMask.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}

	_glWidth = glGetUniformLocation(_programID, "width");
	_glHeight = glGetUniformLocation(_programID, "height");
	_glThreshold = glGetUniformLocation(_programID, "threshold");
	_glMax = glGetUniformLocation(_programID, "maximum");

	_firstPassProgramID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/LaplaceFirstPassMask.frag");
	_firstPassTextureSampler = glGetUniformLocation(_firstPassProgramID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _firstPassFrameBuffer, _firstPassTexture)) {
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glMaskSize = glGetUniformLocation(_firstPassProgramID, "maskSize");
	_glWidthFirstPass = glGetUniformLocation(_firstPassProgramID, "width");
	_glHeightFirstPass = glGetUniformLocation(_firstPassProgramID, "height");
	_glMaskSampler = glGetUniformLocation(_firstPassProgramID, "maskWeights");
	_glMax2 = glGetUniformLocation(_firstPassProgramID, "maximum");
	glGenTextures(1, &_maskWeightsTexture);
	InitMask();
}

void LaplaceFilter::InitMask()
{
		float tempMax = 0;
		float tempMin = 0;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				if(_weights[i + _maskSize * j] > 0){
					tempMax += _weights[i + _maskSize * j];
				} else {
					tempMin += -_weights[i + _maskSize * j];
				}
			}
		}

		_max = max(tempMax, tempMin);

	_maskWeightsTexture = WeightedTexture(_maskSize, _weights, _maskWeightsTexture);

	glUseProgram(_firstPassProgramID);
	glUniform1i(_glMaskSize, _maskSize);
	glUniform1f(_glWidthFirstPass, _width);
	glUniform1f(_glHeightFirstPass, _height);
	glUniform1f(_glMax2, _max);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, _maskWeightsTexture);
	glUniform1i(_glMaskSampler, 1);

	glUseProgram(_programID);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
	glUniform1f(_glMax, _max);
	glUniform1f(_glThreshold, _threshold);
}

float LogWeight(float x, float y, float sigma)
{
	static float twoPiSqrd = sqrt(2.0f * M_PI);
	float xyOverSigma = -(x * x + y * y) / (sigma * sigma);
	float expPart = expf(xyOverSigma / 2.0f);
	float sqrtPart = -(1.0f) / (twoPiSqrd * sigma * sigma * sigma);
	float mainPart = 2.0f + xyOverSigma;
	return expPart * sqrtPart * mainPart;
}

void LaplaceFilter::RenderUI()
{
	bool maskChanged = false;
	if (ImGui::InputInt("Mask Size", &_maskSize)) {
		glUseProgram(_firstPassProgramID);
		glUniform1i(_glMaskSize, _maskSize);
		maskChanged = true;
	}
	if (ImGui::InputFloat("Threshold", &_threshold, 0.01f, 0.1f, 2)) {
		glUseProgram(_programID);
		glUniform1f(_glThreshold, _threshold);
	}
	if(ImGui::InputFloat("Sigma", &_sigma, 0.1f, 1.0f, 1)){
		maskChanged = true;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				_weights[i + _maskSize * j] = LogWeight(i - _maskSize / 2, j - _maskSize / 2, _sigma);
			}
		}
	}

	if (ImGui::Button("Laplacian-Gaussian")) {
		maskChanged = true;
		for (int i = 0; i < _maskSize; i++) {
			for (int j = 0; j < _maskSize; j++) {
				_weights[i + _maskSize * j] = LogWeight(i - _maskSize / 2, j - _maskSize / 2, _sigma);
			}
		}
	}

	if (ImGui::Button("Laplacian")) {
		maskChanged = true;
		static const float weights[] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
		_max = 4;
		_maskSize = 3;
		memcpy(_weights, weights, sizeof(float) * _maskSize * _maskSize);
	}

	ImGui::PushItemWidth(50);
	for (int i = 0; i < _maskSize; i++) {
		for (int j = 0; j < _maskSize; j++) {
			maskChanged |= ImGui::InputFloat(string("Mask Row").append(to_string(i * _maskSize + j)).c_str(), &_weights[i * _maskSize + j]);
			ImGui::SameLine();
		}
		ImGui::Text("");
	}
	ImGui::PopItemWidth();

	if (maskChanged) {
		InitMask();
	}
}

void LaplaceFilter::ApplyFilter(GLuint prevTexture)
{
	glBindFramebuffer(GL_FRAMEBUFFER, _firstPassFrameBuffer);
	glUseProgram(_firstPassProgramID);
	ApplyTexture(_firstPassProgramID, _maskWeightsTexture, _glMaskSampler);
	DrawTexturedTriangles(prevTexture, _firstPassTextureSampler);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
	glUseProgram(_programID);
	DrawTexturedTriangles(_firstPassTexture, _textureSampler);
}

void BilateralFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/BilateralFilter.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture))
	{
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	_glMaskSize = glGetUniformLocation(_programID, "maskSize");
	_glWidth = glGetUniformLocation(_programID, "width");
	_glHeight = glGetUniformLocation(_programID, "height");
	_glSigmaR = glGetUniformLocation(_programID, "sigmaR");
	_glSigmaS = glGetUniformLocation(_programID, "sigmaS");

	glUseProgram(_programID);
	glUniform1i(_glMaskSize, _maskSize);
	glUniform1f(_glSigmaS, _sigmaS);
	glUniform1f(_glSigmaR, _sigmaR);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
}

void BilateralFilter::RenderUI()
{
	if (ImGui::InputInt("Mask Size", &_maskSize))
	{
		glUseProgram(_programID);
		glUniform1i(_glMaskSize, _maskSize);
	}
	if (ImGui::InputFloat("Sigma S", &_sigmaS))
	{
		glUseProgram(_programID);
		glUniform1f(_glSigmaS, _sigmaS);
	}
	if (ImGui::InputFloat("Sigma R", &_sigmaR))
	{
		glUseProgram(_programID);
		glUniform1f(_glSigmaR, _sigmaR);
	}
}

void BilateralFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void HeatFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Anisotropic.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture))
	{
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}

	_textureSamplerBis = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _frameBufferBis, _textureBis))
	{
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}

	_glBorderDetector = glGetUniformLocation(_programID, "borderDetector");
	_glSigma = glGetUniformLocation(_programID, "sigma");
	_glWidth = glGetUniformLocation(_programID, "width");
	_glHeight = glGetUniformLocation(_programID, "height");

	glUseProgram(_programID);
	glUniform1i(_glBorderDetector, static_cast<int>(_borderDetectorType));
	glUniform1f(_glSigma, _sigma);
	glUniform1f(_glWidth, _width);
	glUniform1f(_glHeight, _height);
}

void HeatFilter::RenderUI()
{

	if (ImGui::InputInt("Heat Ierations", &iterations))
	{
		if (iterations > 0)
		{
			hasChanged = true;
		}
	}

	if (_borderDetectorType != NONE)
	{
		if (ImGui::InputFloat("Border Sigma", &_sigma, 0.01f))
		{
			if (_sigma != 0)
			{
				hasChanged = true;
			}
		}
	}

	if (ImGui::Button("Isotropic"))
	{
		_borderDetectorType = NONE;
		hasChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Leclerc"))
	{
		_borderDetectorType = LECLERC;
		hasChanged = true;
	}

	ImGui::SameLine();

	if (ImGui::Button("Lorentziano"))
	{
		_borderDetectorType = LORENTZIANO;
		hasChanged = true;
	}

	if (_borderDetectorType == NONE)
	{
		ImGui::Text("No border detector");
	}

	if (_borderDetectorType == LECLERC)
	{
		ImGui::Text("Using Leclerc");
	}

	if (_borderDetectorType == LORENTZIANO)
	{
		ImGui::Text("Using Lorentziano");
	}

	if (hasChanged)
	{
		glUseProgram(_programID);
		glUniform1i(_glBorderDetector, static_cast<int>(_borderDetectorType));
		glUniform1f(_glSigma, _sigma);
		glUniform1f(_glWidth, _width);
		glUniform1f(_glHeight, _height);
	}
}

void HeatFilter::ApplyFilter(GLuint prevTexture)
{
	if (hasChanged)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, _outputFramebuffer);
		DrawTexturedTriangles(prevTexture, _textureSampler);

		for (int i = 1; i < iterations; ++i)
		{
			prevTexture = _outputTexture;
			_outputTexture = _textureBis;
			_textureBis = prevTexture;

			auto newBuffer = _frameBufferBis;
			_frameBufferBis = _outputFramebuffer;
			_outputFramebuffer = newBuffer;

			auto newSample = _textureSamplerBis;
			_textureSamplerBis = _textureSampler;
			_textureSampler = newSample;

			glBindFramebuffer(GL_FRAMEBUFFER, newBuffer);
			DrawTexturedTriangles(prevTexture, newSample);
		}

		hasChanged = false;
	}
}

void SusanFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Susan.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture))
	{
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	glUseProgram(_programID);
	_glEpsilon = glGetUniformLocation(_programID, "epsilon");
	_glEdgeDetectionCutoff = glGetUniformLocation(_programID, "edgeDetectionCutoff");
	GLuint glHeight = glGetUniformLocation(_programID, "height");
	GLuint glWidth = glGetUniformLocation(_programID, "width");
	glUniform1f(_glEpsilon, _epsilon);
	glUniform1f(_glEdgeDetectionCutoff, _edgeDetectionCutoff);
	glUniform1f(glHeight, _height);
	glUniform1f(glWidth, _width);
}

void SusanFilter::RenderUI()
{
	if(ImGui::InputFloat("Epsilon", &_epsilon, 0.001f, 0.01f, 3)) {
		glUseProgram(_programID);
		glUniform1f(_glEpsilon, _epsilon);
	}
	if(ImGui::InputFloat("Edge detection cutoff", &_edgeDetectionCutoff, 0.001f, 0.01f, 3)) {
		glUseProgram(_programID);
		glUniform1f(_glEdgeDetectionCutoff, _edgeDetectionCutoff);
	}
}

void SusanFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);
}

void CannyFilter::InitShader()
{
	_programID = LoadShaders("./src/shaders/Passthrough.vert", "./src/shaders/Canny.frag");
	_textureSampler = glGetUniformLocation(_programID, "myTextureSampler");
	if (!InitOutputTexture(_width, _height, _outputFramebuffer, _outputTexture))
	{
		std::cout << "Error rendering to texture." << std::endl;
		return;
	}
	glUseProgram(_programID);
	GLuint glHeight = glGetUniformLocation(_programID, "height");
	GLuint glWidth = glGetUniformLocation(_programID, "width");
	_glT1 = glGetUniformLocation(_programID, "t1");
	_glT2 = glGetUniformLocation(_programID, "t2");
	glUniform1f(glHeight, _height);
	glUniform1f(glWidth, _width);
}

void CannyFilter::RenderUI()
{
	if (ImGui::InputFloat("T1", &_t1, 0.01f, 0.001f, 3)) {
		glUseProgram(_programID);
		glUniform1f(_glT1, _t1);
	}
	if (ImGui::InputFloat("T2", &_t2, 0.01f, 0.001f, 3)) {
		glUseProgram(_programID);
		glUniform1f(_glT2, _t2);
	}
}

void CannyFilter::ApplyFilter(GLuint prevTexture)
{
	DrawTexturedTriangles(prevTexture, _textureSampler);	
}