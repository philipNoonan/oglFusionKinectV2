#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding = 0) uniform isampler3D volumeDataTexture; // interp access

layout(binding = 0, rg16i) uniform iimage3D volumeData; // texel access
layout(binding = 1, rgba32f) uniform image2D inVertex;

layout(binding = 2, rgba32f) uniform image2D testImage;
struct reduSDFType
{
    float h;
    float D;
    float J[6];
};
layout(std430, binding = 12) buffer TrackData
{
    reduSDFType trackOutput [];
};

uniform ivec2 imageSize; 

uniform mat4 Ttrack;
uniform vec3 volDim;
uniform vec3 volSize; 
uniform float c;
uniform float eps;

mat3 r1p;
mat3 r1m;

mat3 r2p;
mat3 r2m;

mat3 r3p;
mat3 r3m;

void setMats()
{
    float w_h = 0.01f;
    mat3 rot;
    rot[0] = vec3(Ttrack[0][0], Ttrack[0][1], Ttrack[0][2]);
    rot[1] = vec3(Ttrack[1][0], Ttrack[1][1], Ttrack[1][2]);
    rot[2] = vec3(Ttrack[2][0], Ttrack[2][1], Ttrack[2][2]);

    mat3 Rotdiff;
    Rotdiff[0][0] = 1.0f; Rotdiff[1][0] = 0.0f; Rotdiff[2][0] = 0.0f;
    Rotdiff[0][1] = 0.0f; Rotdiff[1][1] = 1.0f; Rotdiff[2][1] = -w_h;
    Rotdiff[0][2] = 0.0f; Rotdiff[1][2] = w_h;  Rotdiff[2][2] = 1.0f;

    r1p = Rotdiff * rot;

    Rotdiff[2][1] = w_h;
    Rotdiff[1][2] = -w_h;

    r1m = Rotdiff * rot;

    Rotdiff[2][1] = 0;
    Rotdiff[1][2] = 0;
    Rotdiff[2][0] = w_h;
    Rotdiff[0][2] = -w_h;

    r2p = Rotdiff * rot;

    Rotdiff[2][0] = -w_h;
    Rotdiff[0][2] = w_h;

    r2m = Rotdiff * rot;

    Rotdiff[2][0] = 0;
    Rotdiff[0][2] = 0;
    Rotdiff[1][0] = -w_h;
    Rotdiff[0][1] = w_h;

    r3p = Rotdiff * rot;

    Rotdiff[1][0] = w_h;
    Rotdiff[0][1] = -w_h;

    r3m = Rotdiff * rot;
}

//float vs(uvec3 pos, inout bool interpolated)

float vs(uvec3 pos)
{
    //vec4 data = imageLoad(volumeData, ivec3(pos));
    //return data.x; // convert short to float

    ivec4 data = imageLoad(volumeData, ivec3(pos));
    //if (data.y > 0)
    //{
    //    interpolated = true;
    //}
    //else
    //{
    //    interpolated = false;
    //}
    //return 0.0f;
    return float(data.x); // convert short to float

}

