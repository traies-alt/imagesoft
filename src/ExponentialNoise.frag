#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform usampler2D randomSampler;
uniform float lambda;
uniform float contamination;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb;
	vec3 randomP = texture(randomSampler, UV).rgb / float(32767);
	
	if (randomP.r < contamination) {
		color = pixel * -log(randomP) / lambda; 
	} else {
		color = pixel;
	}
}