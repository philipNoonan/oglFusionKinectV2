#version 430

layout(local_size_x = 32, local_size_y = 32) in;

// bindings
layout(binding = 0) uniform sampler2D tex_I0;
layout(binding = 1) uniform sampler2D tex_I1;

layout(binding = 0, rg32f) uniform image2D flow_texture_x_y;
layout(binding = 1, rgba32f) uniform image2D delta_flow_texture_u_v; // this image flip flops per compute RG -> BA, allows for read write without overwriting

layout(binding = 2, rgba8) uniform image2DArray im_mix_diff_warp_I;

layout(binding = 3, rg32f) uniform image2DArray im_grads_mix_diff_x_y;
layout(binding = 4, rg32f) uniform image2DArray im_second_grads_mix_diff_x_y;

layout(binding = 5, r32f) uniform image2D im_smoothness_weight; 
layout(binding = 6, rg32f) uniform image2D im_smoothness_term; 

layout(binding = 7, rg32f) uniform image2D total_flow_texture_u_v;


struct dataTerms
{
    float A11;
    float A12;
    float A22;
    float b1;
    float b2;
};
layout(std430, binding = 8) coherent buffer dataTermBuf
{
    dataTerms dataTermBuffer [];
};

float luminance(vec3 color)
{
    //return 0.2126 * float(color.x) / 255.0f + 0.7152 * float(color.y) / 255.0f + 0.0722 * float(color.z) / 255.0f;
    return 0.299 * float(color.x) + 0.587 * float(color.y) + 0.114 * float(color.z);

}

// uniforms
uniform int level;
uniform int flipflop;
uniform int iter;

uniform float zeta_squared = 0.01f;
uniform float epsilon_squared = 0.00001f;
uniform float delta2 = 2.5f;
uniform float gamma2 = 5.0f;

//    variational_refinement_iter = 5;
//    variational_refinement_alpha = 20.f;
//    variational_refinement_gamma = 10.f;
//    variational_refinement_delta = 5.f;
//    fixedPointIterations = 5;
//    sorIterations = 5;
//    alpha = 20.0f;
//    delta = 5.0f;
//    gamma = 10.0f;
//    omega = 1.6f;
//    zeta = 0.1f;
//    epsilon = 0.001f;

// subroutines

subroutine void launchSubroutine();
subroutine uniform launchSubroutine variRefineSubroutine;




// compute color consistency

// compute gradient consitency

// compute smoothness term

// compute data term
//

// shared datas




vec4 averageImage(vec4 warpedPixel, vec4 _t0Pixel)
{
    vec4 avPixel = mix(warpedPixel, _t0Pixel, 0.5f);
    imageStore(im_mix_diff_warp_I, ivec3(gl_GlobalInvocationID.xy, 0), vec4(avPixel.xyz, 1));

    return avPixel;
}

vec4 differenceImage(vec4 warpedPixel, vec4 _t0Pixel)
{
    vec4 subtractPixel = warpedPixel - _t0Pixel;
    imageStore(im_mix_diff_warp_I, ivec3(gl_GlobalInvocationID.xy, 1), vec4(subtractPixel.xyz, 1));

    return subtractPixel;
}

vec4 warpImage(vec2 _pix)
{
    vec4 tempPixel = textureLod(tex_I1, _pix, level);
    imageStore(im_mix_diff_warp_I, ivec3(gl_GlobalInvocationID.xy, 2), vec4(tempPixel.xyz, 1));

    return tempPixel;
}

subroutine(launchSubroutine)
void prepareBuffers()
{
    // here we turn the input color im_I0_I1, and flow image im_Ux_Uy into stuff
    // STEP 1 I1 needs to be warped by flow
    // warpImage(I1, flowUV, I1warped);
    ivec2 imSize = ivec2(imageSize(flow_texture_x_y).xy);
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 pos = ivec2(gl_LocalInvocationID.xy);

    vec2 flowVec = imageLoad(flow_texture_x_y, pix).xy;
    //vec2 flowVec = vec2(0);


    if (pix.x - flowVec.x <= 0 || pix.x + flowVec.x >= imSize.x || pix.y - flowVec.y <= 0 || pix.y + flowVec.y >= imSize.y) return;

    vec4 t0Pixel = textureLod(tex_I0, vec2(float(pix.x) / float(imSize.x), float(pix.y) / float(imSize.y)), level);

    vec4 warpedPixel = warpImage(vec2( (float(pix.x + flowVec.x)  / float(imSize.x) ), (float(pix.y + flowVec.y)  / float(imSize.y))   )     );

    vec4 averagePixel = averageImage(warpedPixel, t0Pixel);

    vec4 differencePixel = differenceImage(warpedPixel, t0Pixel);

    // compute sobel for averagePixel
    // compute sobel for differencePixel

   

} 


