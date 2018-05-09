#version 430
// blockDim == local_size
// gridDim == number of work groups
layout(local_size_x = 112, local_size_y = 1) in; // 

struct reduType
{
    int result;
    float error;
    float J[6];
};
layout(std430, binding = 0) buffer TrackData
{
    reduType trackOutput [];
};

//layout(binding = 0, r32f) uniform image2D outputData;

layout(std430, binding = 1) buffer OutputData
{
    float outputData [];
};

uniform ivec2 imageSize; 
shared float S[112][32];

void main()
{
    

    uint sline = gl_LocalInvocationID.x; // 0 - 111

    float sums[32];
    for (int i = 0; i < 32; ++i) 
    { 
        sums[i] = 0.0f;
    }

    // float * jtj = sums + 7; sums[7] = jtj;
    // float * info = sums + 28; sums[28] = info; 

    for (uint y = gl_WorkGroupID.x; y < imageSize.y; y += gl_NumWorkGroups.x) // y = (0:8); y < 424; y += 8
    {
        for (uint x = sline; x < imageSize.x; x += gl_WorkGroupSize.x) // x = (0:112); x < 512; x += 112
        {
            reduType row = trackOutput[(y * imageSize.x) + x];
            if (row.result < 1)
            {
                if (row.result == -4)
                {
                    sums[28] += 1;
                }
                if (row.result == -5)
                {
                    sums[28] += 1;
                }
                if (row.result > -4)
                {
                    sums[28] += 1;
                }
                continue;
            }

            // Error part
            sums[0] += row.error * row.error;

            // JTe part
            for (int i = 0; i < 6; ++i)
            {
                sums[i + 1] += row.error * row.J[i];
            }

            // JTJ part, unfortunatly the double loop is not unrolled well...
            sums[7] += row.J[0] * row.J[0];
            sums[8] += row.J[0] * row.J[1];
            sums[9] += row.J[0] * row.J[2];
            sums[10] += row.J[0] * row.J[3];
            sums[11] += row.J[0] * row.J[4];
            sums[12] += row.J[0] * row.J[5];

            sums[13] += row.J[1] * row.J[1];
            sums[14] += row.J[1] * row.J[2];
            sums[15] += row.J[1] * row.J[3];
            sums[16] += row.J[1] * row.J[4];
            sums[17] += row.J[1] * row.J[5];

            sums[18] += row.J[2] * row.J[2];
            sums[19] += row.J[2] * row.J[3];
            sums[20] += row.J[2] * row.J[4];
            sums[21] += row.J[2] * row.J[5];

            sums[22] += row.J[3] * row.J[3];
            sums[23] += row.J[3] * row.J[4];
            sums[24] += row.J[3] * row.J[5];

            sums[25] += row.J[4] * row.J[4];
            sums[26] += row.J[4] * row.J[5];

            sums[27] += row.J[5] * row.J[5];

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
        for(uint i = 1; i < gl_WorkGroupSize.x; ++i)
        {
            S[0][sline] += S[i][sline];
        }

        outputData[sline + gl_WorkGroupID.x * 32] = S[0][sline];
        //imageStore(outputData, ivec2(sline, gl_WorkGroupID.x), vec4(S[0][sline]));

    }



}