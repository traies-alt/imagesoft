#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float factor;
uniform vec3 c;
uniform int dynamicRange;

void main() {
	if (dynamicRange == 0) {
		color = factor * texture( myTextureSampler, UV ).rgb;
	} else {
		color = c * log(vec3(1, 1, 1) + factor * texture( myTextureSampler, UV ).rgb);
	}
}