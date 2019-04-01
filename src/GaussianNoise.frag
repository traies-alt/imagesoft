#version 330 core

#define M_PI 3.1415926535897932384626433832795

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform usampler2D randomSampler;
uniform usampler2D randomSampler2;
uniform float sigma;
uniform float mu;
uniform float contamination;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 x1 = texture(randomSampler, UV).rgb  / float(32767);
	vec3 x2 = texture(randomSampler2, UV).rgb  / float(32767);

	if (x1.r < contamination) {
		vec3 y1 = sqrt(-2*log(x1)) * cos(2 * M_PI * x2);
		// vec3 y2 = sqrt(-2*log(x1)) * sin(2 * 3.14 * x2);
		color = pixel + sqrt(sigma) * y1 + mu;
	} else {
		color = pixel;
	}
}