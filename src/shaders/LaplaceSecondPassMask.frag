#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float width;
uniform float height;
uniform float threshold;
uniform float maximum;

void main() {
	float x = floor(UV.x * width); 
	vec3 curr = texture(myTextureSampler, UV).rgb * (maximum) - (maximum / 2);
	vec3 prevX = texture(myTextureSampler, vec2(float(x - 1) / width, UV.y)).rgb * 4 - 2;
	vec3 nextX = texture(myTextureSampler, vec2(float(x + 1) / width, UV.y)).rgb * 4 - 2;
	
	color = vec3(1);
	if (x > 0) {
		if (curr.r == 0 && x < width - 1) {
			color.r = prevX.r * nextX.r > 0 ? color.r : (abs(prevX.r + nextX.r) > threshold ? 0: color.r);
		} else {
			color.r = prevX.r * curr.r > 0 ? color.r : (abs(prevX.r + curr.r) > threshold ? 0: color.r);
		}
		if (curr.g == 0 && x < width - 1) {
			color.g = prevX.g * nextX.g > 0 ? color.g : (abs(prevX.g + nextX.g) > threshold ? 0: color.g);
		} else {
			color.g = prevX.g * curr.g > 0 ? color.g : (abs(prevX.g + curr.g) > threshold ? 0: color.g);
		}
		if (curr.b == 0 && x < width - 1) {
			color.b = prevX.b * nextX.b > 0 ? color.b : (abs(prevX.b + nextX.b) > threshold ? 0: color.b);
		} else {
			color.b = prevX.b * curr.b > 0 ? color.b : (abs(prevX.b + curr.b) > threshold ? 0: color.b);
		}
	}

	float y = floor(UV.y * height);
	vec3 prevY = texture(myTextureSampler, vec2(UV.x, float(y - 1) / height)).rgb * 4 - 2;
	vec3 nextY = texture(myTextureSampler, vec2(UV.x, float(y + 1) / height)).rgb * 4 - 2;
	
	if (y > 0) {
		if (curr.r == 0 && y < height - 1) {
			color.r = prevY.r * nextY.r > 0 ? color.r : (abs(prevY.r + nextY.r) > threshold ? 0: color.r);
		} else {
			color.r = prevY.r * curr.r > 0 ? color.r : (abs(prevY.r + curr.r) > threshold ? 0: color.r);
		}
		if (curr.g == 0 && y < height - 1) {
			color.g = prevY.g * nextY.g > 0 ? color.g : (abs(prevY.g + nextY.g) > threshold ? 0: color.g);
		} else {
			color.g = prevY.g * curr.g > 0 ? color.g : (abs(prevY.g + curr.g) > threshold ? 0: color.g);
		}
		if (curr.b == 0 && y < height - 1) {
			color.b = prevY.b * nextY.b > 0 ? color.b : (abs(prevY.b + nextY.b) > threshold ? 0: color.b);
		} else {
			color.b = prevY.b * curr.b > 0 ? color.b : (abs(prevY.b + curr.b) > threshold ? 0: color.b);
		}
	}
}