int vsGrad(uvec3 pos)
{
    ivec4 data = imageLoad(volumeData, ivec3(pos));
    return data.y;
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
              (((vs(uvec3(upper_lower.x, lower.y, lower.z)) - vs(uvec3(lower_lower.x, lower.y, lower.z))) * (1.0 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, lower.z)) - vs(uvec3(lower_upper.x, lower.y, lower.z))) * factor.x) * (1.0 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, lower.z)) - vs(uvec3(lower_lower.x, upper.y, lower.z))) * (1.0 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, lower.z)) - vs(uvec3(lower_upper.x, upper.y, lower.z))) * factor.x) * factor.y) * (1.0 - factor.z)
            + (((vs(uvec3(upper_lower.x, lower.y, upper.z)) - vs(uvec3(lower_lower.x, lower.y, upper.z))) * (1.0 - factor.x)
            + (vs(uvec3(upper_upper.x, lower.y, upper.z)) - vs(uvec3(lower_upper.x, lower.y, upper.z))) * factor.x) * (1.0 - factor.y)
            + ((vs(uvec3(upper_lower.x, upper.y, upper.z)) - vs(uvec3(lower_lower.x, upper.y, upper.z))) * (1.0 - factor.x)
            + (vs(uvec3(upper_upper.x, upper.y, upper.z)) - vs(uvec3(lower_upper.x, upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.y =
          (((vs(uvec3(lower.x, upper_lower.y, lower.z)) - vs(uvec3(lower.x, lower_lower.y, lower.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, lower.z)) - vs(uvec3(upper.x, lower_lower.y, lower.z))) * factor.x) * (1.0 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, lower.z)) - vs(uvec3(lower.x, lower_upper.y, lower.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, lower.z)) - vs(uvec3(upper.x, lower_upper.y, lower.z))) * factor.x) * factor.y) * (1.0 - factor.z)
        + (((vs(uvec3(lower.x, upper_lower.y, upper.z)) - vs(uvec3(lower.x, lower_lower.y, upper.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper_lower.y, upper.z)) - vs(uvec3(upper.x, lower_lower.y, upper.z))) * factor.x) * (1.0 - factor.y)
        + ((vs(uvec3(lower.x, upper_upper.y, upper.z)) - vs(uvec3(lower.x, lower_upper.y, upper.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper_upper.y, upper.z)) - vs(uvec3(upper.x, lower_upper.y, upper.z))) * factor.x) * factor.y) * factor.z;

    gradient.z =
          (((vs(uvec3(lower.x, lower.y, upper_lower.z)) - vs(uvec3(lower.x, lower.y, lower_lower.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_lower.z)) - vs(uvec3(upper.x, lower.y, lower_lower.z))) * factor.x) * (1.0 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_lower.z)) - vs(uvec3(lower.x, upper.y, lower_lower.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_lower.z)) - vs(uvec3(upper.x, upper.y, lower_lower.z))) * factor.x) * factor.y) * (1.0 - factor.z)
        + (((vs(uvec3(lower.x, lower.y, upper_upper.z)) - vs(uvec3(lower.x, lower.y, lower_upper.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, lower.y, upper_upper.z)) - vs(uvec3(upper.x, lower.y, lower_upper.z))) * factor.x) * (1.0 - factor.y)
        + ((vs(uvec3(lower.x, upper.y, upper_upper.z)) - vs(uvec3(lower.x, upper.y, lower_upper.z))) * (1.0 - factor.x)
        + (vs(uvec3(upper.x, upper.y, upper_upper.z)) - vs(uvec3(upper.x, upper.y, lower_upper.z))) * factor.x) * factor.y) * factor.z;

    return gradient * vec3(volDim.x / volSize.x, volDim.y / volSize.y, volDim.z / volSize.z) * (0.5f * 0.00003051944088f);


}



float interpVol(vec3 pos, inout bool interpolated)
{
    // scaled pos is in voxel index space
    //vec3 scaled_pos = vec3((pos.x * volSize.x / volDim.x) - 0.5f, (pos.y * volSize.y / volDim.y) - 0.5f, (pos.z * volSize.z / volDim.z) - 0.5f);
    ivec3 base = ivec3(floor(pos));
    vec3 factor = fract(pos);
    ivec3 lower = max(base, ivec3(0));
    ivec3 upper = min(base + ivec3(1), ivec3(volSize) - ivec3(1));
    ivec4 tData = imageLoad(volumeData, ivec3(base));
    //ivec4 tData = texture(volumeDataTexture, base);
    if (tData.y > 0)
    {
        interpolated = true;
    }
    else
    {
        interpolated = false;
    }

    //return float(tData.x);
    return (
          ((vs(uvec3(lower.x, lower.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, lower.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, lower.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, lower.z)) * factor.x) * factor.y) * (1 - factor.z)
        + ((vs(uvec3(lower.x, lower.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, lower.y, upper.z)) * factor.x) * (1 - factor.y)
         + (vs(uvec3(lower.x, upper.y, upper.z)) * (1 - factor.x) + vs(uvec3(upper.x, upper.y, upper.z)) * factor.x) * factor.y) * factor.z
        ) * 0.00003051944088f;
}

