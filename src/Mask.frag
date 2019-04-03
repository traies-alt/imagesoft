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
	int x = int(UV.x * width);
	int y = int(UV.y * height);

	vec3 acum = vec3(0);
	for (int i = max(0, x - int(maskSize) / 2); i <= min(width-1, x + int(maskSize) / 2); i++ ) {
		for (int j = max(0, y - int(maskSize) / 2); j <= min(height-1, y + int(maskSize) / 2); j++) {
			float normalX = float(i) / width;
			float normalY = float(j) / height;
			
			vec3 neighbour = texture(myTextureSampler, vec2( normalX, normalY)).rgb;
			float weight = texture(maskWeights, vec2(normalX, normalY)).r;
			acum += neighbour * weight;
		}
	}
	color = acum / maskDivision;
}