#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float epsilon;
uniform float edgeDetectionCutoff;
uniform float width;
uniform float height;

void main() {
	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	vec3 centerColor = texture(myTextureSampler, UV).rgb;
	float center = (centerColor.r + centerColor.g + centerColor.y) / 3.0;
	float acum = 0;
	float total = 0;
	float endWidth =  min(width, x + 3);
	float endHeight = min(height, y + 3);
	float startWidth = max(0, x - 3);
	float startHeight = max(0, y - 3);
	for (float i = startWidth; i <= endWidth; i++ ) {
		for (float j = startHeight; j <= endHeight; j++) {
			if (abs(i - x) + abs(j - y) <= 4) {
				float normalX = i / width;
				float normalY = j / height;

				vec3 neighbour = texture(myTextureSampler, vec2( normalX, normalY)).rgb;
				float grey = (neighbour.r + neighbour.g + neighbour.y) / 3.0;
				if (abs(grey - center) <= epsilon) {
					acum++;
				}
				total++;
			}
		}
	}
	float s = 1.0 - acum / total;
	if (abs(s - 0.75) <= edgeDetectionCutoff) {
		color = vec3(1, 0, 0);
	} else if (abs(s - 0.5) <= edgeDetectionCutoff) {
		color = vec3(1);
	} else {
		color = vec3(0);
	}
}