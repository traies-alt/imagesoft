#version 330 core

in vec2 UV;
layout(location = 0) out vec3 color;
uniform sampler2D myTextureSampler;
uniform int borderDetector;
uniform float sigma;
uniform float width;
uniform float height;

void main() {
    vec3 pixelData = texture( myTextureSampler, UV ).rgb;

    float x = floor(UV.x * width);
    float y = floor(UV.y * height);

    if(x==0 || x>width-1 || y == 0 || y > height-1) {
        color=pixelData;
        return;
    }

    vec3 Dn = texture(myTextureSampler, vec2(float(x + 1) / width, UV.y)).rgb - pixelData;
    vec3 Ds = texture(myTextureSampler, vec2(float(x - 1) / width, UV.y)).rgb - pixelData;
    vec3 De = texture(myTextureSampler, vec2(UV.x, float(y + 1) / height)).rgb - pixelData;
    vec3 Do = texture(myTextureSampler, vec2(UV.x, float(y + 1) / height)).rgb - pixelData;

    vec3 Dn_2 = vec3(Dn.r*Dn.r, Dn.g*Dn.g, Dn.b*Dn.b);
    vec3 Ds_2 = vec3(Ds.r*Ds.r, Ds.g*Ds.g, Ds.b*Ds.b);
    vec3 De_2 = vec3(De.r*De.r, De.g*De.g, De.b*De.b);
    vec3 Do_2 = vec3(Do.r*Do.r, Do.g*Do.g, Do.b*Do.b);

    vec3 cn = vec3(1,1,1);
    vec3 cs = vec3(1,1,1);
    vec3 ce = vec3(1,1,1);
    vec3 co = vec3(1,1,1);

    if(borderDetector == 1) {
        cn = exp(-(Dn_2) / ((sigma) * (sigma)) );
        cs = exp(-(Ds_2) / ((sigma) * (sigma)) );
        ce = exp(-(De_2) / ((sigma) * (sigma)) );
        co = exp(-(Do_2) / ((sigma) * (sigma)) );
    } else if(borderDetector == 2) {
        cn = 1.0f/(  (  ( (Dn_2) / (sigma * sigma)) + 1  ));
        cs = 1.0f/(  (  ( (Ds_2) / (sigma * sigma)) + 1  ));
        ce = 1.0f/(  (  ( (De_2) / (sigma * sigma)) + 1  ));
        co = 1.0f/(  (  ( (Do_2) / (sigma * sigma)) + 1  ));
    }

    color = pixelData + 0.25 *(Dn*cn+ Ds*cs + De*ce + Do*co);
}
