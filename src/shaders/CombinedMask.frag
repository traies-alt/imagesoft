#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D maskWeights1;
uniform sampler2D maskWeights2;
uniform float maskSize1;
uniform float maskSize2;
uniform float width;
uniform float height;
uniform float maskDivision;

void main() {
	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	vec3 acum1 = vec3(0);
	float halfMaskSize =  floor(maskSize1 / 2);
	float endWidth =  min(width, x + halfMaskSize);
	float endHeight = min(height, y + halfMaskSize);
	float startWidth = max(0, x - halfMaskSize);
	float startHeight = max(0, y - halfMaskSize);

	for (float i = startWidth; i <= endWidth; i++) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			float weightX = (i - x) / maskSize1 + 0.5F;
			float weightY = (j - y) / maskSize1 + 0.5F;

			vec3 neighbour = texture(myTextureSampler, vec2(normalX, normalY)).rgb;
			float weight = floor(texture(maskWeights1, vec2(weightX, weightY)).r);
			acum1 += neighbour * weight;
		}
	}

	vec3 acum2 = vec3(0);
	halfMaskSize =  floor(maskSize2 / 2);
	endWidth =  min(width, x + halfMaskSize);
	endHeight = min(height, y + halfMaskSize);
	startWidth = max(0, x - halfMaskSize);
	startHeight = max(0, y - halfMaskSize);
	for (float i = startWidth; i <= endWidth; i++) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			float weightX = (i - x) / maskSize2 + 0.5F;
			float weightY = (j - y) / maskSize2 + 0.5F;

			vec3 neighbour = texture(myTextureSampler, vec2(normalX, normalY)).rgb;
			float weight = floor(texture(maskWeights2, vec2(weightX, weightY)).r);
			acum2 += neighbour * weight;
		}
	}
	color = abs(acum1 - acum2) / maskDivision;
}