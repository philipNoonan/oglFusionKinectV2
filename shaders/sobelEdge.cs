#version 430

layout(local_size_x = 32, local_size_y = 32) in;

uniform float EdgeThreshold = 0.1;

uniform int imageType = 0;
uniform int level;

subroutine void launchSubroutine();
subroutine uniform launchSubroutine sobelSubroutine;

// PLEASE REMEMBER THE DOUBLE LINE PROBLEM https://docs.opencv.org/3.2.0/d5/d0f/tutorial_py_gradients.html



float luminance(vec3 color)
{
    //return 0.2126 * float(color.x) / 255.0f + 0.7152 * float(color.y) / 255.0f + 0.0722 * float(color.z) / 255.0f;
    return 0.299 * float(color.x) + 0.587 * float(color.y) + 0.114 * float(color.z);
   // return float(color.x) + float(color.y) + float(color.z);

}

shared float localData[gl_WorkGroupSize.x + 2][gl_WorkGroupSize.y + 2];
shared float localDataY[gl_WorkGroupSize.x + 2][gl_WorkGroupSize.y + 2];

layout(binding = 0) uniform sampler2D tex_I0;

layout(binding= 0, rgba8) uniform image2D InputImgUI;
layout(binding= 1, rg32f) uniform image2D InputImgF;
layout(binding= 2, rg16i) uniform iimage2D InputImgI;


layout(binding= 7, rg32f) uniform image2D inputGradientXY;



layout(binding= 3, rgba8) uniform image2D OutputImg;
layout(binding= 4, rg32f) uniform image2D outputGradientXY;
layout(binding= 5, rg32f) uniform image2DArray outputGradientArray;
layout(binding= 6, r32f) uniform image2D outputWeight;

//layout(binding= 3, r16i) uniform iimage2D outputGradientY;


subroutine(launchSubroutine)
void applyFilter()
{
    uvec2 p = gl_LocalInvocationID.xy + uvec2(1, 1);

    float sx = localData[p.x - 1][p.y - 1] + 2 * localData[p.x - 1][p.y] + localData[p.x - 1][p.y + 1] - (localData[p.x + 1][p.y - 1] + 2 * localData[p.x + 1][p.y] + localData[p.x + 1][p.y + 1]);
    float sy = localData[p.x - 1][p.y + 1] + 2 * localData[p.x][p.y + 1] + localData[p.x + 1][p.y + 1] - (localData[p.x - 1][p.y - 1] + 2 * localData[p.x][p.y - 1] + localData[p.x + 1][p.y - 1]);

    float dist = sx * sx + sy * sy;

    if (dist > EdgeThreshold)
        imageStore(OutputImg, ivec2(gl_GlobalInvocationID.xy), vec4(1.0));
    else
        imageStore(OutputImg, ivec2(gl_GlobalInvocationID.xy), vec4(0, 0, 0, 1));
}

