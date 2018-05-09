#version 430

layout(local_size_x = 5, local_size_y = 5) in;

layout(binding= 0, rgba8) uniform image2D ColorTexturePrevious;
layout(binding= 1, rgba8) uniform image2D ColorTextureCurrent;


layout(std430, binding= 0) buffer pos3D // ouput
{
    vec4 Position3D [];
};

layout(std430, binding= 2) buffer pos3Dprevious // previous
{
    vec4 Position3D_prev [];
};

// here we want to copy into a shared local group memory and if a vertex is greatly different, we want to make it the average position and color of its neighbours

shared float localData[gl_WorkGroupSize.x][gl_WorkGroupSize.y];
shared int localDataColor[gl_WorkGroupSize.x][gl_WorkGroupSize.y];

vec4 previousVert;
uvec2 pix;
ivec2 size;
vec4 mColor;


void applyFilter()
{
    float sdepth = 0;
    for (int i = 0; i < gl_WorkGroupSize.x; i++)
    {
        for (int j = 0; j < gl_WorkGroupSize.y; j++)
        {
            sdepth += localData[i][j];
        }
    }

    float mDepth = sdepth / (gl_WorkGroupSize.x * gl_WorkGroupSize.y);

    float dif = 0.0f;
    for (int k = 0; k < 9; k++)
    {
        dif += pow(localData[gl_LocalInvocationID.x][gl_LocalInvocationID.y] - mDepth, 2.0f);
    }

    float stdDev = sqrt(dif / 8.0f); // 8 is N - 1

    if (stdDev > 10.0f)
    {
        // 
        // get the mean color of the local -1 + 1 pixel
        localDataColor[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = 0;
        //imageStore(ColorTextureCurrent, ivec2(pix.x, pix.y), mColor);
    }
    else
    {
        localDataColor[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = 1;
    }

}

void getMeanColor()
{

}

void main()
{
    pix = gl_GlobalInvocationID.xy;
    size = ivec2(1920, 1080);
    pix.y = pix.y += 2;


    if (pix.x > 5 && pix.x < size.x - 5 && pix.y > 5 && pix.y < size.y - 5)
    {

        ////outputVert = Position3D[(pix.y * size.x) + pix.x];
        //vec4 verts[9];
        //int k = 0;
        //int kSize = 3;
        //for (int i = -kSize; i <= kSize; i+= kSize)
        //{
        //    for (int j = -kSize; j <= kSize; j+= kSize)
        //    {
        //        verts[k] = Position3D_prev[((pix.y + j) * size.x) + (pix.x + i)];
        //        k++;
        //    }
        //}
        
        //for (int i = 0; i < 9; i++)
        //{
        //    if (distance(verts[i], verts[4]) > 10.0f)
        //    {
        //        Position3D[(pix.y * size.x) + pix.x] = verts[8];
        //        vec4 tCol = imageLoad(ColorTexturePrevious, ivec2(pix));
        //        imageStore(ColorTextureCurrent, ivec2(pix.x, pix.y), tCol);


        //    }
        //}



        previousVert = Position3D_prev[(pix.y * size.x) + pix.x];




        localData[gl_LocalInvocationID.x][gl_LocalInvocationID.y] = previousVert.z;

        barrier();

        applyFilter();

        barrier();

        float nValidPixels = 0.0f;

        for (int i = -1; i < 2; i++)
        {
            for (int j = -1; j < 2; j++)
            {
                if (localDataColor[gl_LocalInvocationID.x + i][gl_LocalInvocationID.y + j] != 0)
                {
                    nValidPixels++;
                    mColor += imageLoad(ColorTexturePrevious, ivec2(pix.x + i, pix.y + j));
                }
            }
        }

        if (localDataColor[gl_LocalInvocationID.x][gl_LocalInvocationID.y] == 0)
        {
            if (nValidPixels > 0)
            {
                mColor = mColor / nValidPixels;
            }
            else
            {
                //mColor = mColor / 9.0f;

                mColor = vec4(1.0f, 1.0f, 0.0f, 0.0f);
            }
            imageStore(ColorTextureCurrent, ivec2(pix.x, pix.y), mColor);

        }





    }

}



