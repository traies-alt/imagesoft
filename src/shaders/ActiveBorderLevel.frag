#version 330 core

#define inside 0
#define lin 0.3
#define lout 0.6
#define outside 1

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D levelValueSampler;

void main() {
	float value = texture(levelValueSampler, UV).r;
	vec3 pixel = texture(myTextureSampler, UV).rgb;

	if(abs(value - outside) < 0.01) {
		color = pixel;
	} else if(abs(value - inside) < 0.01) {
		color = pixel;
	} else if(abs(value - lout) < 0.01) {
		color = vec3(1, 0, 0);
	} else if(abs(value - lin) < 0.01) {
		color = vec3(0, 1, 0);
	}
}