#version 330 core

#define inside 0
#define lin 0.3
#define lout 0.6
#define outside 1

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform sampler2D levelValueSampler;
uniform float width;
uniform float height;
uniform vec3 omega;
uniform bool shouldMove;

float P_in(vec3 pixel) {
	return 1 - distance(omega, pixel);
}

float P_out(vec3 pixel) {
	return distance(omega, pixel);
}

float Fd(vec3 pixel) {
	return log(P_in(pixel) / P_out(pixel));
}

bool should_expand(float x, float y) {

	bool existsN = false;
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);

	if (pos1.x>= width || pos1.x<=0 || pos1.y>=height || pos1.y<=0) return false;
	if (pos2.x>= width || pos2.x<=0 || pos2.y>=height || pos2.y<=0) return false;
	if (pos3.x>= width || pos3.x<=0 || pos3.y>=height || pos3.y<=0) return false;
	if (pos4.x>= width || pos4.x<=0 || pos4.y>=height || pos4.y<=0) return false;
	if (pos5.x>= width || pos5.x<=0 || pos5.y>=height || pos5.y<=0) return false;
	if (pos6.x>= width || pos6.x<=0 || pos6.y>=height || pos6.y<=0) return false;
	if (pos7.x>= width || pos7.x<=0 || pos7.y>=height || pos7.y<=0) return false;
	if (pos8.x>= width || pos8.x<=0 || pos8.y>=height || pos8.y<=0) return false;

	existsN = existsN || ((abs(texture(levelValueSampler, pos1).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos1).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos2).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos2).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos3).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos3).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos4).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos4).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos5).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos5).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos6).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos6).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos7).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos7).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos8).r - lout) < 0.01) && (Fd(texture(myTextureSampler, pos8).rgb) > 0));

	return existsN;
}

bool should_contract(float x, float y) {

	bool existsN = false;

	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	if (pos1.x>= width || pos1.x<=0 || pos1.y>=height || pos1.y<=0) return false;
	if (pos2.x>= width || pos2.x<=0 || pos2.y>=height || pos2.y<=0) return false;
	if (pos3.x>= width || pos3.x<=0 || pos3.y>=height || pos3.y<=0) return false;
	if (pos4.x>= width || pos4.x<=0 || pos4.y>=height || pos4.y<=0) return false;
	if (pos5.x>= width || pos5.x<=0 || pos5.y>=height || pos5.y<=0) return false;
	if (pos6.x>= width || pos6.x<=0 || pos6.y>=height || pos6.y<=0) return false;
	if (pos7.x>= width || pos7.x<=0 || pos7.y>=height || pos7.y<=0) return false;
	if (pos8.x>= width || pos8.x<=0 || pos8.y>=height || pos8.y<=0) return false;

	existsN = existsN || ((abs(texture(levelValueSampler, pos1).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos1).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos2).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos2).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos3).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos3).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos4).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos4).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos5).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos5).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos6).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos6).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos7).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos7).rgb) > 0));
	existsN = existsN || ((abs(texture(levelValueSampler, pos8).r - lin) < 0.01) && (Fd(texture(myTextureSampler, pos8).rgb) > 0));

	return existsN;
}

bool is_lout(float value, float x, float y) {
	if(value <= lin + 0.01) {
		return false;
	}

	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);

	bool existsN = false;
	existsN = existsN || (texture(levelValueSampler, vec2((x + 1)/width, y/height)).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2((x - 1)/width, y/height)).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2(x/width, (y + 1)/height)).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2(x/width, (y - 1)/height)).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, pos5).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, pos6).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, pos7).r <= lin + 0.01);
	existsN = existsN || (texture(levelValueSampler, pos8).r <= lin + 0.01);

	return existsN;
}

bool is_lin(float value, float x, float y) {
	if(value >= lout - 0.01) {
		return false;
	}

	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);

	bool existsN = false;
	existsN = existsN || (texture(levelValueSampler, vec2((x + 1)/width, y/height)).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2((x - 1)/width, y/height)).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2(x/width, (y + 1)/height)).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, vec2(x/width, (y - 1)/height)).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, pos5).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, pos6).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, pos7).r >= lout - 0.01);
	existsN = existsN || (texture(levelValueSampler, pos8).r >= lout - 0.01);


	return existsN;
}

void main() {

	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	float value = texture(levelValueSampler, UV).r;
	vec3 pixel = texture(myTextureSampler, UV).rgb;

	if(shouldMove) {
		if (x>= width || x<=0 || y>=height || y<=0) {
			color = vec3(outside, 0, 0);
		} else if (abs(value - outside) < 0.01 && should_expand(x, y)) {
			color = vec3(lout, 0, 0);
		} else if (abs(value - inside) < 0.01 && should_contract(x, y)) {
			color = vec3(lin, 0, 0);
		} else if (abs(value - lout) < 0.01 && !is_lout(value, x, y)) {
			color = vec3(outside, 0, 0);
		} else if (abs(value - lin) < 0.01 && !is_lin(value, x, y)) {
			color = vec3(inside, 0, 0);
		}
	}else {
		color = vec3(value, 0, 0);
	}
}