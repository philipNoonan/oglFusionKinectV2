#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

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

// uniforms
uniform int baseLevel;
uniform float isoLevel = 1000.0f;
uniform uint totalSum;
uniform uint volumeType = 1; // 0 = float, 1 = RG16I

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
subroutine uniform launchSubroutine histoPyramidsSubroutine;


subroutine(launchSubroutine)
bool constructHPLevel()
{
    ivec3 writePos = ivec3(gl_GlobalInvocationID.xyz);
    ivec3 readPos = writePos * 2;
    ivec3 imSize = ivec3(0);

    if (baseLevel == 1)
    {
        imSize = imageSize(histoPyramidBaseLevel);
    }
    else if (baseLevel == 0)
    {
        imSize = imageSize(volumeData);
    }
    if (readPos.x > imSize.x || readPos.y > imSize.y || readPos.z > imSize.z)
    {
        return false;
    }

    uint writeValue = 0;
    if (baseLevel == 1)
    {
        writeValue = imageLoad(histoPyramidBaseLevel, readPos).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[1].xyz)).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[2].xyz)).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[3].xyz)).x +

                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[4].xyz)).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[5].xyz)).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[6].xyz)).x +
                         imageLoad(histoPyramidBaseLevel, readPos + ivec3(cubeOffsets[7].xyz)).x;
    }
    else if (baseLevel == 0)
    {
        writeValue = imageLoad(volumeData, readPos).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[1].xyz)).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[2].xyz)).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[3].xyz)).x +

                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[4].xyz)).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[5].xyz)).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[6].xyz)).x +
                         imageLoad(volumeData, readPos + ivec3(cubeOffsets[7].xyz)).x;
    }

 

    imageStore(volumeDataOutput, writePos, uvec4(writeValue));

    return true;
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
bool classifyCubes()
{
    ivec3 pos = ivec3(gl_GlobalInvocationID.xyz);

    ivec3 texSize = textureSize(histoPyramidTexture, 0);


    if (pos.x <= 10 || pos.x >= texSize.x - 10 || pos.y <= 10 || pos.y >= texSize.y -10 || pos.z <= 10 || pos.z >= texSize.z - 10)
    {
        return false;
    }

    float first = getVolumeData(pos, 0).x;

    float field[8];
    field[0] = getVolumeData(pos, 0).x;
    field[1] = getVolumeData(pos + ivec3(cubeOffsets[1].xyz), 0).x;
    field[2] = getVolumeData(pos + ivec3(cubeOffsets[2].xyz), 0).x;
    field[3] = getVolumeData(pos + ivec3(cubeOffsets[3].xyz), 0).x;
    field[4] = getVolumeData(pos + ivec3(cubeOffsets[4].xyz), 0).x;
    field[5] = getVolumeData(pos + ivec3(cubeOffsets[5].xyz), 0).x;
    field[6] = getVolumeData(pos + ivec3(cubeOffsets[6].xyz), 0).x;
    field[7] = getVolumeData(pos + ivec3(cubeOffsets[7].xyz), 0).x;

    if (volumeType == 1)
    {
        // we are only interested in voxels that are on the +ve -> 0 -> -ve boundary
        // not the 0 -> -ve boundary behind the front surface
        // therefore need to have at least one positive value in field[]
        bool positiveFound = false;
        for (int i = 0; i < 7; i++)
        {
            if (field[i] > 0)
            {
                positiveFound = true;
            }
        }

        if (positiveFound == false)
        {
            imageStore(histoPyramidBaseLevel, pos, uvec4(0, 0, 0, 0));


            return false;
        }
    }

    // https://stackoverflow.com/questions/43769622/bit-manipulation-to-store-multiple-values-in-one-int-c
    uint cubeIndex;
    cubeIndex = uint(field[0] < isoLevel);
    cubeIndex += uint(field[1] < isoLevel) * 2;
    cubeIndex += uint(field[3] < isoLevel) * 4;
    cubeIndex += uint(field[2] < isoLevel) * 8;
    cubeIndex += uint(field[4] < isoLevel) * 16;
    cubeIndex += uint(field[5] < isoLevel) * 32;
    cubeIndex += uint(field[7] < isoLevel) * 64;
    cubeIndex += uint(field[6] < isoLevel) * 128;

    uint numTri = texelFetch(nrOfTrianglesTable, int(cubeIndex), 0).x;

    imageStore(histoPyramidBaseLevel, pos, uvec4(numTri, cubeIndex, 0, 0));

    return true;

}




void main()
{

    bool done = histoPyramidsSubroutine();
}