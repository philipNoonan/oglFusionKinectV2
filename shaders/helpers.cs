#version 430

layout(local_size_x = 32, local_size_y = 32) in; // 

layout(binding = 0, rg16i) uniform iimage3D volumeData;
layout(binding = 1, r32f) uniform image2D testImage;


layout(std430, binding= 0) buffer pos2D
{
    vec2 Position2D [];
};

layout(std430, binding= 1) buffer pos3D
{
    vec3 Position3D [];
};


uniform vec3 volSize;
uniform mat4 invK;
uniform int buffer2DWidth;

subroutine void helperFunction();
subroutine uniform helperFunction performHelperFunction;

vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
}

subroutine(helperFunction)
void resetVolume()
{
    uvec2 pix = gl_GlobalInvocationID.xy;

    if (pix.x < volSize.x && pix.y < volSize.y)
    {
        for (int zDep = 0; zDep < volSize.z; zDep++)
        {
            imageStore(volumeData, ivec3(pix.x, pix.y, zDep), ivec4(0));
        }
    }

}

subroutine(helperFunction)
void trackPointsToVerts()
{
  //  ivec2 pos = ivec2(gl_GlobalInvocationID.xy);

  //  vec2 pix = Position2D[(pos.y * buffer2DWidth) + pos.x];

   // float depth = imageLoad(testImage, ivec2(pix)).x;

   // vec3 tPos = (depth / 1000.0f) * rotate(invK, vec3(pix.x, pix.y, 1.0f));

    //Position3D[(pos.y * int(float(buffer2DWidth) * 1.5f)) + pos.x] = tPos;



}

void main()
{
    performHelperFunction();
}