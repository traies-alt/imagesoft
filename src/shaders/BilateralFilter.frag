#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float width;
uniform float height;
uniform float sigmaS;
uniform float sigmaR;
uniform int maskSize;

vec3 w(float i, float j, float k, float l, vec3 pixelOrig, vec3 pixelNeighbor) {
	return exp(
			- (
					( vec3((i-k)*(i-k) + (j-l)*(j-l)) ) / ( 2 * sigmaS * sigmaS)
					-
					( (pixelOrig - pixelNeighbor) * (pixelOrig - pixelNeighbor) ) / (2 * sigmaR * sigmaR) 
				)
			);
}

void main() {
	vec3 pixelData = texture(myTextureSampler, UV).rgb;
	float x = floor(UV.x * width);
	float y = floor(UV.y * height);
	

	vec3 acumSup = vec3(0);
	vec3 acumInf = vec3(0);
	float halfMaskSize =  maskSize / 2;
	float endWidth =  min(width, x + halfMaskSize);
	float endHeight = min(height, y + halfMaskSize);
	float startWidth = max(0, x - halfMaskSize);
	float startHeight = max(0, y - halfMaskSize);

	for (float i = startWidth; i <= endWidth; i++) {
		for (float j = startHeight; j <= endHeight; j++) {
			float neighborX = i / width;
			float neighborY = j / height;

			vec3 neighbor = texture(myTextureSampler, vec2(neighborX, neighborY)).rgb;
			vec3 weight = w(x,y, i, j, pixelData, neighbor);
			acumSup += neighbor * weight;
			acumInf += weight;
		}
	}

	color = acumSup / acumInf;
}