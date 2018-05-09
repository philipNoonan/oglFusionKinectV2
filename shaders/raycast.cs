#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding= 0) uniform isampler3D volumeDataTexture;
//layout(binding= 1) uniform sampler3D volumeColorTexture;
//layout(binding= 2) uniform sampler2D currentTextureColor;
     
layout(binding = 0, rg16i) uniform iimage3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively
layout(binding = 1, rgba32f) uniform image2D refVertex;
layout(binding = 2, rgba32f) uniform image2D refNormal;
    //layout(binding = 1, r32f) uniform image2D volumeSlice;
//layout(binding = 3, rgba32f) uniform image2D volumeSliceNorm;
//layout(binding = 2, rgba32f) uniform image3D volumeColor;



//layout(std430, binding= 2) buffer posTsdf3D // ouput
//{
//    vec4 PositionTSDF [];
//};

//layout(std430, binding= 3) buffer norm3D // ouput
//{
//    vec4 NormalTSDF [];
//};


uniform mat4 view; // == raycast pose * invK
uniform float nearPlane;
uniform float farPlane;
uniform float step;
uniform float largeStep;
uniform vec3 volDim;
uniform vec3 volSize;

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

float vs(uvec3 pos)
{  
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    //return data.x; // convert short to float

    ivec4 data = imageLoad(volumeData, ivec3(pos));
    return float(data.x); // convert short to float

}

float interpVol(vec3 pos)
{
    vec3 scaled_pos = vec3((pos.x * volSize.x / volDim.x) - 0.5f, (pos.y * volSize.y / volDim.y) - 0.5f, (pos.z * volSize.z / volDim.z) - 0.5f);
    ivec3 base = ivec3(floor(scaled_pos));
    vec3 factor = fract(scaled_pos);
    ivec3 lower = max(base, ivec3(0));
    ivec3 upper = min(base + ivec3(1), ivec3(volSize) - ivec3(1));
    return (
          ((vs(uvec3(lower.x, lower.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, lower.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, lower.z)) * factor.x) * factor.y) * (1 - factor.z)
        + ((vs(uvec3(lower.x, lower.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, upper.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, upper.z)) * factor.x) * factor.y) * factor.z
        ) * 0.00003051944088f;
}

float interpDistance(vec3 pos, inout bool interpolated)
{
    //vec3 scaled_pos = vec3((pos.x * volSize.x / volDim.x) - 0.5f, (pos.y * volSize.y / volDim.y) - 0.5f, (pos.z * volSize.z / volDim.z) - 0.5f);
     
    float i = pos.x;
    float j = pos.y;
    float k = pos.z;
    float w_sum = 0.0;
    float sum_d = 0.0;

    ivec3 current_voxel;
    float w = 0;
    float volume;
    int a_idx;
    interpolated = false;

    for (int i_offset = 0; i_offset < 2; i_offset++)
    {
        for (int j_offset = 0; j_offset < 2; j_offset++)
        {
            for (int k_offset = 0; k_offset < 2; k_offset++)
            {
                current_voxel.x = int(i) + i_offset;
                current_voxel.y = int(j) + j_offset;
                current_voxel.z = int(k) + k_offset;
                volume = abs(current_voxel.x - i) + abs(current_voxel.y - j) + abs(current_voxel.z - k);

                ivec4 data = imageLoad(volumeData, current_voxel);



                if (data.y > 0)
                {
                    interpolated = true;
                    // if (volume < 0.00001)
                    // {
                    //    return float (data.x * 0.00003051944088f);
                    // }
                    w = 1.0f / volume;
                    w_sum += float(w);
                    sum_d += float(w) * float(data.x * 0.00003051944088f);
                }

            }
        }
    }
    return sum_d / w_sum;
}

