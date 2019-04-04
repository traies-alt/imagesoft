#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform vec3 threshold;

void main() {
	vec3 pixelData = texture( myTextureSampler, UV ).rgb;

	if (pixelData.r < threshold.r && pixelData.g < threshold.g && pixelData.b < threshold.b) {
		color = vec3(1, 1, 1);
	} else {
		color = vec3(0, 0, 0);
	}
}