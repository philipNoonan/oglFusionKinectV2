#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0, rg16i) uniform iimage3D volumeData;
layout(binding = 1, rgba32f) uniform image2D outputImage;

uniform mat4 view; // == raycast pose * invK
uniform float nearPlane;
uniform float farPlane;
uniform float step;
uniform float largeStep;
uniform vec3 volDim;
uniform vec3 volSize;


float vs(uvec3 pos)
{
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    //return data.x; // convert short to float

    ivec4 data = imageLoad(volumeData, ivec3(pos));
    return float(data.x); // convert short to float

}
vec3 rotate(mat4 M, vec3 V)
{
    // glsl and glm [col][row]
    return vec3(dot(vec3(M[0][0], M[1][0], M[2][0]), V),
                dot(vec3(M[0][1], M[1][1], M[2][1]), V),
                dot(vec3(M[0][2], M[1][2], M[2][2]), V));
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

vec4 intensityProjection(uvec2 pos)
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

    vec4 rayColor = vec4(0,0,0,1);

    if (tnear < tfar)
    {
        float t = tnear;
        float stepsize = largeStep;

        float f_tt = 0;

        for (; t < tfar; t += stepsize)
        {
            f_tt = interpVol(vec3(origin + direction * t));

            if (f_tt > 0)
            {
                rayColor.x += f_tt;
            }
            else if (f_tt == 0)
            {
                rayColor.z += f_tt;
            }
            else if (f_tt > 0)
            {
                rayColor.y += f_tt;
            }
        }
    }

    return rayColor;


}


void main()
{

    uvec2 pix = gl_GlobalInvocationID.xy;

    vec4 rayCol = intensityProjection(pix);

    imageStore(outputImage, ivec2(pix), rayCol);


}
