#version 330 core
#define M_PI 3.1415926535897932384626433832795
#define M_45 0.7853981634;

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform float width;
uniform float height;
uniform float t1;
uniform float t2;

float hMask[9] = float[9](
	-1, -2, -1,
	 0,  0,  0,
	 1,  2,  1
);

float vMask[9] = float[9](
	-1, 0, 1,
	-2, 0, 2,
	-1, 0, 1
);

vec2 calcMagnitudeAngle(float x, float y, float width, float height)
{
	float acum1 = 0;
	float endWidth =  min(width, x + 1);
	float endHeight = min(height, y + 1);
	float startWidth = max(0, x - 1);
	float startHeight = max(0, y - 1);

	for (float i = startWidth; i <= endWidth; i++) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			int maskX = int(i - x + 1);
			int maskY = int(j - y + 1);

			vec3 neighbour = texture(myTextureSampler, vec2(normalX, normalY)).rgb;
			float weight = hMask[maskX + maskY * 3];
			float grey = (neighbour.r + neighbour.g + neighbour.b) / 3.0;
			acum1 += grey * weight;
		}
	}

	float acum2 = 0;
	endWidth =  min(width, x + 1);
	endHeight = min(height, y + 1);
	startWidth = max(0, x - 1);
	startHeight = max(0, y - 1);
	for (float i = startWidth; i <= endWidth; i++) {
		for (float j = startHeight; j <= endHeight; j++) {
			float normalX = i / width;
			float normalY = j / height;
			int maskX = int(i - x + 1);
			int maskY = int(j - y + 1);

			vec3 neighbour = texture(myTextureSampler, vec2(normalX, normalY)).rgb;
			float weight = vMask[maskX + maskY * 3];
			float grey = (neighbour.r + neighbour.g + neighbour.b) / 3.0;
			acum2 += grey * weight;
		}
	}
	return vec2(acum1, acum2);
}

void main() {
	float x = floor(UV.x * width);
	float y = floor(UV.y * height);

	vec2 mag = calcMagnitudeAngle(x, y, width, height);
	float angle = 0;
	if (mag.x > 0) {
		angle = 180 * (atan(mag.y, mag.x) / M_PI);
	}
	vec2 mag1 = vec2(0);
	vec2 mag2 = vec2(0);
	if (angle <= 22.5 || angle > 157.5) {
		// HORIZONTAL
		mag1 = calcMagnitudeAngle(x + 1, y, width, height);
		mag2 = calcMagnitudeAngle(x - 1, y, width, height);
	} else if (angle > 22.5 && angle <= 67.5) {
		mag1 = calcMagnitudeAngle(x + 1, y + 1, width, height);
		mag2 = calcMagnitudeAngle(x - 1, y - 1, width, height);
	} else if (angle > 67.5 && angle <= 112.5) {
		// VERTICAL
		mag1 = calcMagnitudeAngle(x, y + 1, width, height);
		mag2 = calcMagnitudeAngle(x, y - 1, width, height);
	} else {
		mag1 = calcMagnitudeAngle(x - 1, y + 1, width, height);
		mag2 = calcMagnitudeAngle(x + 1, y - 1, width, height);
	}
	float mag1Norm = mag1.x * mag1.x + mag1.y * mag1.y; 
	float mag2Norm = mag2.x * mag2.x + mag2.y * mag2.y; 
	float magNorm = mag.x * mag.x + mag.y * mag.y; 
	
	float greyColor = mag1Norm < magNorm && mag2Norm < magNorm ? sqrt(magNorm) : 0;
	if (greyColor <= t1) {
		color = vec3(0);
	} else if (greyColor >= t2) {
		color = vec3(1);
	} else {
		vec2 ma = calcMagnitudeAngle(x + 1, y, width, height);
		float maNorm = ma.x * ma.x + ma.y * ma.y; 
		if (maNorm >= t2) {
			color = vec3(1);
			return;
		}
		vec2 mb = calcMagnitudeAngle(x - 1, y, width, height);
		float mbNorm = mb.x * mb.x + mb.y * mb.y; 
		if (mbNorm >= t2) {
			color = vec3(1);
			return;
		}
		
		vec2 mc = calcMagnitudeAngle(x, y + 1, width, height);
		float mcNorm = mc.x * mc.x + mc.y * mc.y; 
		if (mcNorm >= t2) {
			color = vec3(1);
			return;
		}

		vec2 md = calcMagnitudeAngle(x, y - 1, width, height);
		float mdNorm = md.x * md.x + md.y * md.y; 
		if (mdNorm >= t2) {
			color = vec3(1);
			return;
		}
		vec2 me = calcMagnitudeAngle(x + 1, y + 1, width, height);
		float meNorm = me.x * me.x + me.y * me.y; 
		if (meNorm >= t2) {
			color = vec3(1);
			return;
		}
		vec2 mf = calcMagnitudeAngle(x + 1, y - 1, width, height);
		float mfNorm = mf.x * mf.x + mf.y * mf.y; 
		if (mfNorm >= t2) {
			color = vec3(1);
			return;
		}
		
		vec2 mg = calcMagnitudeAngle(x - 1, y + 1, width, height);
		float mgNorm = mg.x * mg.x + mg.y * mg.y; 
		if (mgNorm >= t2) {
			color = vec3(1);
			return;
		}

		vec2 mh = calcMagnitudeAngle(x - 1, y - 1, width, height);
		float mhNorm = mh.x * mh.x + mh.y * mh.y; 
		if (mhNorm >= t2) {
			color = vec3(1);
			return;
		}
	}
}