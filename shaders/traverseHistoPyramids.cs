#version 430

layout(local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

// images
layout(binding = 0, r32ui) uniform uimage3D volumeData;
layout(binding = 1, r32ui) uniform uimage3D volumeDataOutput;
layout(binding = 2, rg16ui) uniform uimage3D histoPyramidBaseLevel;

// textures
layout(binding = 0) uniform usampler3D histoPyramidTexture; 
layout(binding = 1) uniform sampler3D volumeFloatTexture; 

layout(binding = 3) uniform usampler1D edgeTable;
layout(binding = 4) uniform usampler1D triTable;
layout(binding = 5) uniform usampler1D nrOfTrianglesTable;
layout(binding = 6) uniform usampler1D offsets3;

layout(binding = 7) uniform isampler3D volumeRGShortTexture;

// buffers 
layout(std430, binding = 0) buffer posBuf
{
    // DONT USE VEC3 IN SSBO https://stackoverflow.com/questions/38172696/should-i-ever-use-a-vec3-inside-of-a-uniform-buffer-or-shader-storage-buffer-o
    vec4 pos [];
};
//layout(std430, binding = 1) buffer normBuf
//{
//    vec3 norm [];
//};

// uniforms
uniform int baseLevel;
uniform float isoValue;
uniform uint totalSum;
uniform int volumeType = 1;

uniform uvec4 cubeOffsets[8] = {
    {0, 0, 0, 0},
    {1, 0, 0, 0},
    {0, 0, 1, 0},
    {1, 0, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {1, 1, 1, 0},    
    };

subroutine bool launchSubroutine();
subroutine uniform launchSubroutine traverseHistoPyramidsSubroutine;


// current = ivec4 x y z sum
void scanHPLevel(uint target, int lod, inout uvec4 current)
{
    uint neighbors[8];

    if (lod == 0)
    {
        neighbors[0] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz)).x;
        neighbors[1] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[1].xyz)).x;
        neighbors[2] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[2].xyz)).x;
        neighbors[3] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[3].xyz)).x;

        neighbors[4] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[4].xyz)).x;
        neighbors[5] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[5].xyz)).x;
        neighbors[6] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[6].xyz)).x;
        neighbors[7] = imageLoad(histoPyramidBaseLevel, ivec3(current.xyz) + ivec3(cubeOffsets[7].xyz)).x;
    }
    else if (lod > 0)
    {
        neighbors[0] = texelFetch(histoPyramidTexture, ivec3(current.xyz), lod).x;
        neighbors[1] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[1].xyz), lod).x;
        neighbors[2] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[2].xyz), lod).x;
        neighbors[3] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[3].xyz), lod).x;

        neighbors[4] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[4].xyz), lod).x;
        neighbors[5] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[5].xyz), lod).x;
        neighbors[6] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[6].xyz), lod).x;
        neighbors[7] = texelFetch(histoPyramidTexture, ivec3(current.xyz) + ivec3(cubeOffsets[7].xyz), lod).x;
    }


    uint acc = uint(current.w) + neighbors[0];

    uint cmp[8];

    cmp[0] = acc <= target ? 1 : 0;
    acc += neighbors[1];
    cmp[1] = acc <= target ? 1 : 0;
    acc += neighbors[2];
    cmp[2] = acc <= target ? 1 : 0;
    acc += neighbors[3];
    cmp[3] = acc <= target ? 1 : 0;
    acc += neighbors[4];
    cmp[4] = acc <= target ? 1 : 0;
    acc += neighbors[5];
    cmp[5] = acc <= target ? 1 : 0;
    acc += neighbors[6];
    cmp[6] = acc <= target ? 1 : 0;
    cmp[7] = 0;


    current += cubeOffsets[(cmp[0] + cmp[1] + cmp[2] + cmp[3] + cmp[4] + cmp[5] + cmp[6] + cmp[7])];
    current[0] = current[0] * 2;
    current[1] = current[1] * 2;
    current[2] = current[2] * 2;
    current[3] = current[3] +
        cmp[0] * neighbors[0] +
        cmp[1] * neighbors[1] +
        cmp[2] * neighbors[2] +
        cmp[3] * neighbors[3] +
        cmp[4] * neighbors[4] +
        cmp[5] * neighbors[5] +
        cmp[6] * neighbors[6] +
        cmp[7] * neighbors[7];


}

float getVolumeData(ivec3 pos, int level)
{
    if (volumeType == 0)
    {
        return texelFetch(volumeFloatTexture, pos, level).x;
    }
    else if (volumeType == 1)
    {
        return float(texelFetch(volumeRGShortTexture, pos, level).x);
    }

    return 0.0f;
}

