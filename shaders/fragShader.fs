#version 430 core
const float PI = 3.1415926535897932384626433832795f;

layout (binding=0) uniform sampler2D currentTextureDepth;
layout (binding=1) uniform sampler2D currentTextureNormal;
layout (binding=2) uniform sampler2D currentTextureTrack;
layout (binding=3) uniform sampler2D currentTextureFlow;
layout (binding=4) uniform sampler2D currentTextureColor;

layout (binding=5) uniform isampler3D currentTextureVolume; 
layout(binding = 5, rg16i) uniform iimage3D volumeData; // texel access


in vec2 TexCoord;
in float zDepth;
layout(location = 0) out vec4 color;

uniform mat4 model;
uniform vec3 ambient;
uniform vec3 light;
uniform float irLow = 0.0f;
uniform float irHigh = 65536.0f;

uniform float slice;


subroutine vec4 getColor();
subroutine uniform getColor getColorSelection;

subroutine(getColor)
vec4 fromDepth()
{
	vec4 tColor = texture(currentTextureDepth, vec2(TexCoord));
    float valueColor = smoothstep(0.1, 0.5, tColor.x / 1000.0f);
	return vec4(valueColor.xxx, 1.0f);
}

subroutine(getColor)
vec4 fromColor()
{
	vec4 tColor = texture(currentTextureColor, vec2(TexCoord));
	return tColor.zyxw;
}


subroutine(getColor)
vec4 fromRayNorm()
{
	vec4 tCol = texture(currentTextureNormal, vec2(TexCoord));
	return vec4(tCol.xy, -tCol.z, tCol.w);
}

subroutine(getColor)
vec4 fromTrack()
{
	vec4 tCol = texture(currentTextureTrack, vec2(TexCoord));
	return vec4(tCol);
}


subroutine(getColor)
vec4 fromPoints()
{
	return vec4(0.03f, 1.0f, 0.02f, 1.0f);
}

int ncols = 0;
int MAXCOLS = 60;
int colorwheel[60][3];


void setcols(int r, int g, int b, int k)
{
    colorwheel[k][0] = r;
    colorwheel[k][1] = g;
    colorwheel[k][2] = b;
}

void makecolorwheel()
{
    // relative lengths of color transitions:
    // these are chosen based on perceptual similarity
    // (e.g. one can distinguish more shades between red and yellow 
    //  than between yellow and green)
    int RY = 15;
    int YG = 6;
    int GC = 4;
    int CB = 11;
    int BM = 13;
    int MR = 6;
    ncols = RY + YG + GC + CB + BM + MR;
    //printf("ncols = %d\n", ncols);
    if (ncols > MAXCOLS) return;

    int i;
    int k = 0;
    for (i = 0; i < RY; i++) setcols(255,	   255*i/RY,	 0,	       k++);
    for (i = 0; i < YG; i++) setcols(255-255*i/YG, 255,		 0,	       k++);
    for (i = 0; i < GC; i++) setcols(0,		   255,		 255*i/GC,     k++);
    for (i = 0; i < CB; i++) setcols(0,		   255-255*i/CB, 255,	       k++);
    for (i = 0; i < BM; i++) setcols(255*i/BM,	   0,		 255,	       k++);
    for (i = 0; i < MR; i++) setcols(255,	   0,		 255-255*i/MR, k++);
}

subroutine(getColor)
vec4 fromFlow()
{
	int length = 50;
	vec4 tFlow = textureLod(currentTextureFlow, TexCoord, 0);
/*	 
	if (ncols == 0)
	makecolorwheel();

	vec4 outcol = vec4(1);
	float rad = sqrt(tFlow.x * tFlow.x + tFlow.y * tFlow.y);
    float a = atan(-tFlow.y, -tFlow.x) / PI;
    float fk = (a + 1.0) / 2.0 * (ncols-1);
    int k0 = int(fk);
    int k1 = (k0 + 1) % ncols;
    float f = fk - k0;
    //f = 0; // uncomment to see original color wheel
    for (int b = 0; b < 3; b++) 
	{
		float col0 = colorwheel[k0][b] / 255.0;
		float col1 = colorwheel[k1][b] / 255.0;
		float col = (1 - f) * col0 + f * col1;
		if (rad <= 1)
			col = 1 - rad * (1 - col); // increase saturation with radius
		else
			col *= .75; // out of range
		
		outcol[2 - b] = col;
    }

	return vec4(outcol.xyz, 0.5);




//	vec4 tDep = texture(currentTextureDepth, vec2(TexCoord.x + (tFlow.x / 1920.0f), TexCoord.y + (tFlow.y / 1080.0f)));
vec4 color = vec4(0);
	for (int i = 0; i < length; i++)
	{
		color += texture(currentTextureFlow, TexCoord + tFlow.xy * float(i) / 1000.0f, 0);
		color += texture(currentTextureFlow, TexCoord - tFlow.xy * float(i) / 1000.0f, 0);

	}

	return color / 50.0f; 
	// cart to polar
	// sqrt(x^2 + y^2) = magnitude
	// atan (y / x) = angle from x axis

*/ 


	float mag = sqrt(tFlow.x * tFlow.x + tFlow.y * tFlow.y);
	float ang = atan(tFlow.y,  tFlow.x);

	//https://gist.github.com/KeyMaster-/70c13961a6ed65b6677d

	ang -= 1.57079632679;
    if(ang < 0.0) 
	{
		ang += 6.28318530718; 
    }
    ang /= 6.28318530718; 
	ang = 1.0 - ang; 

	//float smoothMag = smoothstep(0.25, 10.0, mag);

	// ang to rgb taken from https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
	vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(ang + K.xyz) * 6.0 - K.www);

    vec3 rgb = mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), mag * 0.1);

//	return vec4(1.1 * tFlow.x * tFlow.x, 1.1 * tFlow.y * tFlow.y, 0, 1); 

	//return vec4(tFlow.x < 0 ? 1 : 0, tFlow.y < 0 ? 1 : 0, 0, 1);
	//return vec4(rgb, mag > 1.0 ? 1 : 0);
		return vec4(rgb, 0.7);

}

subroutine(getColor)
vec4 fromVolume()
{
	ivec4 tData = texture(currentTextureVolume, vec3(TexCoord, slice) );
	//ivec4 tData = imageLoad(volumeData, ivec3(TexCoord.x * 128.0f, TexCoord.y * 128.0f, slice));

	return vec4(1.0f * float(tData.x) * 0.00003051944088f, 1.0f * float(tData.x) * -0.00003051944088f, 0.0, 1.0f);
	//return vec4(1.0, 0.0, 0.0, 1.0);
}


void main()
{
	//vec3 normals = normalize(cross(dFdx(vert4D.xyz), dFdy(vert4D.xyz)));



	color = getColorSelection();

}