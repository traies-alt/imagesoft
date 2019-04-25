#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D maskWeights;
uniform float maskSize;
uniform float width;
uniform float height;
uniform float maskDivision;

void main() {
	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	vec3 acum = vec3(0);
	float halfMaskSize =  floor(maskSize / 2);
	float endWidth =  min(width, x + halfMaskSize);
	float endHeight = min(height, y + halfMaskSize);
	float startWidth = max(0, x - halfMaskSize);
	float startHeight = max(0, y - halfMaskSize);
	for (float i = startWidth; i <= endWidth; i++ ) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			float weightX = (i - x) / maskSize + 0.5F;
			float weightY = (j - y) / maskSize + 0.5F;
			
			vec3 neighbour = texture(myTextureSampler, vec2( normalX, normalY)).rgb;
			float weight = texture(maskWeights, vec2(weightX, weightY)).r;
			acum += neighbour * weight;
		}
	}
	color = acum / maskDivision;
}