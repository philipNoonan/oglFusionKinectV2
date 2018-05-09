#version 430 core
layout (location = 0) in vec3 positionColor;
layout (location = 1) in vec2 texCoordColor;
layout (location = 2) in vec3 positionDepth;
layout (location = 3) in vec2 texCoordDepth;

layout (location = 4) in vec3 position;
layout (location = 5) in vec2 texCoord;

layout (location = 6) in vec4 position4D;

layout (location = 7) in vec2 trackedPoints; 

//layout (std430, binding = 7) buffer trackedPoints; 

uniform mat4 model;
uniform mat4 ViewProjection;
uniform mat4 projection;
uniform mat4 MVP;
uniform vec2 imSize;

out vec2 TexCoord;
out float zDepth;

subroutine vec4 getPosition();
subroutine uniform getPosition getPositionSubroutine;

subroutine(getPosition)
vec4 fromTexture()
{
	TexCoord = vec2(texCoordColor.x, 1 - texCoordColor.y);
	return vec4(MVP * vec4(positionDepth, 1.0f));
}

subroutine(getPosition)
vec4 fromStandardTexture()
{
	TexCoord = vec2(texCoord.x, 1 - texCoord.y);
	return vec4(MVP * vec4(((position.x + 1.0) / 2.0) * imSize.x, ((position.y + 1.0) / 2.0) * imSize.y, position.z, 1.0f));
}

subroutine(getPosition)
vec4 fromPosition4D()
{
	return vec4(MVP * vec4(position4D.xyz, 1.0f));
	zDepth = position4D.z;
}

subroutine(getPosition)
vec4 fromPosition2D()
{
    gl_PointSize = 0.5f;
	return vec4(MVP * vec4(trackedPoints.x, imSize.y - trackedPoints.y, 0.0f, 1.0f));
	//	return vec4(trackedPoints.xy, 1000.0f, 1.0f);

}



void main()
{
	gl_Position = getPositionSubroutine();
}