subroutine(launchSubroutine)
void getGradients()  
{ 
    uvec2 p = gl_LocalInvocationID.xy + uvec2(1, 1);
    float lesser = 3.0f;
    float upper = 10.0f;

    float sx = - (lesser * localData[p.x - 1][p.y - 1] + upper * localData[p.x - 1][p.y] + lesser * localData[p.x - 1][p.y + 1]) + (lesser * localData[p.x + 1][p.y - 1] + upper * localData[p.x + 1][p.y] + lesser * localData[p.x + 1][p.y + 1]);
    float sy =  (lesser * localData[p.x - 1][p.y + 1] + upper * localData[p.x][p.y + 1] + lesser * localData[p.x + 1][p.y + 1]) - (lesser * localData[p.x - 1][p.y - 1] + upper * localData[p.x][p.y - 1] + lesser * localData[p.x + 1][p.y - 1]);

  //  float sx = (3.0 * localData[p.x + 1][p.y - 1] + 10.0 * localData[p.x + 1][p.y] + 3.0 * localData[p.x + 1][p.y + 1]) - (3.0 * localData[p.x - 1][p.y - 1] + 10.0 * localData[p.x - 1][p.y] + 3.0 * localData[p.x - 1][p.y + 1]);
 //   float sy = (3.0 * localData[p.x - 1][p.y + 1] + 10.0 * localData[p.x][p.y + 1] + 3.0 * localData[p.x + 1][p.y + 1]) - (3.0 * localData[p.x - 1][p.y - 1] + 10.0 * localData[p.x][p.y - 1] + 3.0 * localData[p.x + 1][p.y - 1]);


    if (imageType == 0 || imageType == 4)
    {
        imageStore(outputGradientXY, ivec2(gl_GlobalInvocationID.xy), vec4((sx), sy, 0, 0));

    } 
    else if (imageType == 1)
    {
        imageStore(outputGradientXY, ivec2(gl_GlobalInvocationID.xy), vec4(sx, sy , 0, 0));
        //imageStore(outputGradientXY, ivec2(gl_GlobalInvocationID.xy), ivec4((sx) * 255.0f, sy * 255.0f, 0, 0));

    }
    else if (imageType == 2)
    {
        imageStore(outputGradientArray, ivec3(gl_GlobalInvocationID.xy, 0), vec4(sx, sy, 0, 0));

        //float sxY = -(lesser * localDataY[p.x - 1][p.y - 1] + upper * localDataY[p.x - 1][p.y] + lesser * localDataY[p.x - 1][p.y + 1]) + (lesser * localDataY[p.x + 1][p.y - 1] + upper * localDataY[p.x + 1][p.y] + lesser * localDataY[p.x + 1][p.y + 1]);
        float syY = (lesser * localDataY[p.x - 1][p.y + 1] + upper * localDataY[p.x][p.y + 1] + lesser * localDataY[p.x + 1][p.y + 1]) - (lesser * localDataY[p.x - 1][p.y - 1] + upper * localDataY[p.x][p.y - 1] + lesser * localDataY[p.x + 1][p.y - 1]);

        //float syY = (lesser * localDataY[p.x - 1][p.y + 1] + upper * localDataY[p.x][p.y + 1] + lesser * localDataY[p.x + 1][p.y + 1]) - (lesser * localDataY[p.x - 1][p.y - 1] + upper * localDataY[p.x][p.y - 1] + lesser * localDataY[p.x + 1][p.y - 1]);
        //float syY = (1.0 * localDataY[p.x - 1][p.y + 1] + 2.0 * localDataY[p.x][p.y + 1] + 1.0 * localDataY[p.x + 1][p.y + 1]) - (1.0 * localDataY[p.x - 1][p.y - 1] + 2.0 * localDataY[p.x][p.y - 1] + 1.0 * localDataY[p.x + 1][p.y - 1]);

        // we dont need the first chanel output here
        imageStore(outputGradientArray, ivec3(gl_GlobalInvocationID.xy, 1), vec4(0, syY, 0, 0));

    }
    //imageStore(outputGradientY, ivec2(gl_GlobalInvocationID.xy), ivec4(sy * 32766.0f));

}

subroutine(launchSubroutine)
void getSmoothness()
{
    // [-0.5, 0.0, 0.5]
    // [-0.5
    //   0.0
    //   0.5]
    uvec2 p = gl_LocalInvocationID.xy + uvec2(1, 1);

    float ux = (-0.5f * localData[p.x - 1][p.y] + localData[p.x][p.y] + 0.5f * localData[p.x + 1][p.y]); // CHECK ORDER OF ME
    float uy = (-0.5f * localData[p.x][p.y - 1] + localData[p.x][p.y] + 0.5f * localData[p.x][p.y + 1]);

    float vx = (-0.5f * localDataY[p.x - 1][p.y] + localDataY[p.x][p.y] + 0.5f * localDataY[p.x + 1][p.y]);
    float vy = (-0.5f * localDataY[p.x][p.y - 1] + localDataY[p.x][p.y] + 0.5f * localDataY[p.x][p.y + 1]);

    float weight = 0.25 / (ux * ux + uy * uy + vx * vx + vy * vy + 0.001f * 0.001f);

    imageStore(outputWeight, ivec2(gl_GlobalInvocationID.xy), vec4(weight));
    //imageStore(outputGradientArray, ivec3(gl_GlobalInvocationID.xy, 0), ivec4(sx * 32766.0f, sy * 32766.0f, 0, 0));
    //imageStore(outputGradientArray, ivec3(gl_GlobalInvocationID.xy, 1), ivec4(sxY * 32766.0f, syY * 32766.0f, 0, 0));

}

