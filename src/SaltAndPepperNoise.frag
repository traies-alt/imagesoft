#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform usampler2D randomSampler;
uniform float contamination1;
uniform float contamination2;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 randomP = texture(randomSampler, UV).rgb / float(32767);
	if (randomP.r < contamination1) {
		color = vec3(0, 0, 0);
	} else if (randomP.r > contamination2) {
		color = vec3(1, 1, 1);
	} else {
		color = pixel;
	}
}