vec4 raycast(uvec2 pos)
{
    vec3 origin = vec3(view[3][0], view[3][1], view[3][2]);
    vec3 direction = rotate(view, vec3(pos.x, pos.y, 1.0f));

    // intersect ray with a box
    // http://www.siggraph.org/education/materials/HyperGraph/raytrace/rtinter3.htm
    // compute intersection of ray with all six bbox planes
    vec3 invR = vec3(1.0f, 1.0f, 1.0f) / direction;
    vec3 tbot = -1.0f * invR * origin;
    vec3 ttop = invR * (volDim - origin);
    // re-order intersections to find smallest and largest on each axis
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    // find the largest tmin and the smallest tmax
    float largest_tmin = max(max(tmin.x, tmin.y), max(tmin.x, tmin.z));
    float smallest_tmax = min(min(tmax.x, tmax.y), min(tmax.x, tmax.z));
    // check against near and far plane
    float tnear = max(largest_tmin, nearPlane);
    float tfar = min(smallest_tmax, farPlane);


    if (tnear < tfar)
    {

        // first walk with largesteps until we found a hit
        float t = tnear;
        float stepsize = largeStep;
        //float f_t = volume.interp(origin + direction * t); // do a sampler3D?

        //vec3 tPos = (origin + direction * t);
        //vec3 scaled_pos = vec3((tPos.x * volSize.x / volDim.x), (tPos.y * volSize.y / volDim.y), (tPos.z * volSize.z / volDim.z));
        //vec3 base = floor(scaled_pos);
        // vec4 interpData = texture(volumeDataTexture, vec3(origin + direction * t) / volDim.x); // volume texture reads should just be 0 - 1
        // ivec4 interpData = texture(volumeDataTexture, base / volSize.x); // volume texture reads should just be 0 - 1

        //float f_t = interpData.x;
        //float f_t = float(interpData.x) * 0.00003051944088f;
        bool isInterp;
        float f_t = interpVol(vec3(origin + direction * t));

        //vec3 tPos = (origin + direction * t);
        //vec3 scaled_pos = vec3((tPos.x * volSize.x / volDim.x) - 0.5f, (tPos.y * volSize.y / volDim.y) - 0.5f, (tPos.z * volSize.z / volDim.z) - 0.5f);
        //float f_t = interpDistance(scaled_pos, isInterp);

        // float currWeight = 0;

        float f_tt = 0;
        if (f_t >= 0)
        {  // ups, if we were already in it, then don't render anything here


            for (; t < tfar; t += stepsize)
            {
                //f_tt = volume.interp(origin + direction * t); // another sampler3d
                //vec4 d1 = texture(volumeDataTexture, vec3(origin + direction * t) / volDim.x); // if volDim is square!!!
                //ivec4 d1 = texture(volumeDataTexture, vec3(origin + direction * t) / volDim.x); // if volDim is square!!!

                //currWeight = d1.z;
                //f_tt = d1.x;
                //f_tt = float(d1.x) * 0.00003051944088f;

                f_tt = interpVol(vec3(origin + direction * t));

                //tPos = (origin + direction * t);
                //scaled_pos = vec3((tPos.x * volSize.x / volDim.x) - 0.5f, (tPos.y * volSize.y / volDim.y) - 0.5f, (tPos.z * volSize.z / volDim.z) - 0.5f);
                //f_tt = interpDistance(scaled_pos, isInterp);

                if (f_tt < 0) // got it, jump out of inner loop
                {
                    //imageStore(volumeSliceNorm, ivec2(pos), vec4(1, 0, 0, 1));

                    break; // 
                }
                if (f_tt < 0.8f)
                {
                    stepsize = step;
                }
                f_t = f_tt;
            }
            if (f_tt < 0) // got it, calculate accurate intersection
            {

                t = t + stepsize * f_tt / (f_t - f_tt);
                return vec4(origin + direction * t, t);

            }
        }
    }

    return vec4(0.0f, 0.0f, 0.0f, 0.0f);

}