void loadImage(ivec2 pos, ivec2 localDataLoc)
{ 
    if (imageType == 0) // rgba8ui
    {
        // gauss smoothed
        //localData[localDataLoc.x][localDataLoc.y] = luminance(imageLoad(InputImgUI, pos).xyz);
        localData[localDataLoc.x][localDataLoc.y] = luminance(imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y - 1)).xyz) * 0.077847f + luminance(imageLoad(InputImgUI, ivec2(pos.x, pos.y - 1)).xyz) * 0.123317f + luminance(imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y - 1)).xyz) * 0.077847f +
                                             luminance(imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y)).xyz) * 0.123317f + luminance(imageLoad(InputImgUI, ivec2(pos.x, pos.y)).xyz) * 0.195346f + luminance(imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y)).xyz) * 0.123317f +
                                             luminance(imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y + 1)).xyz) * 0.077847f + luminance(imageLoad(InputImgUI, ivec2(pos.x, pos.y + 1)).xyz) * 0.123317f + luminance(imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y + 1)).xyz) * 0.077847f;

    }
    else if (imageType == 1) // rg float 2 
    {
        vec2 inputVec = imageLoad(InputImgF, pos).xy;  
        localData[localDataLoc.x][localDataLoc.y] = inputVec.x;
        localDataY[localDataLoc.x][localDataLoc.y] = inputVec.y;

    }
    else if (imageType == 2) // rg float 32
    {
        //vec2 inputVec = imageLoad(InputImgF, pos).xy;

        vec2 inputVec = imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y - 1)).xy * 0.077847f + imageLoad(InputImgUI, ivec2(pos.x, pos.y - 1)).xy * 0.123317f + imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y - 1)).xy * 0.077847f +
                        imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y)).xy * 0.123317f + imageLoad(InputImgUI, ivec2(pos.x, pos.y)).xy * 0.195346f + imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y)).xy * 0.123317f +
                        imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y + 1)).xy * 0.077847f + imageLoad(InputImgUI, ivec2(pos.x, pos.y + 1)).xy * 0.123317f + imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y + 1)).xy * 0.077847f;



        localData[localDataLoc.x][localDataLoc.y] = inputVec.x;
        localDataY[localDataLoc.x][localDataLoc.y] = inputVec.y;

    }
    else if (imageType == 3) // rg short 16 i
    {
        ivec2 inputVec = ivec2(imageLoad(InputImgI, pos).xy);
        localData[localDataLoc.x][localDataLoc.y] = float(inputVec.x) * 0.00003051757f;
        localDataY[localDataLoc.x][localDataLoc.y] = float(inputVec.y) * 0.00003051757f;

    }
    else if (imageType == 4) // red float 32
    {
        // gauss smoothed
        localData[localDataLoc.x][localDataLoc.y] = texelFetch(tex_I0, pos, level).x;
        //localData[localDataLoc.x][localDataLoc.y] = (imageLoad(InputImgF, ivec2(pos.x - 1, pos.y - 1)).x) * 0.077847f + (imageLoad(InputImgUI, ivec2(pos.x, pos.y - 1)).x) * 0.123317f + (imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y - 1)).x) * 0.077847f +
        //                                     (imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y)).x) * 0.123317f + (imageLoad(InputImgUI, ivec2(pos.x, pos.y)).x) * 0.195346f + (imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y)).x) * 0.123317f +
        //                                     (imageLoad(InputImgUI, ivec2(pos.x - 1, pos.y + 1)).x) * 0.077847f + (imageLoad(InputImgUI, ivec2(pos.x, pos.y + 1)).x) * 0.123317f + (imageLoad(InputImgUI, ivec2(pos.x + 1, pos.y + 1)).x) * 0.077847f;


    }



}

