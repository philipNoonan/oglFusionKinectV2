#version 430

layout (local_size_x = 32, local_size_y = 32) in;

uniform mat4 invK;
uniform vec4 camPams; // camPams.x = fx, camPams.y = fy, camPams.z = cx, camPams.w = cy

layout(binding=0, r32f) uniform image2D InputImage;
layout(binding=1, rgba32f) uniform image2D OutputImage;

layout(std430, binding=0) buffer pos3D
{
    vec4 Position3D[];
};


//layout(std430, binding= 1) buffer color3D
//{
//    vec4 Color3D [];
//};


vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
}

// int oneDindex = (row * length_of_row) + column; // Indexes

void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    ivec2 size = imageSize(InputImage);
    //pix.y = pix.y += 2;

    float x;
    float y;
    float z;

    if (pix.x < size.x && pix.y < size.y)
    {
        vec4 depth = imageLoad(InputImage, ivec2(pix));
        // vec4 color = vec4(texture(currentTextureColor, vec2(pix.x / 1920.0f, pix.y / 1080.0f)));
        //if (depth.x > 0)
        //{
        vec3 tPos;
        //tPos.x = (pix.x - camPams.z) * (1.0f / camPams.x) * depth.x / 1000.0f;
        //tPos.y = (pix.y - camPams.w) * (1.0f / camPams.y) * depth.x / 1000.0f;
        //tPos.z = depth.x / 1000.0f;
        //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(x, y, z, 0.0f));
        //Position3D[(pix.y * size.x) + pix.x] = vec4(x, y, z, 0.0f);

        tPos = (depth.x / 1000.0f) * rotate(invK, vec3(pix.x, pix.y, 1.0f));
        imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(tPos, 0.0f));
        Position3D[(pix.y * size.x) + pix.x] = vec4(tPos, 0.0f);

        //Color3D[(pix.y * size.x) + pix.x] = vec4(color.xyz,0); // FIX ME DONT USE FLOAT £" FOR COLOR USE BYTES!!!
        //Position3D[(pix.x * size.x) + pix.y] = vec3(pix.x / 100.0f, pix.y / 100.0f, -depth / 100.0f);

        //}
        //else
        //{
        //   imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
        //   Position3D[(pix.x * size.x) + pix.y] = vec4(0, 0, 0, 0);
        //}

    }

}