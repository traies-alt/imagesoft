#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform vec3 lower;
uniform vec3 lowerTo;
uniform vec3 higher;
uniform vec3 higherTo;

float mapTo(float value, float lower, float lowerTo, float higher, float higherTo)
{
	if (value < lower) {
		return value / lower * lowerTo;
	} else if (value < higher) {
		return lowerTo + (value - lower) / (higher - lower) * higherTo;
	} else {
		return higherTo + (value - higher) / (1 - higher) * (1 - higherTo);
	}
}

void main() {
	vec3 pixelData = texture( myTextureSampler, UV ).rgb;

	color.r = mapTo(pixelData.r, lower.r, lowerTo.r, higher.r, higherTo.r);
	color.g = mapTo(pixelData.g, lower.g, lowerTo.g, higher.g, higherTo.g);
	color.b = mapTo(pixelData.b, lower.b, lowerTo.b, higher.b, higherTo.b);
}