subroutine(launchSubroutine)
void computeDataTerm()
{

    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    //ivec2 pos = ivec2(gl_LocalInvocationID.xy);
    ivec2 imSize = imageSize(flow_texture_x_y).xy;


    if (pix.x > imSize.x || pix.y > imSize.y) return;

    float Iz = luminance(imageLoad(im_mix_diff_warp_I, ivec3(pix.xy, 1)).xyz);

    //vec2 flowUV = imageLoad(flow_texture_x_y, ivec2(pix)).xy;
    vec2 d_U_V;

    if (iter == 0)
    {
        d_U_V = vec2(0);
    }
    else
    {
        if (flipflop == 0)
        {
            d_U_V = imageLoad(delta_flow_texture_u_v, ivec2(pix)).xy;
        }
        else if (flipflop == 1)
        {
            d_U_V = imageLoad(delta_flow_texture_u_v, ivec2(pix)).zw;
        }
    }


    vec2 Ix_Iy = imageLoad(im_grads_mix_diff_x_y, ivec3(pix.xy, 0)).xy;
    vec2 Ixz_Iyz = imageLoad(im_grads_mix_diff_x_y, ivec3(pix.xy, 1)).xy;

    vec2 Ixx_Ixy = imageLoad(im_second_grads_mix_diff_x_y, ivec3(pix.xy, 0)).xy;
    float Iyy = imageLoad(im_second_grads_mix_diff_x_y, ivec3(pix.xy, 1)).y;

    float iz1p = Iz;
    float ix1p = Ix_Iy.x;
    float iy1p = Ix_Iy.y;

    float ixx1p = Ixx_Ixy.x;
    float ixy1p = Ixx_Ixy.y;
    float iyy1p = Iyy;
    float ixz1p = Ixz_Iyz.x;
    float iyz1p = Ixz_Iyz.y;

    float dnorm = 0.1f * 0.1f;
    float dup = d_U_V.x;
    float dvp = d_U_V.y;

    float hdover3 = 5.0 * 0.5 / 3.0;
    float hgover3 = 10.0 * 0.5 / 0.3;
    float epscolor = 0.001 * 0.001;
    float epsgrad = 0.001f * 0.001f;

    float tmp = iz1p + (ix1p) * (dup) + (iy1p) * (dvp);
    float n1 = (ix1p) * (ix1p) + (iy1p) * (iy1p) + dnorm;

    tmp = hdover3 / sqrt(3.0 * tmp * tmp / n1 + epscolor);
    tmp /= n1;

    float _A11 = tmp * ix1p * ix1p;
    float _A12 = tmp * ix1p * iy1p;
    float _A22 = tmp * iy1p * iy1p;
    float _b1 = -tmp * iz1p * ix1p;
    float _b2 = -tmp * iz1p * iy1p;


    // dpsi gradient
    n1 = (ixx1p) * (ixx1p) + (ixy1p) * (ixy1p) + dnorm;
    float n2 = (iyy1p) * (iyy1p) + (ixy1p) * (ixy1p) + dnorm;
    tmp = ixz1p + (ixx1p) * (dup) + (ixy1p) * (dvp);
    float tmp2 = iyz1p + (ixy1p) * (dup) + (iyy1p) * (dvp);


    tmp = hgover3 / sqrt(3.0 * tmp * tmp / n1 + 3.0 * tmp2 * tmp2 / n2 + epsgrad);
    tmp2 = tmp / n2; tmp /= n1;

    _A11 += tmp * (ixx1p) * (ixx1p) + tmp2 * (ixy1p) * (ixy1p);
    _A12 += tmp * (ixx1p) * (ixy1p) + tmp2 * (ixy1p) * (iyy1p);
    _A22 += tmp2 * (iyy1p) * (iyy1p) + tmp * (ixy1p) * (ixy1p);
    _b1 -= tmp * (ixx1p) * (ixz1p) + tmp2 * (ixy1p) * (iyz1p);
    _b2 -= tmp2 * (iyy1p) * (iyz1p) + tmp * (ixy1p) * (ixz1p);

    _A11 *= 3.0f;
    _A12 *= 3.0f;
    _A22 *= 3.0f;
    _b1 *= 3.0f;
    _b2 *= 3.0f;

    dataTermBuffer[pix.y * imSize.x + pix.x].A11 = _A11;
    dataTermBuffer[pix.y * imSize.x + pix.x].A12 = _A12;
    dataTermBuffer[pix.y * imSize.x + pix.x].A22 = _A22;
    dataTermBuffer[pix.y * imSize.x + pix.x].b1 = _b1;
    dataTermBuffer[pix.y * imSize.x + pix.x].b2 = _b2;

    memoryBarrierBuffer();

    dataTerms dtb = dataTermBuffer[pix.y * imSize.x + pix.x];
    memoryBarrierBuffer();

    float tmpval = dtb.b1;

    //imageStore(im_smoothness_weight, ivec2(gl_GlobalInvocationID.xy), vec4(tmpval));



    //// STEP 1. compute color consistency terms
    //// normalization factor
    //float derivNorm = Ix_Iy.x * Ix_Iy.x + Ix_Iy.y * Ix_Iy.y + zeta_squared;

    //// Color constancy penalty (computed by Taylor expansion)
    //float Ik1z = Iz + Ix_Iy.x * d_U_V.x + Ix_Iy.y * d_U_V.y;

    //// weight of the color contancy term in the current fixed point iteration divided by derivnorm
    //float weight = (delta2 / sqrt(Ik1z * Ik1z / derivNorm + epsilon_squared)) / derivNorm;

    ////dataTermBuffer[pix.y * imageSize.x + pix.x].A11 = weight * Ix_Iy.x * Ix_Iy.x + zeta_sqaured;
    ////dataTermBuffer[pix.y * imageSize.x + pix.x].A12 = weight * Ix_Iy.x * Ix_Iy.y;
    ////dataTermBuffer[pix.y * imageSize.x + pix.x].A22 = weight * Ix_Iy.y * Ix_Iy.y + zeta_sqaured;
    ////dataTermBuffer[pix.y * imageSize.x + pix.x].b1 = -weight * Iz * Ix_Iy.x;
    ////dataTermBuffer[pix.y * imageSize.x + pix.x].b2 = -weight * Iz * Ix_Iy.y;
    //// add respective color constancy terms to the linear system coefficients
    //float _A11 = weight * Ix_Iy.x * Ix_Iy.x + zeta_squared;
    //float _A12 = weight * Ix_Iy.x * Ix_Iy.y;
    //float _A22 = weight * Ix_Iy.y * Ix_Iy.y + zeta_squared;
    //float _b1 = -weight * Iz * Ix_Iy.x;
    //float _b2 = -weight * Iz * Ix_Iy.y;

    //// STEP 2. Compute gradient constancy terms
    //// Normalization factor for x gradients
    //derivNorm = Ixx_Ixy.x * Ixx_Ixy.x + Ixx_Ixy.y * Ixx_Ixy.y + zeta_squared;

    //// Normalization factor for y gradients
    //float derivNorm2 = Iyy * Iyy + Ixx_Ixy.y * Ixx_Ixy.y + zeta_squared;

    //// Gradient constancy penalties (computed by taylor expansion)
    //float Ik1zx = Ixz_Iyz.x + Ixx_Ixy.x * d_U_V.x + Ixx_Ixy.y * d_U_V.y;// this is not the correct flow, this flow needs to be found out through multiple iterations
    //float Ik1zy = Ixz_Iyz.y + Ixx_Ixy.y * d_U_V.x + Iyy * d_U_V.y;

    //// weight of the gradient constancy term in the curent fixed point iteration
    //weight = gamma2 / sqrt(Ik1zx * Ik1zx / derivNorm + Ik1zy * Ik1zy / derivNorm2 + epsilon_squared);

    //// add respective gradient constancy componants to the linear system coefficients

    //dataTermBuffer[pix.y * imSize.x + pix.x].A11 = _A11 + weight * (Ixx_Ixy.x * Ixx_Ixy.x / derivNorm + Ixx_Ixy.y * Ixx_Ixy.y / derivNorm2);
    //dataTermBuffer[pix.y * imSize.x + pix.x].A12 = _A12 + weight * (Ixx_Ixy.x * Ixx_Ixy.y / derivNorm + Ixx_Ixy.y * Iyy / derivNorm2);
    //dataTermBuffer[pix.y * imSize.x + pix.x].A22 = _A22 + weight * (Ixx_Ixy.y * Ixx_Ixy.y / derivNorm + Iyy * Iyy / derivNorm2);
    //dataTermBuffer[pix.y * imSize.x + pix.x].b1 = _b1 - weight * (Ixx_Ixy.x * Ixz_Iyz.x / derivNorm + Ixx_Ixy.y * Ixz_Iyz.y / derivNorm2);
    //dataTermBuffer[pix.y * imSize.x + pix.x].b2 = _b2 - weight * (Ixx_Ixy.y * Ixz_Iyz.x / derivNorm + Iyy * Ixz_Iyz.y / derivNorm2);

    // barrier();



    ////dataTermBuffer[pix.y * imSize.x + pix.x].A11 = Ix_Iy.x;//_A11 + weight * (Ixx_Ixy.x * Ixx_Ixy.x / derivNorm + Ixx_Ixy.y * Ixx_Ixy.y / derivNorm2);
    ////dataTermBuffer[pix.y * imSize.x + pix.x].A12 = Ix_Iy.x;//_A12 + weight * (Ixx_Ixy.x * Ixx_Ixy.y / derivNorm + Ixx_Ixy.y * Iyy / derivNorm2);
    ////dataTermBuffer[pix.y * imSize.x + pix.x].A22 = Ix_Iy.x;//_A22 + weight * (Ixx_Ixy.y * Ixx_Ixy.y / derivNorm + Iyy * Iyy / derivNorm2);
    //// dataTermBuffer[pix.y * imSize.x + pix.x].b1 = Ix_Iy.x;//_b1 + -weight * (Ixx_Ixy.x * Ixz_Iyz.x / derivNorm + Ixx_Ixy.y * Ixz_Iyz.y / derivNorm2);
    //// dataTermBuffer[pix.y * imSize.x + pix.x].b2 = Ix_Iy.x; //_b2 + -weight * (Ixx_Ixy.y * Ixz_Iyz.x / derivNorm + Iyy * Ixz_Iyz.y / derivNorm2);


    //// compute this for each pixel, output a buffer of 5 float struct

}

