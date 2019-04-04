#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float gamma;

void main() {
	vec3 pixel = texture( myTextureSampler, UV ).rgb; 
	color.r = pow(pixel.r, gamma);
	color.g = pow(pixel.g, gamma);
	color.b = pow(pixel.b, gamma);
}