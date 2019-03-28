#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float factor;

void main() {
	color = factor * texture( myTextureSampler, UV ).rgb;
}