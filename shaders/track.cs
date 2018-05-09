#version 430

layout (local_size_x = 32, local_size_y = 32) in;

uniform mat4 view;
uniform mat4 Ttrack;
uniform float dist_threshold;
uniform float normal_threshold;

//layout(binding = 0, rgba8ui) uniform image2D TrackData;
layout(binding = 0, rgba32f) uniform image2D inVertex;
layout(binding = 1, rgba32f) uniform image2D inNormal;
layout(binding = 2, rgba32f) uniform image2D refVertex;
layout(binding = 3, rgba32f) uniform image2D refNormal;
layout(binding = 4, r32f) uniform image2D differenceImage;
layout(binding = 5, rgba32f) uniform image2D trackImage;

struct reduType
{
    int result;
    float error;
    float J[6];
};
layout(std430, binding = 0) buffer TrackData
{
    reduType trackOutput[];
};

vec3 opMul(mat4 M, vec3 v)
{
    return vec3(
        dot(vec3(M[0][0], M[1][0], M[2][0]), v) + M[3][0],
        dot(vec3(M[0][1], M[1][1], M[2][1]), v) + M[3][1],
        dot(vec3(M[0][2], M[1][2], M[2][2]), v) + M[3][2]);
}

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
    ivec2 inSize = imageSize(inVertex); // mipmapped sizes
    ivec2 refSize = imageSize(refVertex); // full depth size

    if (pix.x < inSize.x && pix.y < inSize.y)
    {
        vec4 normals = imageLoad(inNormal, ivec2(pix));

        if (normals.x == 2)
        {
            trackOutput[(pix.y * inSize.x) + pix.x].result = -1; // does this matter since we are in a low mipmap not full size???
            imageStore(trackImage, ivec2(pix), vec4(0,0,0,0));

        }
        else
        {
            vec4 projectedVertex = Ttrack * vec4(imageLoad(inVertex, ivec2(pix)).xyz, 1.0f); // CHECK ME AGAINT THE OLD CRAPPY OPMUL
            //vec3 projectedVertex = opMul(Ttrack, imageLoad(inVertex, ivec2(pix)).xyz); // CHECK ME AGAINT THE OLD CRAPPY OPMUL

            vec4 projectedPos = view * projectedVertex;
            //vec3 projectedPos = opMul(view, projectedVertex);


            vec2 projPixel = vec2(projectedPos.x / projectedPos.z + 0.5f, projectedPos.y / projectedPos.z + 0.5f);



            // vec2 projPixel = vec2(pix.x * 2, pix.y * 2);


            if (projPixel.x < 0 || projPixel.x > refSize.x - 1 || projPixel.y < 0 || projPixel.y > refSize.y - 1)
            {
                trackOutput[(pix.y * inSize.x) + pix.x].result = -2;
                imageStore(trackImage, ivec2(pix), vec4(1.0f, 0, 0, 1.0f));

            }
            else
            {
                ivec2 refPixel = ivec2(projPixel.x, projPixel.y);
                vec3 referenceNormal = imageLoad(refNormal, refPixel).xyz;
                vec3 tmp = imageLoad(refVertex, refPixel).xyz;
                imageStore(differenceImage, ivec2(projPixel), vec4(tmp.z, 0.0f, 0.0f, 1.0f));


                if (referenceNormal.x == -2)
                {
                    trackOutput[(pix.y * inSize.x) + pix.x].result = -3;
                    imageStore(trackImage, ivec2(pix), vec4(0, 1.0f, 0, 1.0f));

                }
                else
                {
                    vec3 diff = imageLoad(refVertex, refPixel).xyz - projectedVertex.xyz;

                    vec3 projectedNormal = rotate(Ttrack, imageLoad(inNormal, ivec2(pix)).xyz); // input mipmap sized pixel

                    if (length(diff) > dist_threshold)
                    {
                        trackOutput[(pix.y * inSize.x) + pix.x].result = -4;
                        imageStore(trackImage, ivec2(pix), vec4(0, 0, 1.0f, 1.0f));

                    }
                    else if (dot(projectedNormal, referenceNormal) < normal_threshold)
                    {
                        trackOutput[(pix.y * inSize.x) + pix.x].result = -5;
                        imageStore(trackImage, ivec2(pix), vec4(1.0f, 1.0f, 0, 1.0f));

                    }
                    else
                    {
                        imageStore(trackImage, ivec2(pix), vec4(0.5f, 0.5f, 0.5f, 1.0f));


                        trackOutput[(pix.y * inSize.x) + pix.x].result = 1;
                        trackOutput[(pix.y * inSize.x) + pix.x].error = dot(referenceNormal, diff);

                        trackOutput[(pix.y * inSize.x) + pix.x].J[0] = referenceNormal.x;
                        trackOutput[(pix.y * inSize.x) + pix.x].J[1] = referenceNormal.y;
                        trackOutput[(pix.y * inSize.x) + pix.x].J[2] = referenceNormal.z;

                        vec3 crossProjVertRefNorm = cross(projectedVertex.xyz, referenceNormal);
                        trackOutput[(pix.y * inSize.x) + pix.x].J[3] = crossProjVertRefNorm.x;
                        trackOutput[(pix.y * inSize.x) + pix.x].J[4] = crossProjVertRefNorm.y;
                        trackOutput[(pix.y * inSize.x) + pix.x].J[5] = crossProjVertRefNorm.z;
                    }
                }

            }
        }
        







        // vec4 color = vec4(texture(currentTextureColor, vec2(pix.x / 1920.0f, pix.y / 1080.0f)));
        //if (depth.x > 0)
        //{
        //x = (pix.x - camPams.z) * (1.0f / camPams.x) * depth.x;
        //y = (pix.y - camPams.w) * (1.0f / camPams.y) * depth.x;
        //z = depth.x;
        //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(x, y, z, 0.0f));
        //Position3D[(pix.y * size.x) + pix.x] = vec4(x, y, z, 0.0f);

       // vec3 tPos = depth.x * rotate(invK, vec3(pix.x, pix.y, 1.0f));
       // imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(tPos, 0.0f));
      //  Position3D[(pix.y * size.x) + pix.x] = vec4(tPos, 0.0f);



    }

}