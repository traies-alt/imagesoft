#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform vec3 threshold;

void main() {
	vec3 pixelData = texture( myTextureSampler, UV ).rgb;

	color.r = pixelData.r > threshold.r ? 1 : 0;
	color.g = pixelData.g > threshold.g ? 1 : 0;
	color.b = pixelData.b > threshold.b ? 1 : 0;
}