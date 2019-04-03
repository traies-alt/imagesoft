#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D maskWeights;
uniform float maskSize;
uniform float width;
uniform float height;
uniform float maskDivision;

#define sort(arr, size)		for (int i = 0; i < size; i++) {float m = arr[i];int minIndex= i;for (int j = i + 1; j < size; j++) {if (arr[j] < m) {m = arr[j];minIndex = j;}}arr[minIndex] = arr[i];arr[i] = m;}

float redArray[16]; 
float greenArray[16]; 
float blueArray[16]; 

void main() {
	float x = UV.x * width;
	float y = UV.y * height;

	vec3 acum = vec3(0);
	
	int count = 0;
	float minWidth =  min(width-1, x + maskSize / 2);
	float minHeight = min(height-1, y + maskSize / 2);
	for (float i = max(0, x - maskSize / 2); i <= minWidth; i++ ) {
		for (float j = max(0, y - maskSize / 2); j <= minHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			
			vec3 neighbour = texture(myTextureSampler, vec2( normalX, normalY)).rgb;
			redArray[count] = neighbour.r;
			greenArray[count] = neighbour.g;
			blueArray[count] = neighbour.b;
			count++;
		}
	}
	int arraySize = count;
	sort(redArray, arraySize);
	sort(blueArray, arraySize);
	sort(greenArray, arraySize);
	color.r = redArray[arraySize / 2];
	color.g = greenArray[arraySize / 2];
	color.b = blueArray[arraySize / 2];
	// color = vec3(0,0,0);
}