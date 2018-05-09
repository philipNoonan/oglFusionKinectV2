#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(binding= 0, rgba32f) uniform image2D InputImage;
layout(binding= 1, rgba32f) uniform image2D OutputImage;
    
    
void main()
{
    uvec2 pix = gl_GlobalInvocationID.xy;
    ivec2 size = imageSize(InputImage);

    //if (pix.x < size.x / 2 && pix.y < size.y / 2)
    //{
    //    imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(0.8f, 0.0f, 0.5f, 1.0f));

    //}

    if (pix.x < size.x && pix.y < size.y)
    {


        vec4 left = imageLoad(InputImage, ivec2(max(pix.x - 1, 0), pix.y));
        vec4 right = imageLoad(InputImage, ivec2(min(pix.x + 1, size.x - 1), pix.y));
        vec4 up = imageLoad(InputImage, ivec2(pix.x, max(pix.y - 1, 0)));
        vec4 down = imageLoad(InputImage, ivec2(pix.x - 1, min(pix.y + 1, size.y - 1)));

        //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(1.0f, 1.0f, 0.5f, 1.0f));

        if (left.z == 0 || right.z == 0 || up.z == 0 || down.z == 0)
        {
            imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(2.0f, 0.0f, 0.0f, 0.0f));

            //    //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(2.0f, 0.0f, 0.0f, 0.0f));
        }
        else
        {
            vec3 dxv = right.xyz - left.xyz;
            vec3 dyv = down.xyz - up.xyz;
            imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(normalize(cross(dyv, dxv)), 1.0f));

            //imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(1.0f, 0.5f, 0.5f, 1.0f));
        }

    }

    //}
    //else
    //{
    //    imageStore(OutputImage, ivec2(pix.x, pix.y), vec4(0.1f, 1.0f, 0.5f, 1.0f));

    //}



}
