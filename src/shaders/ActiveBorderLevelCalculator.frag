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
uniform float prec;
uniform vec3 omega;
uniform bool shouldMove;
uniform bool cleaning;


bool is_lout_compressing(float x, float y);
bool is_lout_expanding(float x, float y);
bool is_lin_compressing(float x, float y);
bool is_lin_expanding(float x, float y);

float P_in(vec3 pixel) {
	return 1 - distance(omega, pixel);
}

float P_out(vec3 pixel) {
	return distance(omega, pixel);
}

float Fd(vec3 pixel) {
	return log(P_in(pixel) / P_out(pixel));
}

bool is_sorrounded(float x, float y, float value) {
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	bool existsN = true;
	existsN = existsN && (abs(texture(levelValueSampler, pos1).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos2).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos3).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos4).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos5).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos6).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos7).r - value) < 0.01);
	existsN = existsN && (abs(texture(levelValueSampler, pos8).r - value) < 0.01);

	return existsN;
}

bool is_near(float x, float y, float value) {
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	bool existsN = false;
	int count = 0;
	existsN = existsN || (abs(texture(levelValueSampler, pos1).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos2).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos3).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos4).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos5).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos6).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos7).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos8).r - value) < 0.01);

	return existsN;
}

bool is_near2(float x, float y, float value) {
	vec2 pos1 = vec2((x + 2)/width, y/height);
	vec2 pos2 = vec2((x - 2)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 2)/height);
	vec2 pos4 = vec2(x/width, (y - 2)/height);
	vec2 pos5 = vec2((x + 2)/width, (y + 2)/height);
	vec2 pos6 = vec2((x - 2)/width, (y - 2)/height);
	vec2 pos7 = vec2((x - 2)/width, (y + 2)/height);
	vec2 pos8 = vec2((x + 2)/width, (y - 2)/height);


	bool existsN = false;
	int count = 0;
	existsN = existsN || (abs(texture(levelValueSampler, pos1).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos2).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos3).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos4).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos5).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos6).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos7).r - value) < 0.01);
	existsN = existsN || (abs(texture(levelValueSampler, pos8).r - value) < 0.01);

	return existsN;
}

bool is_prob_inside(vec3 pixel) {
	return distance(omega, pixel) < prec;
}

bool is_lout_compressing(float x, float y) {
	//is ALL outside transformin in lout
	//IF has an outside turning into lout, this is lin

	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);

	bool existsN = true;
	existsN = existsN && (!(abs(texture(levelValueSampler, pos1).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos1).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos2).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos2).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos3).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos3).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos4).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos4).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos5).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos5).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos6).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos6).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos7).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos7).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos8).r - outside) < 0.01) || is_prob_inside(texture(myTextureSampler, pos8).rgb));

	return existsN;
}

bool is_lout_expanding(float x, float y) {
	//is All lin transformin in lout
	//IF has an lin turning into lout, this is outside
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	bool existsN = true;
	existsN = existsN && (!(abs(texture(levelValueSampler, pos1).r - lin) < 0.01) || is_lin_compressing(pos1.x, pos1.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos2).r - lin) < 0.01) || is_lin_compressing(pos2.x, pos2.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos3).r - lin) < 0.01) || is_lin_compressing(pos3.x, pos3.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos4).r - lin) < 0.01) || is_lin_compressing(pos4.x, pos4.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos5).r - lin) < 0.01) || is_lin_compressing(pos5.x, pos5.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos6).r - lin) < 0.01) || is_lin_compressing(pos6.x, pos6.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos7).r - lin) < 0.01) || is_lin_compressing(pos7.x, pos7.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos8).r - lin) < 0.01) || is_lin_compressing(pos8.x, pos8.y));

	return existsN;
}

bool is_lin_expanding(float x, float y) {
	//is all lout transformin in lin
	//IF has an lout turning into lin, this is inside
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	bool existsN = true;
	existsN = existsN && (!(abs(texture(levelValueSampler, pos1).r - lout) < 0.01) || is_lout_compressing(pos1.x, pos1.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos2).r - lout) < 0.01) || is_lout_compressing(pos2.x, pos2.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos3).r - lout) < 0.01) || is_lout_compressing(pos3.x, pos3.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos4).r - lout) < 0.01) || is_lout_compressing(pos4.x, pos4.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos5).r - lout) < 0.01) || is_lout_compressing(pos5.x, pos5.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos6).r - lout) < 0.01) || is_lout_compressing(pos6.x, pos6.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos7).r - lout) < 0.01) || is_lout_compressing(pos7.x, pos7.y));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos8).r - lout) < 0.01) || is_lout_compressing(pos8.x, pos8.y));

	return existsN;
}


