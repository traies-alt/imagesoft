#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform int band;

void main() {
	vec3 pixelData = texture( myTextureSampler, UV ).rgb;

	if (band == 0) {
		float mean = (pixelData.r + pixelData.g + pixelData.b) / 3.0f;
		color = vec3(mean, mean, mean);
	}
	if (band == 1) {
		color = vec3(pixelData.r, 0, 0);
	}
	if (band == 2) {
		color = vec3(0, pixelData.g, 0);
	}
	if (band == 3) {
		color = vec3(0 , 0, pixelData.b);
	}
}