vec3 getGradient(vec4 hit)
{
    vec3 scaled_pos = vec3((hit.x * volSize.x / volDim.x) - 0.5f, (hit.y * volSize.y / volDim.y) - 0.5f, (hit.z * volSize.z / volDim.z) - 0.5f);
    ivec3 baseVal = ivec3(floor(scaled_pos));
    vec3 factor = fract(scaled_pos);
    ivec3 lower_lower = max(baseVal - ivec3(1), ivec3(0));
    ivec3 lower_upper = max(baseVal, ivec3(0));
    ivec3 upper_lower = min(baseVal + ivec3(1), ivec3(volSize) - ivec3(1));
    ivec3 upper_upper = min(baseVal + ivec3(2), ivec3(volSize) - ivec3(1));
    ivec3 lower = lower_upper;
    ivec3 upper = upper_lower;

    vec3 gradient;

    gradient.x =
              (((vs(uvec3(upper_lower.x, lower.y, lower.z)) - vs(uvec3(lower_lower.x, lower.y, lower.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, lower.z)) - vs(uvec3(lower_upper.x, lower.y, lower.z))) * factor.x) * (1 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, lower.z)) - vs(uvec3(lower_lower.x, upper.y, lower.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, lower.z)) - vs(uvec3(lower_upper.x, upper.y, lower.z))) * factor.x) * factor.y) * (1 - factor.z)
            + (((vs(uvec3(upper_lower.x, lower.y, upper.z)) - vs(uvec3(lower_lower.x, lower.y, upper.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, upper.z)) - vs(uvec3(lower_upper.x, lower.y, upper.z))) * factor.x) * (1 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, upper.z)) - vs(uvec3(lower_lower.x, upper.y, upper.z))) * (1 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, upper.z)) - vs(uvec3(lower_upper.x, upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.y =
          (((vs(uvec3(lower.x, upper_lower.y, lower.z)) - vs(uvec3(lower.x, lower_lower.y, lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, lower.z)) - vs(uvec3(upper.x, lower_lower.y, lower.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, lower.z)) - vs(uvec3(lower.x, lower_upper.y, lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, lower.z)) - vs(uvec3(upper.x, lower_upper.y, lower.z))) * factor.x) * factor.y) * (1 - factor.z)
        + (((vs(uvec3(lower.x, upper_lower.y, upper.z)) - vs(uvec3(lower.x, lower_lower.y, upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, upper.z)) - vs(uvec3(upper.x, lower_lower.y, upper.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, upper.z)) - vs(uvec3(lower.x, lower_upper.y, upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, upper.z)) - vs(uvec3(upper.x, lower_upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.z =
          (((vs(uvec3(lower.x, lower.y, upper_lower.z)) - vs(uvec3(lower.x, lower.y, lower_lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_lower.z)) - vs(uvec3(upper.x, lower.y, lower_lower.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_lower.z)) - vs(uvec3(lower.x, upper.y, lower_lower.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_lower.z)) - vs(uvec3(upper.x, upper.y, lower_lower.z))) * factor.x) * factor.y) * (1 - factor.z)
        + (((vs(uvec3(lower.x, lower.y, upper_upper.z)) - vs(uvec3(lower.x, lower.y, lower_upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_upper.z)) - vs(uvec3(upper.x, lower.y, lower_upper.z))) * factor.x) * (1 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_upper.z)) - vs(uvec3(lower.x, upper.y, lower_upper.z))) * (1 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_upper.z)) - vs(uvec3(upper.x, upper.y, lower_upper.z))) * factor.x) * factor.y) * factor.z;

    return gradient * vec3(volDim.x / volSize.x, volDim.y / volSize.y, volDim.z / volSize.z) * (0.5f * 0.00003051944088f);


}

void main()
{
   // ivec2 depthSize = ivec2(512, 424);// imageSize(depthImage);

    uvec2 pix = gl_GlobalInvocationID.xy;

    vec4 hit = raycast(pix);


    if (hit.w > 0)
    {
        // vec4 currentColor = texture(currentTextureColor, vec2(pix.x / 1920.0f, pix.y / 1080.0f));

        // imageStore(volumeColor, ivec3(hit.xyz), currentColor);
        //imageStore(volumeSlice, ivec2(pix), vec4(mod(hit.w * 10.f, 1.0f), 0, 0, 0));
        // PositionTSDF[(pix.y * depthSize.x) + pix.x] = vec4(hit.xyz, 1.0f); // hit.w = hit.z + zshift
        imageStore(refVertex, ivec2(pix), vec4(hit.xyz, 1.0f));
        vec3 surfNorm = getGradient(hit);// volume.grad(make_float3(hit));
        if (length(surfNorm) == 0)
        {
            //imageStore(volumeSliceNorm, ivec2(pix), vec4(0, 0, 0, 0));
            imageStore(refNormal, ivec2(pix), vec4(0.0f));
                      //  NormalTSDF[(pix.y * depthSize.x) + pix.x] = vec4(0);
        }
        else
        {
            //imageStore(volumeSliceNorm, ivec2(pix), currentColor);

            //imageStore(volumeSliceNorm, ivec2(pix), vec4(normalize(surfNorm), 0.0f));
           // NormalTSDF[(pix.y * depthSize.x) + pix.x] = vec4(normalize(surfNorm), 0.0f);
            imageStore(refNormal, ivec2(pix), vec4(normalize(surfNorm), 1.0f));

        }
    }
    else
    {
        //imageStore(volumeSlice, ivec2(pix), vec4(hit.z / 2.0f, 0, 0, 0));
        //imageStore(volumeSliceNorm, ivec2(pix), vec4(0, 0, 0, 0));
        imageStore(refVertex, ivec2(pix), vec4(0.0f));
        imageStore(refNormal, ivec2(pix), vec4(0.0f));

        //PositionTSDF[(pix.y * depthSize.x) + pix.x] = vec4(0);
       // NormalTSDF[(pix.y * depthSize.x) + pix.x] = vec4(0, 0, 0, 0); // set x = 2??
    }


    //vec3 pos = opMul(invTrack, getVolumePosition(pix));
    //vec3 cameraX = opMul(K, pos);
    //vec3 delta = rotate(invTrack, vec3(0.0f, 0.0f, volDim.z / volSize.z));
    //vec3 cameraDelta = rotate(K, delta);
}










// USEFUL SANITY CHECKING STUFF

// vec4 interpData = texture(volumeDataTexture, vec3(pix / 256.0f, 2)); // texture float reads are from 0 - 1
//vec4 interpData = texelFetch(volumeDataTexture, ivec3(pix, 0), 0);
// vec4 interpData = imageLoad(volumeData, ivec3(pix, 0));

//if (interpData.x > -5.0f && interpData.x < 0.0f)
//{
//    imageStore(volumeSlice, ivec2(pix), vec4(0.5f, 0, 0, 0));
//}

//if (interpData.x > 0.0f && interpData.x < 5.0f)
//{
//    imageStore(volumeSlice, ivec2(pix), vec4(0.25f, 0, 0, 0));
//}