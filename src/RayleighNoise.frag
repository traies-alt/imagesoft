#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D randomSampler;
uniform float xi;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 randomP = texture(randomSampler, UV).rgb;

	color = pixel * xi * sqrt(-2*log(1-randomP)); 
}