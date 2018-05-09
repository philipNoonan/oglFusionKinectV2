#version 430

layout (local_size_x = 32, local_size_y = 32) in;

layout(binding=0, r32f) uniform image2D InputImage;
layout(binding=1, r32f) uniform image2D OutputImage;


float checkPixel(vec4 imageValue)
{
   if (imageValue.x > 5.0f && imageValue.x < 4000.0f)
    {
        return imageValue.x;
    }
   else
    {

        return 0.0f;
    }
}


void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    pix.y = pix.y += 2;
    ivec2 size = imageSize(InputImage);
    float validPixels = 9.0f;
    float depthSum = 0.0f;
    float meanDepth;
    float currDepth;

    if (pix.x > 1 && pix.x < size.x - 1 && pix.y > 1 && pix.y < size.y - 1)
    {

        // for each pixel in the sub image, check if depth is valie (i.e. within range 0.005 -> 4.0)
        // if not in range, substract the number of valid pixels count so that average is valid

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                float tDepth = checkPixel(imageLoad(InputImage, ivec2(pix.x + i - 1, pix.y + j - 1)));
                if (tDepth == 0.0f)
                {
                    validPixels--;
                }
                else
                {
                    depthSum += tDepth;
                }
                if (i == 1 && j == 1)
                {
                    currDepth = tDepth;
                }
            }
        }

        if (validPixels > 6.0f)
        {
            meanDepth = depthSum / validPixels;
        }
        else
        {
            meanDepth = 0.0f;
        }
        
        if (meanDepth > 0.0f && abs(currDepth - meanDepth) < 5.0f)
        {
            imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(currDepth, 0.0f, 0.0f, 0.0f)); // make this a -2 value to skip???
        }
        else
        {
            imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(0.0f, 0.0f, 0.0f, 0.0f));
        }





    }




}
