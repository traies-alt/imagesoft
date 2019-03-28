#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler1D eqSampler;

void main() {
	vec3 pixelData = texture( myTextureSampler, UV ).rgb;

	color.r = texture(eqSampler, pixelData.r).r;
	color.g = texture(eqSampler, pixelData.g).g;
	color.b = texture(eqSampler, pixelData.b).b;
}