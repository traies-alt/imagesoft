#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform usampler2D randomSampler;
uniform float xi;
uniform float contamination;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 randomP = texture(randomSampler, UV).rgb / float(32767);

	if (randomP.r < contamination) {
		color = pixel * xi * sqrt(-2.0*log(1.0-randomP));
	} else {
		color = pixel;
	}
}