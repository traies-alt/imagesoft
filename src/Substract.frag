#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;

uniform sampler2D myTextureSampler;
uniform sampler2D secondSampler;
uniform int factor;
void main()
{
	// color = clamp(texture( myTextureSampler, UV ).rgb - texture( secondSampler, UV ).rgb, vec3(0, 0, 0), vec3(1, 1, 1))
	color = texture(myTextureSampler, UV).rgb + factor * texture(secondSampler, UV).rgb;
}