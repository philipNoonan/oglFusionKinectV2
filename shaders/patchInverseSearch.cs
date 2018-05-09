#version 430

layout(local_size_x = 8) in;

layout(binding = 0) uniform sampler2D tex_I0;
layout(binding = 1) uniform sampler2D tex_I1;

layout(binding = 0, rg32f) uniform image2D im_UV_x_y;
layout(binding = 1, rg32f) uniform image2D im_S_x_y;

layout(binding = 2, rg32f) uniform image2D im_grad_I0_x_y;
layout(binding = 3, rgba32f) uniform image2D im_prod_I0_xx_yy_xy;
layout(binding = 4, rg32f) uniform image2D im_sum_I0_x_y;

uniform int patch_stride = 4;
uniform int patch_size = 8;

subroutine void launchSubroutine();
subroutine uniform launchSubroutine patchInverseSubroutine;




float computeSSDMeanNorm()
{

    float sum_square_diff = 0.0f;
    float sum_diff = 0.0f;

    // process 16 patches per workgroup, 1 per invocation

    int xOffset = int(gl_LocalInvocationID.x) * patch_size / 2;
    int yOffset = int(gl_LocalInvocationID.y) * patch_size / 2;

    // loops offest by 4 (0-8, 4-12, 8-16, 12-20)
    for (int i = xOffset; i < xOffset + patch_size; ++i)
    {
        for (int j = yOffset; j < yOffset + patch_size; ++j)
        {
            float diff = y_data[i - xOffset][j - yOffset] - x_data[i][j];
            sum_diff += diff;
            sum_square_diff += diff * diff;
        }
    }

    return sum_square_diff - sum_diff * sum_diff / float(patch_size * patch_size);

}

float processPatchMeanNorm(inout float dst_dUx, inout float dst_dUy, float x_grad_sum, float y_grad_sum)
{
    float n = float(patch_size * patch_size);
    float sum_square_diff = 0.0f;
    float sum_diff = 0.0f;
    float diff;

    float sum_I0x_mul = 0.0f;
    float sum_I0y_mul = 0.0f;

    // here we need to process the gradient full size images
    int xOffset = int(gl_LocalInvocationID.x) * patch_size / 2;
    int yOffset = int(gl_LocalInvocationID.y) * patch_size / 2;


    for (int i = xOffset; i < xOffset + patch_size; ++i)
    {
        for (int j = yOffset; j < yOffset + patch_size; ++j)
        {
            diff = y_data[i - xOffset][j - yOffset] - x_data[i][j]; //SWAPED THESE AROUND
            sum_diff += diff;
            sum_square_diff += diff * diff;

            sum_I0x_mul += diff * x_grad_data[i][j];
            sum_I0y_mul += diff * y_grad_data[i][j];

        }
    }



    dst_dUx = sum_I0x_mul - sum_diff * x_grad_sum / n;
    dst_dUy = sum_I0y_mul - sum_diff * y_grad_sum / n;
    return sum_square_diff - sum_diff * sum_diff / n;
}





subroutine(launchSubroutine)
void patchInverseFwd1()
{
    ivec2 denseSize = ivec2(imageSize(im_UV_x_y).xy);
    ivec2 sparseSize = ivec2(imageSize(im_S_x_y).xy);

    int id = int(gl_GlobalInvocationID.y);
    int is = id / 8;

    if (id >= (denseSize.y)) return;

    int i = is * patch_stride;
    int j = 0;

    int psz = patch_size;
    int psz2 = psz / 2;

    vec2 prev_flow_UxUy = imageLoad(im_UV_x_y, ivec2(j + psz2, i + psz2)).xy;

    imageStore(im_S_x_y, ivec2(0, is), vec4(prev_flow_UxUy, 0, 0));
    j += patch_stride;

    for (int js = 1; js < sparseSize.x; js++, j += patch_stride)
    {
        float min_SSD, cur_SSD;

        vec2 flow_UxUy = imageLoad(im_UV_x_y, ivec2(j + psz2, i + psz2)).xy;





    }
}
subroutine(launchSubroutine)
void patchInverseFwd2()
{

}

subroutine(launchSubroutine)
void patchInverseBwd2()
{

}

subroutine(launchSubroutine)
void patchInverseBwd2()
{

}

void main()
{
    patchInverseSubroutine();
}