float SDF(vec3 location)
{
    float i, j, k;
    float x, y, z;

    x = modf(location.x, i);
    y = modf(location.y, j);
    z = modf(location.z, k);

    int I = int(i);
    int J = int(j);
    int K = int(k);

    float N0 = imageLoad(volumeData, ivec3(I,           J,          K)).x * 0.00003051944088f;
    float N1 = imageLoad(volumeData, ivec3(I,           J + 1.0,    K)).x * 0.00003051944088f;
    float N2 = imageLoad(volumeData, ivec3(I + 1.0,     J,          K)).x * 0.00003051944088f;
    float N3 = imageLoad(volumeData, ivec3(I + 1.0,     J + 1.0,    K)).x * 0.00003051944088f;
                                                                    
    float N4 = imageLoad(volumeData, ivec3(I,           J,          K + 1)).x * 0.00003051944088f;
    float N5 = imageLoad(volumeData, ivec3(I,           J + 1.0,    K + 1)).x * 0.00003051944088f;
    float N6 = imageLoad(volumeData, ivec3(I + 1.0,     J,          K + 1)).x * 0.00003051944088f;
    float N7 = imageLoad(volumeData, ivec3(I + 1.0,     J + 1.0,    K + 1)).x * 0.00003051944088f;

    float a0, a1, b0, b1;

    a0 = N0 * (1.0 - z) + N4 * z;
    a1 = N1 * (1.0 - z) + N5 * z;
    b0 = N2 * (1.0 - z) + N6 * z;
    b1 = N3 * (1.0 - z) + N7 * z;

    return (a0 * (1.0 - y) + a1 * y) * (1.0 - x) + (b0 * (1.0 - y) + b1 * y) * x;
}


float SDFGradient(vec3 location, vec3 location_offset)
{

    //vec3 location_offset = vec3(0, 0, 0);
    //location_offset(dim) = stepSize;

    float upperVal = SDF(location.xyz + location_offset);
    float lowerVal = SDF(location.xyz - location_offset);
    float gradient = (upperVal - lowerVal) / (2.0f * (volDim.x / volSize.x));
    return gradient;



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
                        if (volume < 0.00001)
                        {
                            return float (data.x * 0.00003051944088f);
                        }
                        w = 1.0f / volume;
                        w_sum += float(w);
                        sum_d += float(w) * float(data.x * 0.00003051944088f);
                    }
                
            }
        }
    }
    return sum_d / w_sum;
}

float[6] getJ(vec3 dsdf, float[3][6] dxdxi)
{
    float oJ[6];
    oJ[0] = dsdf.x * dxdxi[0][0] + dsdf.y * dxdxi[1][0] + dsdf.z * dxdxi[2][0];
    oJ[1] = dsdf.x * dxdxi[0][1] + dsdf.y * dxdxi[1][1] + dsdf.z * dxdxi[2][1];
    oJ[2] = dsdf.x * dxdxi[0][2] + dsdf.y * dxdxi[1][2] + dsdf.z * dxdxi[2][2];
    oJ[3] = dsdf.x * dxdxi[0][3] + dsdf.y * dxdxi[1][3] + dsdf.z * dxdxi[2][3];
    oJ[4] = dsdf.x * dxdxi[0][4] + dsdf.y * dxdxi[1][4] + dsdf.z * dxdxi[2][4];
    oJ[5] = dsdf.x * dxdxi[0][5] + dsdf.y * dxdxi[1][5] + dsdf.z * dxdxi[2][5];
    return oJ;
}