// dont need to make the smoothness image, just when its needed read in the smoothness weights
subroutine(launchSubroutine)
void computeSmoothnessTerm()
{
    // weights calculated in sobel shader

    // input weight image, output dst_hori = weight im + weight im x + 1
    // output dst_vert = weight im + weight im y + 1
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = imageSize(im_smoothness_weight).xy;

    float sigma_u = 0.0f;
    float sigma_v = 0.0f;
    float sum_dpsis = 0.0f;

    if (pix.x > imSize.x - 1 || pix.y > imSize.y - 1) return;

    float weight = imageLoad(im_smoothness_weight, pix).x;
    float weight_right = imageLoad(im_smoothness_weight, ivec2(pix.x + 1, pix.y)).x;
    float weight_up = imageLoad(im_smoothness_weight, ivec2(pix.x, pix.y + 1)).x;

    imageStore(im_smoothness_term, pix, vec4(weight + weight_right, weight + weight_up, 0, 0));
    
}

subroutine(launchSubroutine)
void computeSOR()
{
    // time to solve
    // read in the a11, a12, a22, b1, b2 buffer data//
    // read in the d_u_v image
    // read in the weight image, remember that the vert and hori weight thingy is the weight plus the pixel above or next to it
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = imageSize(im_smoothness_term).xy;

    vec2 flowVec;
    float delta_u, delta_v;

    // DONT READ WRITE EVERY SOLVER ITERATION, JUST STORE VALUES AND REUSE THEM IN HERE
    for (int i = 0; i < 4; i++)
    {
        float sigma_u = 0.0f;
        float sigma_v = 0.0f;
        float sum_dpsis = 0.0f;

        if (pix.x < 1 || pix.x > imSize.x - 1 || pix.y < 1 || pix.y > imSize.y - 1) return;

        vec2 smooth_horiz_vert = imageLoad(im_smoothness_term, ivec2(pix.x, pix.y)).xy;

        vec2 smooth_horiz_vert_down = imageLoad(im_smoothness_term, ivec2(pix.x, pix.y - 1)).xy;
        //vec2 smooth_horiz_vert_up = imageLoad(im_smoothness_term, ivec2(pix.x, pix.y + 1)).x;
        vec2 smooth_horiz_vert_left = imageLoad(im_smoothness_term, ivec2(pix.x - 1, pix.y)).xy;
        //vec2 smooth_horiz_vert_right = imageLoad(im_smoothness_term, ivec2(pix.x + 1, pix.y)).x;

        vec2 flowVec_down;
        vec2 flowVec_up;
        vec2 flowVec_left;
        vec2 flowVec_right;
        if (iter == 0 && i == 0)
        {
            flowVec = vec2(0);
            flowVec_down = vec2(0);
            flowVec_up = vec2(0);
            flowVec_left = vec2(0);
            flowVec_right = vec2(0);
        }
        else
        {
            if (flipflop == 0)
            {
                flowVec = imageLoad(delta_flow_texture_u_v, pix).xy;
                flowVec_down = imageLoad(delta_flow_texture_u_v, ivec2(pix.x, pix.y - 1)).xy;
                flowVec_up = imageLoad(delta_flow_texture_u_v, ivec2(pix.x, pix.y + 1)).xy;
                flowVec_left = imageLoad(delta_flow_texture_u_v, ivec2(pix.x - 1, pix.y)).xy;
                flowVec_right = imageLoad(delta_flow_texture_u_v, ivec2(pix.x + 1, pix.y)).xy;
            }
            else if (flipflop == 1)
            {
                flowVec = imageLoad(delta_flow_texture_u_v, pix).zw;
                flowVec_down = imageLoad(delta_flow_texture_u_v, ivec2(pix.x, pix.y - 1)).zw;
                flowVec_up = imageLoad(delta_flow_texture_u_v, ivec2(pix.x, pix.y + 1)).zw;
                flowVec_left = imageLoad(delta_flow_texture_u_v, ivec2(pix.x - 1, pix.y)).zw;
                flowVec_right = imageLoad(delta_flow_texture_u_v, ivec2(pix.x + 1, pix.y)).zw;
            }
        }

        //// sub the laplacian, i.e. multiply the horizontal smoothness with the original flow hori = b1
        //// verty smooth * orig flow vert = b2

        float b1 = smooth_horiz_vert.x * flowVec.x;
        float b2 = smooth_horiz_vert.y * flowVec.y;


        sigma_u -= smooth_horiz_vert_down.y * flowVec_down.x;
        sigma_v -= smooth_horiz_vert_down.y * flowVec_down.y;
        sum_dpsis += smooth_horiz_vert_down.y;

        // if x > 0
        sigma_u -= smooth_horiz_vert_left.x * flowVec_left.x;
        sigma_v -= smooth_horiz_vert_left.x * flowVec_left.y;
        sum_dpsis += smooth_horiz_vert_left.x;

        // if y < height - 1
        sigma_u -= smooth_horiz_vert.y * flowVec_up.x;
        sigma_v -= smooth_horiz_vert.y * flowVec_up.y;
        sum_dpsis += smooth_horiz_vert.x;

        // if x < width - 1
        sigma_u -= smooth_horiz_vert.x * flowVec_right.x;
        sigma_v -= smooth_horiz_vert.x * flowVec_right.y;
        sum_dpsis += smooth_horiz_vert.x;

        // normal x and y
        dataTerms dtb = dataTermBuffer[pix.y * imSize.x + pix.x];


        float A11 = dtb.A11 + sum_dpsis;
        float A12 = dtb.A12;
        float A22 = dtb.A22 + sum_dpsis;
        float B1 = b1 - sigma_u;
        float B2 = b2 - sigma_v;

        //imageStore(im_smoothness_weight, ivec2(gl_GlobalInvocationID.xy), vec4(A12));


        float omega = 1.6f;

      //  float det = A11 * A22 - A12 * A12;

       // delta_u = (1.0f - omega) * flowVec.x + omega * (A22 * B1 - A12 * B2) / det;
       // delta_v = (1.0f - omega) * flowVec.y + omega * (-A12 * B1 + A11 * B2) / det;

        //  delta_u = (1.0f - omega) * flowVec.x + (omega / A11) * (B1 - A12 * flowVec.y);
        //  delta_v = (1.0f - omega) * flowVec.y + (omega / A22) * (B2 - A12 * flowVec.x);

        delta_u = omega * ((sigma_u + B1 - flowVec.y * A12) / A11 - flowVec.x);
        delta_v = omega * ((sigma_v + B2 - flowVec.x * A12) / A22 - flowVec.y);


        if (flipflop == 0)
        {

            imageStore(delta_flow_texture_u_v, pix, vec4(delta_u, delta_v, flowVec.x, flowVec.y));

        }
        else if (flipflop == 1)
        {
            imageStore(delta_flow_texture_u_v, pix, vec4(flowVec.x, flowVec.y, delta_u, delta_v));


        }
        // du->c1[j * du->stride + i] = (1.0f - omega) * du->c1[j * du->stride + i] + omega / A11 * (B1 - A12 * dv->c1[j * du->stride + i]);
        // dv->c1[j * du->stride + i] = (1.0f - omega) * dv->c1[j * du->stride + i] + omega / A22 * (B2 - A12 * du->c1[j * du->stride + i]);
        barrier();
    }

    vec2 preFlow = imageLoad(flow_texture_x_y, pix).xy;

    imageStore(total_flow_texture_u_v, pix, vec4(preFlow.x + delta_u, preFlow.y + delta_v, 0, 0));

    //if (iter == 3)
    //{
    //    if (level > 0)
    //    {
    //        // this is just NN upscaling, think of using texture samples with linear interp?
    //        imageStore(flow_texture_x_y, ivec2(x * 2, y * 2), vec4(2.0 * sum_Ux / sum_coef, 2.0 * sum_Uy / sum_coef, 0, 0));
    //        imageStore(flow_texture_x_y, ivec2(x * 2 + 1, y * 2), vec4(2.0 * sum_Ux / sum_coef, 2.0 * sum_Uy / sum_coef, 0, 0));
    //        imageStore(flow_texture_x_y, ivec2(x * 2, y * 2 + 1), vec4(2.0 * sum_Ux / sum_coef, 2.0 * sum_Uy / sum_coef, 0, 0));
    //        imageStore(flow_texture_x_y, ivec2(x * 2 + 1, y * 2 + 1), vec4(2.0 * sum_Ux / sum_coef, 2.0 * sum_Uy / sum_coef, 0, 0));

    //    }
    //}

}


subroutine(launchSubroutine)
void resize()
{

    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 imSize = ivec2(imageSize(flow_texture_x_y).xy);

    if (pix.x >= imSize.x || pix.y >= imSize.y) return;

    vec2 previousLayerValue =  2.0f * imageLoad(total_flow_texture_u_v, pix).xy;

    // this is just NN upscaling, think of using texture samples with linear interp?
    imageStore(flow_texture_x_y, ivec2(pix.x * 2, pix.y * 2), vec4(previousLayerValue.xy, 0, 0));
    imageStore(flow_texture_x_y, ivec2(pix.x * 2 + 1, pix.y * 2), vec4(previousLayerValue.xy, 0, 0));
    imageStore(flow_texture_x_y, ivec2(pix.x * 2, pix.y * 2 + 1), vec4(previousLayerValue.xy, 0, 0));
    imageStore(flow_texture_x_y, ivec2(pix.x * 2 + 1, pix.y * 2 + 1), vec4(previousLayerValue.xy, 0, 0));
}



void main()
{
    variRefineSubroutine();
    //prepareBuffers();
    // launch subroutines

}



