#version 330 core

#define M_PI 3.1415926535897932384626433832795

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D randomSampler;
uniform sampler2D randomSampler2;
uniform float sigma;
uniform float mu;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 randomP = texture(randomSampler, UV).rgb;
	vec3 randomP2 = texture(randomSampler2, UV).rgb;

	vec3 y1 = sqrt(-2*log(randomP)) * cos(2 * M_PI * randomP2);
	vec3 y2 = sqrt(-2*log(randomP)) * sin(2 * M_PI * randomP2);
	
	float fact = 1 / sqrt(2 * M_PI);
	color = 0.5 + fact * exp(-y1 * y1 / 2) * exp(-y2 * y2 / 2); 
}