void getPartialDerivative(vec3 cameraPoint, vec3 pos, inout bool isInterpolated, inout float SDF_derivative[6], inout float sdf_val)
{
    vec3 current_world_point;
    vec3 plus_h_world_point;
    vec3 minus_h_world_point;
    vec3 plus_h_camera_point;
    vec3 minus_h_camera_point;
    vec3 plus_h_voxel_point;
    vec3 minus_h_voxel_point;
    float plus_h_sdf_value;
    float minus_h_sdf_value;

    vec3 current_voxel_point = vec3(((pos.x - 0.f) * volSize.x / volDim.x) - 0.5f, ((pos.y- 0.f) * volSize.y / volDim.y) - 0.5f, ((pos.z - 0.f) * volSize.z / volDim.z) - 0.5f);

    sdf_val = interpDistance(current_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;

    if (current_voxel_point.x < 0 || current_voxel_point.y < 0 || current_voxel_point.z < 0)
        return;
    if (current_voxel_point.x > volSize.x || current_voxel_point.y > volSize.y || current_voxel_point.z > volSize.z)
        return;




    //tx derivative
    plus_h_voxel_point = current_voxel_point;
    plus_h_voxel_point.x += 1.0f;
    minus_h_voxel_point = current_voxel_point;
    minus_h_voxel_point.x -= 1.0f;
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[0] = (plus_h_sdf_value - minus_h_sdf_value) /  (2.0f / (volSize.x / volDim.x));

    //ty derivative
    plus_h_voxel_point = current_voxel_point;
    plus_h_voxel_point.y += 1.0f;
    minus_h_voxel_point = current_voxel_point;
    minus_h_voxel_point.y -= 1.0f;
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[1] = (plus_h_sdf_value - minus_h_sdf_value) / (2.0f / (volSize.x / volDim.x));

    //tz derivative
    plus_h_voxel_point = current_voxel_point;
    plus_h_voxel_point.z += 1.0f;
    minus_h_voxel_point = current_voxel_point;
    minus_h_voxel_point.z -= 1.0f;
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[2] = (plus_h_sdf_value - minus_h_sdf_value) / (2.0f / (volSize.x / volDim.x));

    //wx derivative
    plus_h_world_point = (r1p * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    minus_h_world_point = (r1m * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    plus_h_voxel_point = vec3((plus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (plus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (plus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    minus_h_voxel_point = vec3((minus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (minus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (minus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[3] = (plus_h_sdf_value - minus_h_sdf_value) /  (2.0f / (volSize.x / volDim.x)); // 0.01 = w_h?

    //wy derivative
    plus_h_world_point = (r2p * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    minus_h_world_point = (r2m * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    plus_h_voxel_point = vec3((plus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (plus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (plus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    minus_h_voxel_point = vec3((minus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (minus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (minus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[4] = (plus_h_sdf_value - minus_h_sdf_value) /  (2.0f / (volSize.x / volDim.x)); // 0.01 = w_h?

    //wz derivative
    plus_h_world_point = (r3p * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    minus_h_world_point = (r3m * cameraPoint) + vec3(Ttrack[3][0], Ttrack[3][1], Ttrack[3][2]);
    plus_h_voxel_point = vec3((plus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (plus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (plus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    minus_h_voxel_point = vec3((minus_h_world_point.x * volSize.x / volDim.x) - 0.5f, (minus_h_world_point.y * volSize.y / volDim.y) - 0.5f, (minus_h_world_point.z * volSize.z / volDim.z) - 0.5f);
    plus_h_sdf_value = interpDistance(plus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    minus_h_sdf_value = interpDistance(minus_h_voxel_point, isInterpolated);
    if (!isInterpolated)
        return;
    SDF_derivative[5] = (plus_h_sdf_value - minus_h_sdf_value) / (2.0f / (volSize.x / volDim.x)); // 0.01 = w_h?

}

void main()
{

    uvec2 pix = gl_GlobalInvocationID.xy;
    //ivec2 inSize = imageSize(inVertex); // mipmapped sizes

    if (pix.x >= 0 && pix.x < imageSize.x - 1 && pix.y >= 0 && pix.y < imageSize.y)
    {
        setMats();
        //vec4 currentPoint = imageLoad(inVertex, ivec2(pix)); // in camera space
        vec4 cameraPoint = imageLoad(inVertex, ivec2(pix));
        vec4 projectedVertex = Ttrack * vec4(cameraPoint.xyz, 1.0f);

        //float D = interpVol(projectedVertex.xyz);// get SDF value from 3D volume texture
                                                 // this is 3d point in world space not volume space

        bool isInterpolated;
        // float sdf_der[6] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
        //  float D;
        //  getPartialDerivative(cameraPoint.xyz, projectedVertex.xyz, isInterpolated, sdf_der, D);
        //  vec3 dSDF_dx = vec3(sdf_der[0], sdf_der[1], sdf_der[2]);

        vec3 cvp = vec3(((projectedVertex.x) * volSize.x / volDim.x) - 0.5f, ((projectedVertex.y) * volSize.y / volDim.y) - 0.5f, ((projectedVertex.z) * volSize.z / volDim.z)) - 0.5f;
        float D = SDF(cvp);

        //float Dup = SDF(cvp + vec3(0,0,1));
        vec3 dSDF_dx = vec3(SDFGradient(cvp, vec3(1, 0, 0)), SDFGradient(cvp, vec3(0, 1, 0)), SDFGradient(cvp, vec3(0, 0, 1)));

        //vec3 dSDF_dx = getGradient(projectedVertex);

        //imageStore(testImage, ivec2(pix), vec4(tvec1.x - dSDF_dx[0] * 1.0, tvec1.y - dSDF_dx[1] * 1.0, tvec1.z - dSDF_dx[2] * 1.0, 1.0f));


        //ivec4 tempVal = texture(volumeDataTexture, vec3(cvp.x / 128.0f, cvp.y / 128.0f, cvp.z / 128.0f));
        //D = tempVal.x * 0.00003051944088f;
        //imageStore(testImage, ivec2(pix), vec4(sdf_der[3] * 0.1f, sdf_der[4] * 0.1f, sdf_der[5] * 0.1f, 1.0f));
        //imageStore(testImage, ivec2(pix), vec4(sdf_der[0] * 1.0f, sdf_der[1] * 1.0f, sdf_der[2] * 1.0f, 1.0f));
        //imageStore(testImage, ivec2(pix), vec4(cameraPoint.z * 1.0f, cameraPoint.z * 1.0f, cameraPoint.z * 1.0f, 1.0f));
        //imageStore(testImage, ivec2(pix), vec4(Dup, Dup, Dup, 1.0f));

        float absD = abs(D);// get abs depth
        if (D < 1.0 && D > -1.0)
        {
            //vec3 testGrad = getGradient(projectedVertex);
            if (true)
            {
                vec3 nDSDF = normalize(dSDF_dx);
                //  vec3 dSDF_dx = vec3(testGrad.x, testGrad.y, testGrad.z);
                //imageStore(testImage, ivec2(pix), vec4(sdf_der[0] * 1.0f, sdf_der[1] * 1.0f, sdf_der[2] * 1.0f, 1.0f));
                imageStore(testImage, ivec2(pix), vec4(nDSDF.xy, -nDSDF.z, 1.0f));

                //   // 3 cols 6 rows 
                float dx_dxi[3][6];

                dx_dxi[0][0] = 0;                   dx_dxi[1][0] = -projectedVertex.z;  dx_dxi[2][0] = projectedVertex.y;    
                dx_dxi[0][1] = projectedVertex.z;   dx_dxi[1][1] = 0;                   dx_dxi[2][1] = -projectedVertex.x; 
	            dx_dxi[0][2] = -projectedVertex.y;  dx_dxi[1][2] = projectedVertex.x;   dx_dxi[2][2] = 0;  
                dx_dxi[0][3] = 1;	                dx_dxi[1][3] = 0;                   dx_dxi[2][3] = 0;
                dx_dxi[0][4] = 0;	                dx_dxi[1][4] = 1;                   dx_dxi[2][4] = 0;
                dx_dxi[0][5] = 0;	                dx_dxi[1][5] = 0;                   dx_dxi[2][5] = 1;

                float J[6] = getJ(dSDF_dx, dx_dxi);

                //float huber = 1.0f;
                //if (absD<0.0005)
                //{
                //    huber = 1.0f;
                //}
                //else if (absD > 0.0005 && absD <= 0.01 )
                //{
                //    huber = 0.0005/absD;
                //}
                //else
                //{
                //    huber = 0.0f;
                //}
                float huber = absD < 0.02f ? 1.0f : 0.02f / absD;

                //imageStore(testImage, ivec2(pix), vec4(J[0] * 1.f, J[1] * 1.0f, J[2] * 1.0f, 1.0f));


//trackOutput[(pix.y * imageSize.x) + pix.x].h = 1;
//trackOutput[(pix.y * imageSize.x) + pix.x].D = D;
//trackOutput[(pix.y * imageSize.x) + pix.x].J = sdf_der;

                trackOutput[(pix.y * imageSize.x) + pix.x].h = huber;
                trackOutput[(pix.y * imageSize.x) + pix.x].D = D;
                trackOutput[(pix.y * imageSize.x) + pix.x].J = J;

            
            }
            else
            {
                    imageStore(testImage, ivec2(pix), vec4(0,0,0, 1.0f));

           // imageStore(testImage, ivec2(pix), vec4(0.0f, 1.0f, 0.0f, 1.0f));

                float J0[6] = { 0, 0, 0, 0, 0, 0 };
                trackOutput[(pix.y * imageSize.x) + pix.x].h = 0;
                trackOutput[(pix.y * imageSize.x) + pix.x].D = 0;
                trackOutput[(pix.y * imageSize.x) + pix.x].J = J0;
            }
        }
        else
        {
               imageStore(testImage, ivec2(pix), vec4(0,0,0, 1.0f));

           // imageStore(testImage, ivec2(pix), vec4(0.0f, 1.0f, 0.0f, 1.0f));

                float J0[6] = { 0, 0, 0, 0, 0, 0 };
                trackOutput[(pix.y * imageSize.x) + pix.x].h = 0;
                trackOutput[(pix.y * imageSize.x) + pix.x].D = 0;
                trackOutput[(pix.y * imageSize.x) + pix.x].J = J0;
        }
       
    
           
    }





}




