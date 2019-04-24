#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float width;

void main() {
	float x = floor(UV.x * width);
	vec3 prev = texture(myTextureSampler, vec2(float(x - 1) / width, UV.y)).rgb * 8 - vec3(4);
	vec3 curr = texture(myTextureSampler, UV).rgb * 8 - vec3(4);
	vec3 next = texture(myTextureSampler, vec2(float(x + 1) / width, UV.y)).rgb * 8 - vec3(4);
	
	if (x == 0) {
		color = vec3(0);
	} else if(curr== vec3(0) && x < width - 1) {
		color.r = prev.r * next.r > 0 ? 1 : 0;
		color.g = prev.g * next.g > 0 ? 1 : 0;
		color.b = prev.b * next.b > 0 ? 1 : 0;
	} else {
		color.r = prev.r * curr.r > 0 ? 1 : 0;
		color.g = prev.g * curr.g > 0 ? 1 : 0;
		color.b = prev.b * curr.b > 0 ? 1 : 0;
	}
	
	// color = texture(myTextureSampler, UV).rgb;
}