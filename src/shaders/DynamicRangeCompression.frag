#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform vec3 c;

void main() {
	color = c * log(vec3(1, 1, 1) + texture( myTextureSampler, UV ).rgb);
}