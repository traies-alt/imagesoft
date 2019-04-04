#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D maskWeights;
uniform float maskSize;
uniform float width;
uniform float height;
uniform float maskDivision;

float array[80];
float median(int size) {
	for (int i = 0; i < size; i++) {
		float m = array[i];
		int minIndex= i;
		for (int j = i + 1; j < size; j++) {
			if (array[j] < m) {
					m = array[j];
					minIndex = j;
			}
		}
		array[minIndex] = array[i];
		array[i] = m;
	}
	return array[size / 2];
}

float mediancolor(int band, float x, float y, float startWidth, float startHeight, float endWidth, float endHeight)
{
	int count = 0;
	for (float i = startWidth; i <= endWidth; i++ ) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			float weightX = (i - x) / maskSize + 0.5F;
			float weightY = (j - y) / maskSize + 0.5F;
			
			vec3 neighbour = texture(myTextureSampler, vec2( normalX, normalY)).rgb;
			float t = floor(texture(maskWeights, vec2(weightX, weightY)).r);
			for (float k = 0.0; k < t; k++) {
				array[count++] = neighbour[band];
			}
		}
	}
	return median(count);
}

void main() {
	float x = UV.x * width;
	float y = UV.y * height;

	vec3 acum = vec3(0);
	
	float halfMaskSize =  floor(int(maskSize) / 2);
	float endWidth =  min(width, x + halfMaskSize);
	float endHeight = min(height, y + halfMaskSize);
	float startWidth = max(0, x - halfMaskSize);
	float startHeight = max(0, y - halfMaskSize);

	color.r = mediancolor(0, x, y, startWidth, startHeight, endWidth, endHeight);
	color.g = mediancolor(1, x, y, startWidth, startHeight, endWidth, endHeight);
	color.b = mediancolor(2, x, y, startWidth, startHeight, endWidth, endHeight);
}