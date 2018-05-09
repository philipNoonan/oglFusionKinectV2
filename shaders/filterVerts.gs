#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out; // you get an https://stackoverflow.com/questions/33814504/unknown-layout-specifier-triangles

uniform float threshold = 20.0f;

in vec2 VTexCoord[];
in float VvertCol[];
in vec4 VTexColor[];
in vec4 Vvert4D[];
in int VTextureType[];

out vec2 TexCoord;
out float vertCol;
out vec4 TexColor;
out vec4 vert4D;

void main()
{
// get the z values for each vert
// if the abs diff between any of the verts > threshold, discard the triangle, if not emit vertex and end primitive
// need to ensure that verts for the textures can get through ok

	if (VTextureType[0] == 1) // we are a normal texture, just display me
	{
		gl_Position = gl_in[0].gl_Position;
		TexCoord = VTexCoord[0];
		vertCol = VvertCol[0];
		TexColor = VTexColor[0];
		vert4D = Vvert4D[0];
		EmitVertex();

		gl_Position = gl_in[1].gl_Position;
		TexCoord = VTexCoord[1];
		vertCol = VvertCol[1];
		TexColor = VTexColor[1];
		vert4D = Vvert4D[1];
		EmitVertex();

		gl_Position = gl_in[2].gl_Position;
		TexCoord = VTexCoord[2];
		vertCol = VvertCol[2];
		TexColor = VTexColor[2];
		vert4D = Vvert4D[2];
		EmitVertex();

		EndPrimitive();
	}
	else // we are a point clouds vertex array, filter me
	{
		if (gl_in[0].gl_Position.z <= 100.0f || gl_in[1].gl_Position.z <= 100.0f || gl_in[2].gl_Position.z <= 100.0f || distance(gl_in[0].gl_Position.z, gl_in[1].gl_Position.z) > threshold || distance(gl_in[0].gl_Position.z, gl_in[2].gl_Position.z) > threshold || distance(gl_in[1].gl_Position.z, gl_in[2].gl_Position.z) > threshold)
		{

		}
		else
		{
			gl_Position = gl_in[0].gl_Position;
			TexCoord = VTexCoord[0];
			vertCol = VvertCol[0];
			TexColor = VTexColor[0];
			vert4D = Vvert4D[0];
			EmitVertex();

			gl_Position = gl_in[1].gl_Position;
			TexCoord = VTexCoord[1];
			vertCol = VvertCol[1];
			TexColor = VTexColor[1];
			vert4D = Vvert4D[1];
			EmitVertex();

			gl_Position = gl_in[2].gl_Position;
			TexCoord = VTexCoord[2];
			vertCol = VvertCol[2];
			TexColor = VTexColor[2];
			vert4D = Vvert4D[2];
			EmitVertex();

			EndPrimitive();
		
		}
	
	}




}