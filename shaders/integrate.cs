#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rg16i) uniform iimage3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively
layout(binding = 1, r32f) uniform image2D depthImage;

uniform mat4 invTrack;
uniform mat4 K;
uniform float mu;
uniform float maxWeight;
uniform vec3 volDim; // vol dim real world span of the volume in meters
uniform vec3 volSize; // voxel grid size of the volume

// this returns the normailsed float distance for the volume 0 - 1
vec3 getVolumePosition(uvec3 p)
{
    return vec3((p.x + 0.5f) * volDim.x / volSize.x, (p.y + 0.5f) * volDim.y / volSize.y, (p.z + 0.5f) * volDim.z / volSize.z);
}

vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
}

vec3 opMul(mat4 M, vec3 v)
{
    return vec3(
        dot(vec3(M[0][0], M[1][0], M[2][0]), v) + M[3][0],
        dot(vec3(M[0][1], M[1][1], M[2][1]), v) + M[3][1],
        dot(vec3(M[0][2], M[1][2], M[2][2]), v) + M[3][2]);
}

vec4 vs(uvec3 pos)
{
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    ivec4 data = imageLoad(volumeData, ivec3(pos));

    return vec4(float(data.x) * 0.00003051944088f, data.y, data.zw); 
}

void set(uvec3 _pix, vec4 _data)
{
    //imageStore(volumeData, vec3(_pix), ivec4(_data.x, _data.y, _data.zw));
    imageStore(volumeData, ivec3(_pix), ivec4(int(_data.x * 32766.0f), int(_data.y), _data.zw));

}

void main()
{
    ivec2 depthSize = ivec2(512, 424);// imageSize(depthImage);
    uvec3 pix = gl_GlobalInvocationID.xyz;

    vec3 pos = opMul(invTrack, getVolumePosition(pix));
    vec3 cameraX = opMul(K, pos);
    vec3 delta = rotate(invTrack, vec3(0.0f, 0.0f, volDim.z / volSize.z));
    vec3 cameraDelta = rotate(K, delta);

    for (pix.z = 0; pix.z < volSize.z; ++pix.z, pos += delta, cameraX += cameraDelta)
    {
        if (pos.z < 0.0001f)
            continue;

        vec2 pixel = vec2(cameraX.x / cameraX.z + 0.5f, cameraX.y / cameraX.z + 0.5f);

        if (pixel.x < 0 || pixel.x > depthSize.x - 1 || pixel.y < 0 || pixel.y > depthSize.y - 1)
            continue;

        uvec2 px = uvec2(pixel.x, pixel.y);
        vec4 depth = imageLoad(depthImage, ivec2(px));
        depth.x = depth.x / 1000.0f;

        if (depth.x == 0)
            continue;

        float diff = (depth.x - cameraX.z) * sqrt(1 + pow(pos.x / pos.z, 2) + pow(pos.y / pos.z, 2));
        //if (abs(diff) < 0.1f)
            if (diff > -mu)
            {
                //if ((diff) < 0.1)
                //{
                float sdf = min(1.0f, diff / mu);
                vec4 data = vs(pix);
                float weightedDistance = (data.y * data.x + sdf) / (data.y + 1);
                //float weightedDistance = (data.y * data.x + diff) / (data.y + 1);

                //if (weightedDistance < 0.1f)
                //{
                    data.x = clamp(weightedDistance, -1.0f, 1.0f);
                    // data.x = diff;
                    data.y = min(data.y + 1, maxWeight);
                //}
                //else
               // {
                //    data.x = 0;
                //    data.y = 0;
                //}
  
                set(pix, data);
            }
            else
            {
                vec4 data;// = vs(pix);

                data.x = 0;
                //// data.x = diff;

                data.y = 0;

                set(pix, data);
            }
        //}
        //else
        //{
        //    vec4 data;// = vs(pix);

        //    data.x = 0;
        //    //// data.x = diff;

        //    data.y = 0;

        //    set(pix, data);
        //}


    }



}



//void main()
//{
//    ivec2 depthSize = imageSize(depthImage);
//    uvec3 pix = gl_GlobalInvocationID.xyz;

//    vec3 pos = opMul(invTrack, getVolumePosition(pix));

//    vec3 cameraX = opMul(K, pos);
//    vec3 delta = rotate(invTrack, vec3(0.0f, 0.0f, volDim.z / volSize.z));
//    vec3 cameraDelta = rotate(K, delta);


//    for (pix.z = 0; pix.z < volSize.z; ++pix.z, pos += delta, cameraX += cameraDelta)
//    {
//        if (pos.z < 0.0001f)
//            continue;

//        vec2 pixel = vec2(cameraX.x / cameraX.z + 0.5f, cameraX.y / cameraX.z + 0.5f);

//        if (pixel.x < 0 || pixel.x > depthSize.x - 1 || pixel.y < 0 || pixel.y > depthSize.y - 1)
//            continue;

//        uvec2 px = uvec2(pixel.x, pixel.y);
//        vec4 depth = imageLoad(depthImage, ivec2(px));
//        depth.x = depth.x / 1000.0f;

//        vec4 data = vs(pix);



//        float diff = (depth.x - cameraX.z) * sqrt(1 + pow(pos.x / pos.z, 2) + pow(pos.y / pos.z, 2));

//        if (abs(diff) < 1.0f)
//        {
//            data.x = diff;
//            data.y = min(data.y + 1, maxWeight);
//            data.z = 100.0f;
//        }
//        else
//        {
//            data.z = max(0.0f, data.z - 1.0f);
//            if (data.z == 0)
//            {
//                data.x = 0.0f;
//            }
//        }
//        set(pix, data);



//        //if (abs(diff) > mu)
//        //{
//        //    float sdf = min(1.0f, diff / mu);
//        //    //vec4 data = vs(pix);
//        //    float weightedDistance = (data.y * data.x + sdf) / (data.y + 1);





//        //    if (diff > 0.05f)
//        //    {
//        //        data.z = max(0.0f, data.z - 1.0f);
//        //    }
//        //    else
//        //    {
//        //        data.y = min(data.y + 1, maxWeight);
//        //        data.z = 100.0f;
//        //    }

//        //    data.x = clamp(weightedDistance, -1.0f, 1.0f);
//        //    set(pix, data);


//        //}
//        //else
//        //{
//        //    data.z = max(0.0f, data.z - 1.0f);
//        //    set(pix, data);
//        //}
//    }









//}






// useful sanity checking stuff

//if (pix.x< 128)
//{
//    imageStore(volumeData, ivec3(pix.x, pix.y, 0), vec4(-2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 1), vec4(-2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 2), vec4(-2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 3), vec4(-2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 4), vec4(-2.0f, 2.0f, 1.0f, 3.0f));


//}
//else
//{
//    imageStore(volumeData, ivec3(pix.x, pix.y, 0), vec4(2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 1), vec4(2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 2), vec4(2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 3), vec4(2.0f, 2.0f, 1.0f, 3.0f));
//    imageStore(volumeData, ivec3(pix.x, pix.y, 4), vec4(2.0f, 2.0f, 1.0f, 3.0f));

//}