subroutine(launchSubroutine)
bool traverseHPLevel()
{
    ivec3 texSize = textureSize(histoPyramidTexture, 0);
    uint target = uint(gl_GlobalInvocationID.x);



    if (target >= totalSum)
    {
        target = 0;
    }

    uvec4 cubePosition = uvec4(0); // x y z sum

    // now traverse pyr from top to bottom depending on image size

    // for loop for all levels using texel fetch and LOD
    if (texSize.x > 512)
    {
        scanHPLevel(target, 9, cubePosition);
    }
    if (texSize.x > 256)
    {
        scanHPLevel(target, 8, cubePosition);
    }
    if (texSize.x > 128)
    {
        scanHPLevel(target, 7, cubePosition);
    }
    if (texSize.x > 64)
    {
        scanHPLevel(target, 6, cubePosition);
    }

    scanHPLevel(target, 5, cubePosition);
    scanHPLevel(target, 4, cubePosition);
    scanHPLevel(target, 3, cubePosition);
    scanHPLevel(target, 2, cubePosition);
    scanHPLevel(target, 1, cubePosition);
    scanHPLevel(target, 0, cubePosition);

    cubePosition.x /= 2;
    cubePosition.y /= 2;
    cubePosition.z /= 2;

    int vertexNr = 0;

    //uvec4 cubeData = texelFetch(histoPyramidTexture, ivec3(cubePosition.xyz), 0);
    uint cubeIndex = imageLoad(histoPyramidBaseLevel, ivec3(cubePosition.xyz)).y;

    // max 5 triangles 
    for (int i = int(target - cubePosition.w) * 3; i < int(target - cubePosition.w + 1) * 3; i++)
    { // for each vertex in triangle
        int edge = int(texelFetch(triTable, int(cubeIndex * 16 + i), 0).x);
        ivec3 point0 = ivec3(cubePosition.x + texelFetch(offsets3, edge * 6, 0).x, cubePosition.y + texelFetch(offsets3, edge * 6 + 1, 0).x, cubePosition.z + texelFetch(offsets3, edge * 6 + 2, 0).x);
        ivec3 point1 = ivec3(cubePosition.x + texelFetch(offsets3, edge * 6 + 3, 0).x, cubePosition.y + texelFetch(offsets3, edge * 6 + 4, 0).x, cubePosition.z + texelFetch(offsets3, edge * 6 + 5, 0).x);


        //// Store vertex in VBO

        //vec3 forwardDifference0 = vec3(
        //        (-texelFetch(volumeFloatTexture, ivec3(point0.x + 1, point0.y, point0.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x - 1, point0.y, point0.z), 0).x),
        //        (-texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y + 1, point0.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y - 1, point0.z), 0).x),
        //        (-texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y, point0.z + 1), 0).x + texelFetch(volumeFloatTexture, ivec3(point0.x, point0.y, point0.z - 1), 0).x)
        //    );

        //vec3 forwardDifference1 = vec3(
        //        (-texelFetch(volumeFloatTexture, ivec3(point1.x + 1, point1.y, point1.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x - 1, point1.y, point1.z), 0).x),
        //        (-texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y + 1, point1.z), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y - 1, point1.z), 0).x),
        //        (-texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y, point1.z + 1), 0).x + texelFetch(volumeFloatTexture, ivec3(point1.x, point1.y, point1.z - 1), 0).x)
        //    );

        float value0 = getVolumeData(ivec3(point0.x, point0.y, point0.z), 0).x;
        float diff = (isoValue - value0) / (getVolumeData(ivec3(point1.x, point1.y, point1.z), 0).x - value0);


        // THIS FIX IS BECAUSE SOMETHING IS DIVIDNG BY ZERO AND FUCKING UP
        if (diff > 1 || diff < 0)
        {
            diff = 0.5;
        }
        // 0.5 ==== diff
        const vec3 vertex = mix(vec3(point0.x, point0.y, point0.z), vec3(point1.x, point1.y, point1.z), diff);
        //const vec3 normal = mix(forwardDifference0, forwardDifference1, diff);


        //    vstore3(vertex, target * 6 + vertexNr * 2, VBOBuffer);
        //    vstore3(normal, target * 6 + vertexNr * 2 + 1, VBOBuffer);
        // if (!isnan(diff))
        // {
        //  vec3 tV = vertex / 256.0f;
        // if (tV.x <0.2 || tV.y <0.2 || tV.z <=0.2)
        // {

        // }
        //else
        // {
        // if (point0.x == 0 || point0.y == 0 || point0.z == 0)
        //  {

        // }
        //  else
        // {
        // target is for each triangle
        // the problem is hereerererrererererere
        pos[target * 3 + vertexNr] = vec4(vertex.xyz, 0.0f);
        // }
        //norm[target * 3 + vertexNr] = normal;
        //}


        // }
        // else
        // {
        //    pos[target * 3 + vertexNr] = vec3(0);
        //     norm[target * 6 + vertexNr * 2] = normal;
        // }


        ++vertexNr;
    }


    return true;
}

void main()
{
    bool done = traverseHistoPyramidsSubroutine();
}