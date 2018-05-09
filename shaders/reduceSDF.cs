#version 430
// blockDim == local_size
// gridDim == number of work groups
layout(local_size_x = 112, local_size_y = 1) in; // 

uniform ivec2 imageSize; 

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

layout(std430, binding = 13) buffer OutputData
{
    float outputData [];
};


shared float S[112][32];


void main()
{
    uint sline = gl_LocalInvocationID.x; // 0 - 111

    float sums[32];
    // sums[0] reserved
    // sums[1] - sums[6] = g0 - g5
    // sums[7] - sums[27] = A00 - A55

    for (int i = 0; i < 32; ++i)
    {
        sums[i] = 0.0f;
    }

    for (uint y = gl_WorkGroupID.x; y < imageSize.y; y += gl_NumWorkGroups.x) // y = (0:8); y < 424; y += 8
    {
        for (uint x = sline; x < imageSize.x; x += gl_WorkGroupSize.x) // x = (0:112); x < 512; x += 112
        {
            reduSDFType row = trackOutput[(y * imageSize.x) + x];

            //// GAUSS NEWTON approximation to hessian
            //float T1[6][6] = getT1(row.h, row.J);
            //float T2[6] = getT2(row.h, row.J, row.D);

            sums[0] += row.D * row.D;

            for (int i = 0; i< 6; ++i)
            {
                sums[i + 1] += row.h * row.J[i] * row.D;
            }

            // Error part
            sums[0] += row.D * row.D;

            sums[7] += row.h * row.J[0] * row.J[0];
            sums[8] += row.h * row.J[0] * row.J[1];
            sums[9] += row.h * row.J[0] * row.J[2];
            sums[10] += row.h * row.J[0] * row.J[3];
            sums[11] += row.h * row.J[0] * row.J[4];
            sums[12] += row.h * row.J[0] * row.J[5];

            sums[13] += row.h * row.J[1] * row.J[1];
            sums[14] += row.h * row.J[1] * row.J[2];
            sums[15] += row.h * row.J[1] * row.J[3];
            sums[16] += row.h * row.J[1] * row.J[4];
            sums[17] += row.h * row.J[1] * row.J[5];

            sums[18] += row.h * row.J[2] * row.J[2];
            sums[19] += row.h * row.J[2] * row.J[3];
            sums[20] += row.h * row.J[2] * row.J[4];
            sums[21] += row.h * row.J[2] * row.J[5];

            sums[22] += row.h * row.J[3] * row.J[3];
            sums[23] += row.h * row.J[3] * row.J[4];
            sums[24] += row.h * row.J[3] * row.J[5];

            sums[25] += row.h * row.J[4] * row.J[4];
            sums[26] += row.h * row.J[4] * row.J[5];

            sums[27] += row.h * row.J[5] * row.J[5];

            sums[28] += 1.0f;


        }
    }

    for (int i = 0; i < 32; ++i)
    {
        S[sline][i] = sums[i];
    }

    barrier(); // wait for threads to finish

    if (sline < 32)
    {
        for (uint i = 1; i < gl_WorkGroupSize.x; ++i)
        {
            S[0][sline] += S[i][sline];
        }

        outputData[sline + gl_WorkGroupID.x * 32] = S[0][sline];
        //imageStore(outputData, ivec2(sline, gl_WorkGroupID.x), vec4(S[0][sline]));

    }



}