void main()
{
    uvec2 gSize = gl_WorkGroupSize.xy * gl_NumWorkGroups.xy;

    // Copy into local memory
    loadImage(ivec2(gl_GlobalInvocationID.xy), ivec2(gl_LocalInvocationID.x + 1, gl_LocalInvocationID.y + 1));

    // Handle the edges
    // Bottom edge
    if (gl_LocalInvocationID.y == 0)
    {
        if (gl_GlobalInvocationID.y > 0)
        {
            loadImage(ivec2(gl_GlobalInvocationID.xy + ivec2(0, -1)), ivec2(gl_LocalInvocationID.x + 1, 0));

            // Lower left corner
            if (gl_LocalInvocationID.x == 0)
            {
                if (gl_GlobalInvocationID.x > 0)
                {
                    loadImage(ivec2(gl_GlobalInvocationID.xy + ivec2(-1, -1)), ivec2(0, 0));
                }
                else
                {
                    localData[0][0] = 0.0;
                    localDataY[0][0] = 0.0;
                }
            }
                

            // Lower right corner
            if (gl_LocalInvocationID.x == gl_WorkGroupSize.x - 1)
            {
                if (gl_GlobalInvocationID.x < gSize.x - 1)
                {
                    loadImage(ivec2(gl_GlobalInvocationID.xy + ivec2(1, -1)), ivec2(gl_WorkGroupSize.x + 1, 0));
                }
                else
                {
                    localData[gl_WorkGroupSize.x + 1][0] = 0.0;
                    localDataY[gl_WorkGroupSize.x + 1][0] = 0.0;
                }
            }

        }
        else
        {
            localData[gl_LocalInvocationID.x + 1][0] = 0.0;
            localDataY[gl_LocalInvocationID.x + 1][0] = 0.0;
        }

    }
    // Top edge
    if (gl_LocalInvocationID.y == gl_WorkGroupSize.y - 1)
    {
        if (gl_GlobalInvocationID.y < gSize.y - 1)
        {
            loadImage(ivec2(gl_GlobalInvocationID.xy + ivec2(0, 1)), ivec2(gl_LocalInvocationID.x + 1, gl_WorkGroupSize.y + 1));
            // Upper left corner
            if (gl_LocalInvocationID.x == 0)
            {
                if (gl_GlobalInvocationID.x > 0)
                {
                    loadImage(ivec2(gl_GlobalInvocationID.xy) + ivec2(-1, 1), ivec2(0, gl_WorkGroupSize.y + 1));
                }
                else
                {
                    localData[0][gl_WorkGroupSize.y + 1] = 0.0;
                    localDataY[0][gl_WorkGroupSize.y + 1] = 0.0;
                }
            }

            // Lower right corner
            if (gl_LocalInvocationID.x == gl_WorkGroupSize.x - 1)
            {
                if (gl_GlobalInvocationID.x < gSize.x - 1)
                {
                    loadImage(ivec2(gl_GlobalInvocationID.xy) + ivec2(1, 1), ivec2(gl_WorkGroupSize.x + 1, gl_WorkGroupSize.y + 1));
                }
                else
                {
                    localData[gl_WorkGroupSize.x + 1][gl_WorkGroupSize.y + 1] = 0.0;
                    localDataY[gl_WorkGroupSize.x + 1][gl_WorkGroupSize.y + 1] = 0.0;
                }

            }

        }
        else
        {
            localData[gl_LocalInvocationID.x + 1][gl_WorkGroupSize.y + 1] = 0.0;
            localDataY[gl_LocalInvocationID.x + 1][gl_WorkGroupSize.y + 1] = 0.0;
        }
    }
    // Left edge
    if (gl_LocalInvocationID.x == 0)
    {
        if (gl_GlobalInvocationID.x > 0)
        {
            loadImage(ivec2(gl_GlobalInvocationID.xy) + ivec2(-1, 0), ivec2(0, gl_LocalInvocationID.y + 1));
        }
        else
        {
            localData[0][gl_LocalInvocationID.y + 1] = 0.0;
            localDataY[0][gl_LocalInvocationID.y + 1] = 0.0;
        }

    }
    // Right edge
    if (gl_LocalInvocationID.x == gl_WorkGroupSize.x - 1)
    {
        if (gl_GlobalInvocationID.x < gSize.x - 1)
        {
            loadImage(ivec2(gl_GlobalInvocationID.xy) + ivec2(1, 0), ivec2(gl_WorkGroupSize.x + 1, gl_LocalInvocationID.y + 1));
        }
        else
        {
            localData[gl_WorkGroupSize.x + 1][gl_LocalInvocationID.y + 1] = 0.0;
            localDataY[gl_WorkGroupSize.x + 1][gl_LocalInvocationID.y + 1] = 0.0;
        }
        


    }

    barrier();

    sobelSubroutine();
}