bool is_lin_compressing(float x, float y) {
	//is an inside transforming in lin
	//IF has an inside turning into lin, this is lout
	vec2 pos1 = vec2((x + 1)/width, y/height);
	vec2 pos2 = vec2((x - 1)/width, y/height);
	vec2 pos3 = vec2(x/width, (y + 1)/height);
	vec2 pos4 = vec2(x/width, (y - 1)/height);
	vec2 pos5 = vec2((x + 1)/width, (y + 1)/height);
	vec2 pos6 = vec2((x - 1)/width, (y - 1)/height);
	vec2 pos7 = vec2((x - 1)/width, (y + 1)/height);
	vec2 pos8 = vec2((x + 1)/width, (y - 1)/height);


	bool existsN = true;
	existsN = existsN && (!(abs(texture(levelValueSampler, pos1).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos1).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos2).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos2).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos3).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos3).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos4).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos4).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos5).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos5).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos6).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos6).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos7).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos7).rgb));
	existsN = existsN && (!(abs(texture(levelValueSampler, pos8).r - inside) < 0.01) || !is_prob_inside(texture(myTextureSampler, pos8).rgb));

	return existsN;
}



void main() {

	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	float value = texture(levelValueSampler, UV).r;
	vec3 pixel = texture(myTextureSampler, UV).rgb;

	color = vec3(value, 0, 0);

	if(cleaning) {
		if (x>= width || x<=0 || y>=height || y<=0) {
			color = vec3(outside, 0, 0);
		} else if(is_sorrounded(x,y,inside)) {
			color = vec3(inside, 0, 0);
		} else if (abs(value - outside) < 0.01) {
			if (is_near(x, y, lin)) {
				color = vec3(lout, 0, 0);
			} else if(is_near(x,y, inside)) {
				color = vec3(lin, 0, 0);
			}
		} else if (abs(value - inside) < 0.01) {
//			if(!is_near(x,y, lin)) {
//				color = vec3(outside, 0, 0);
//			}
		} else if (abs(value - lout) < 0.01) {
			if(is_near(x,y, inside)) {
			  		color = vec3(inside, 0, 0);
			} else if(!is_near(x,y, lin)) {
				color = vec3(outside, 0, 0);
			}
//			else if(!is_near(x,y, outside)) {
//				if(is_near(x,y,inside)) {
//					color = vec3(inside, 0, 0);
//				} else {
//					color = vec3(outside, 0, 0);
//				}
//			}

		} else if (abs(value - lin) < 0.01) {
      		if(!is_near(x,y, inside)) {
				if(is_near(x,y,lout)) {
					color = vec3(outside, 0, 0);
				} else {
					color = vec3(inside, 0, 0);
				}
			}
		}
	} else {
	if(shouldMove) {
		if (x>= width || x<=0 || y>=height || y<=0) {
			color = vec3(outside, 0, 0);
		} else if (abs(value - outside) < 0.01) {
			//OUTSIDE SHOULD BE LOUT
			if(is_near(x,y, lout) && is_prob_inside(pixel)){
				color = vec3(lout, 0, 0);
			}
		} else if (abs(value - inside) < 0.01) {
			//INSIDE SHOULD BE LIN
			if(is_near(x,y, lin) && !is_prob_inside(pixel)){
				color = vec3(lin, 0, 0);
			}
		} else if (abs(value - lout) < 0.01) {
			if(is_lout_compressing(x,y)) {
				color = vec3(lin, 0, 0);					//IF has an outside turning into lout, this is lin
			} else if(is_lout_expanding(x,y)){
				color = vec3(outside, 0, 0);				//IF has an lin turning into lout, this is outside
			}
		} else if (abs(value - lin) < 0.01) {
			if(is_lin_compressing(x,y)) {
				color = vec3(lout, 0, 0);					//IF has an inside turning into lin, this is lout
			} else if(is_lin_expanding(x,y)){
				color = vec3(inside, 0, 0);					//IF has an lout turning into lin, this is inside
			}
		}
	}
	}
}