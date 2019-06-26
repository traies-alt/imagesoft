#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler1;
uniform sampler2D myTextureSampler2;

void main() {
	color = texture(myTextureSampler1, UV).rgb * texture(myTextureSampler2, UV).rgb;
}