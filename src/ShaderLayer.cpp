#include "ShaderLayer.h"
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

    return ProgramID;
}

GLuint RandomTexture(size_t seed, int width, int height, GLuint texture)
{
	
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	srand(seed);
	unsigned int  * pixels = new unsigned int[width * height * 4];
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < 4; k++)
				pixels[(i + j * width) * 4 + k] = rand();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] pixels;
	return texture;
}

GLuint WeightedTexture(int maskSize, float * weights, GLuint texture)
{
	return WeightedTexture2D(maskSize, maskSize, weights, texture);
}

GLuint WeightedTexture2D(int width, int height, float * weights, GLuint texture)
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, weights);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

bool InitOutputTexture(int w, int h, GLuint& oFrameBuffer, GLuint& oTexture)
{
	// The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
	glGenFramebuffers(1, &oFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, oFrameBuffer);

	// The texture we're going to render to
	glGenTextures(1, &oTexture);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, oTexture);

	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, w, h, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, oTexture, 0);

	// Set the list of draw buffers.
	GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers
	return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void GetMinMaxRGB(GLuint texture, int w, int h, unsigned char& minr, unsigned char& ming, unsigned char& minb, unsigned char& maxr, unsigned char& maxg, unsigned char& maxb)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char * pixels = new unsigned char[w * h * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	minr = 255; maxr = 0;
	ming = 255; maxg = 0;
	minb = 255; maxb = 0;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++) {
			unsigned char r = pixels[(i + j * w) * 3];
			minr = r < minr ? r : minr;
			maxr = r > maxr ? r : maxr;
			unsigned char g = pixels[(i + j * w) * 3 + 1];
			ming = g < ming ? g : ming;
			maxg = g > maxg ? g : maxg;
			unsigned char b = pixels[(i + j * w) * 3 + 2];
			minb = b < minb ? b : minb;
			maxb = b > maxb ? b : maxb;
		}

	delete[] pixels;
};

int GetHistogram(GLuint texture, int w, int h, int band, float * hist)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char * pixels = new unsigned char[w * h];
	
	GLuint format;
	switch(band) {
		case 0:
			format = GL_RED;
			break;
		case 1:
			format = GL_GREEN;
			break;
		case 2:
			format = GL_BLUE;
			break;
		default:
			format = GL_LUMINANCE;
			break;
	}
	glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, pixels);
	float max = 0;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++) {
			if (++hist[pixels[i + j * w]] > max ) {
				max = hist[pixels[i + j * w]];
			}
		}
	delete[] pixels;
	return max;
}

void GetHistogramAll(GLuint texture, int w, int h, float * hr, float * hg, float * hb)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char * pixels = new unsigned char[w * h * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++) {
			++hr[pixels[(i + j * w) * 3]];
			++hg[pixels[(i + j * w) * 3 + 1]];
			++hb[pixels[(i + j * w) * 3 + 2]];
		}
	delete[] pixels;
	return;
}


void GetMinMaxForSum(GLuint texture1, GLuint texture2,  int w, int h, float * min, float * max)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);
	unsigned char * pixels1 = new unsigned char[w * h * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels1);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	unsigned char * pixels2 = new unsigned char[w * h * 3];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels2);
	min[0] = 512; max[0] = -255;
	min[1] = 512; max[1] = -255;
	min[2] = 512; max[2] = -255;
	for (int i = 0; i < w; i++)
		for (int j = 0; j < h; j++) {
			for (int k = 0; k < 3; k++) {
				unsigned char r = pixels1[(i + j * w) * 3 + k] + pixels2[(i + j * w) * 3 + k];
				min[k] = r < min[k] ? r : min[k];
				max[k] = r > max[k] ? r : max[k];
			}
		}

	min[0] /= 255; max[0] /= 255;
	min[1] /= 255; max[1] /= 255;
	min[2] /= 255; max[2] /= 255;

	delete[] pixels1;
	delete[] pixels2;
}
