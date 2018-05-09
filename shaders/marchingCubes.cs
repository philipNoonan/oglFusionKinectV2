#version 430

layout(local_size_x = 128, local_size_y = 1) in;

layout(binding = 0, rg16i) uniform iimage3D volumeData; // Gimage3D, where G = i, u, or blank, for int, u int, and floats respectively

layout(binding = 1, r32f) uniform image1D edgeTable;
layout(binding = 2, r32f) uniform image1D triTex;
layout(binding = 3, r32f) uniform image1D numVertsTable;

//layout(binding = 4, rgba32f) uniform image2D testImage;

layout(std430, binding = 2) buffer voxelVertsBuf
{
    uint voxelVerts [];
};
layout(std430, binding = 3) buffer voxelOccupiedBuf
{
    uint voxelOccupied [];
};
layout(std430, binding = 4) buffer voxelOccupiedScanBuf
{
    uint voxelOccupiedScan [];
};
layout(std430, binding = 5) buffer compactedVoxelArrayBuf
{
    uint compactedVoxelArray [];
};
layout(std430, binding = 6) buffer voxelVertsScanBuf
{
    uint voxelVertsScan [];
};
layout(std430, binding = 7) buffer posBuf
{
    vec4 pos [];
};
layout(std430, binding = 8) buffer normBuf
{
    vec4 norm [];
};


uniform uint numVoxels;
uniform uvec3 gridSize;
uniform uvec3 gridSizeShift;
uniform uvec3 gridSizeMask;
uniform vec3 voxelSize;
uniform float isoValue;
uniform uint activeVoxels;
uniform uint maxVerts;

shared vec3 vertlist[12 * gl_WorkGroupSize.x];

subroutine bool launchSubroutine();
subroutine uniform launchSubroutine marchingCubesSubroutine;



// compute position in 3d grid from 1d index
// only works for power of 2 sizes
uvec3 calcGridPos(uint i, uvec3 gridSizeShift, uvec3 gridSizeMask)
{
    uvec3 gridPos;
    gridPos.x = i & gridSizeMask.x;
    gridPos.y = (i >> gridSizeShift.y) & gridSizeMask.y;
    gridPos.z = (i >> gridSizeShift.z) & gridSizeMask.z;
    return gridPos;
}

float sampleVolume(uvec3 p, uvec3 gridSize)
{
    p.x = min(p.x, gridSize.x - 1);
    p.y = min(p.y, gridSize.y - 1);
    p.z = min(p.z, gridSize.z - 1);
    //uint i = (p.z * gridSize.x * gridSize.y) + (p.y * gridSize.x) + p.x;
    ivec4 volData = imageLoad(volumeData, ivec3(p));
    return float(volData.x);// * 0.00003051944088f;
}

// compute interpolated vertex along an edge
vec3 vertexInterp(float isoLevel, vec3 p0, vec3 p1, float f0, float f1)
{
    float t = (isoLevel - f0) / (f1 - f0);
    return mix(p0, p1, t);
}

// calculate normal, but thnk of skipping this and doing it in vertex shader instead
vec3 calcNormal(vec3 v0, vec3 v1, vec3 v2)
{
    vec3 edge0 = v1 - v0;
    vec3 edge1 = v2 - v0;

    return cross(edge0, edge1);
}


// here we calc grid position
// sample volume for gridposition and surrounding 8 corners of unit voxel size gridsize
// determine cubeindex
// read number of verticies from texture
// the outputs are the voxelVerts[] array and the voxelOccupied[] array
subroutine(launchSubroutine)
bool launchClassifyVoxel()
{
    uint i = gl_GlobalInvocationID.x;

    uvec3 gridPos = calcGridPos(i, gridSizeShift, gridSizeMask);

        // read field values at neighbouring grid vertices
    float field[8];
    field[0] = sampleVolume(gridPos, gridSize);
    field[1] = sampleVolume(gridPos + uvec3(1, 0, 0), gridSize);
    field[2] = sampleVolume(gridPos + uvec3(1, 1, 0), gridSize);
    field[3] = sampleVolume(gridPos + uvec3(0, 1, 0), gridSize);
    field[4] = sampleVolume(gridPos + uvec3(0, 0, 1), gridSize);
    field[5] = sampleVolume(gridPos + uvec3(1, 0, 1), gridSize);
    field[6] = sampleVolume(gridPos + uvec3(1, 1, 1), gridSize);
    field[7] = sampleVolume(gridPos + uvec3(0, 1, 1), gridSize);

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
        voxelVerts[i] = 0; // no vertices returned
        voxelOccupied[i] = 0;

        return false;
    }


    // calculate flag indicating if each vertex is inside or outside isosurface
    uint cubeindex;
    cubeindex = uint(field[0] < isoValue);
    cubeindex += uint(field[1] < isoValue) * 2;
    cubeindex += uint(field[2] < isoValue) * 4;
    cubeindex += uint(field[3] < isoValue) * 8;
    cubeindex += uint(field[4] < isoValue) * 16;
    cubeindex += uint(field[5] < isoValue) * 32;
    cubeindex += uint(field[6] < isoValue) * 64;
    cubeindex += uint(field[7] < isoValue) * 128;

    // read number of vertices from texture
    uint numVerts = uint(imageLoad(numVertsTable, int(cubeindex)).x);
    
    if (i < numVoxels)
    {
        voxelVerts[i] = numVerts;
        if (numVerts > 0)
        {
            voxelOccupied[i] = 1;
        }
        else
        {
            voxelOccupied[i] = 0;
        }
    }

    return true;

}

// compactthe voxel array
subroutine(launchSubroutine)
bool launchCompactVoxels()
{
    uint i = gl_GlobalInvocationID.x;

    if (voxelOccupied[i] == 1 && (i < numVoxels))
    {
        compactedVoxelArray[voxelOccupiedScan[i]] = i;
    }

    return true;
}




subroutine(launchSubroutine)
bool launchGenerateTriangles()
{
    uint i = gl_GlobalInvocationID.x;

    if (i > activeVoxels - 1)
    {
        // can't return here because of barrier()
        i = activeVoxels - 1;
    }

    uint voxel = compactedVoxelArray[i];

    // compute position in 3d grid
    uvec3 gridPos = calcGridPos(voxel, gridSizeShift, gridSizeMask);

    vec3 p;
    p.x = -1.0f + (gridPos.x * voxelSize.x);
    p.y = -1.0f + (gridPos.y * voxelSize.y);
    p.z = -1.0f + (gridPos.z * voxelSize.z);

    // calculate cell vertex positions
    vec3 v[8];
    v[0] = p;
    v[1] = p + vec3(voxelSize.x, 0, 0);
    v[2] = p + vec3(voxelSize.x, voxelSize.y, 0);
    v[3] = p + vec3(0, voxelSize.y, 0);
    v[4] = p + vec3(0, 0, voxelSize.z);
    v[5] = p + vec3(voxelSize.x, 0, voxelSize.z);
    v[6] = p + vec3(voxelSize.x, voxelSize.y, voxelSize.z);
    v[7] = p + vec3(0, voxelSize.y, voxelSize.z);

    float field[8];
    field[0] = sampleVolume(gridPos, gridSize);
    field[1] = sampleVolume(gridPos + uvec3(1, 0, 0), gridSize);
    field[2] = sampleVolume(gridPos + uvec3(1, 1, 0), gridSize);
    field[3] = sampleVolume(gridPos + uvec3(0, 1, 0), gridSize);
    field[4] = sampleVolume(gridPos + uvec3(0, 0, 1), gridSize);
    field[5] = sampleVolume(gridPos + uvec3(1, 0, 1), gridSize);
    field[6] = sampleVolume(gridPos + uvec3(1, 1, 1), gridSize);
    field[7] = sampleVolume(gridPos + uvec3(0, 1, 1), gridSize);

    //for (int i = 0; i < 7; i++)
    //{
    //    if (field[i] < -50000 || field[i] > 50000)
    //    {
    //        return false;
    //    }
    //}

    // recalculate flag
    // (this is faster than storing it in global memory)
    uint cubeindex;
    cubeindex = uint(field[0] < isoValue);
    cubeindex += uint(field[1] < isoValue) * 2;
    cubeindex += uint(field[2] < isoValue) * 4;
    cubeindex += uint(field[3] < isoValue) * 8;
    cubeindex += uint(field[4] < isoValue) * 16;
    cubeindex += uint(field[5] < isoValue) * 32;
    cubeindex += uint(field[6] < isoValue) * 64;
    cubeindex += uint(field[7] < isoValue) * 128;

    // use partioned shared memory to avoid using local memory
    uint thread = gl_LocalInvocationID.x;
    vertlist[thread] = vertexInterp(isoValue, v[0], v[1], field[0], field[1]);
    vertlist[gl_WorkGroupSize.x + thread] = vertexInterp(isoValue, v[1], v[2], field[1], field[2]);
    vertlist[(gl_WorkGroupSize.x * 2) + thread] = vertexInterp(isoValue, v[2], v[3], field[2], field[3]);
    vertlist[(gl_WorkGroupSize.x * 3) + thread] = vertexInterp(isoValue, v[3], v[0], field[3], field[0]);
    vertlist[(gl_WorkGroupSize.x * 4) + thread] = vertexInterp(isoValue, v[4], v[5], field[4], field[5]);
    vertlist[(gl_WorkGroupSize.x * 5) + thread] = vertexInterp(isoValue, v[5], v[6], field[5], field[6]);
    vertlist[(gl_WorkGroupSize.x * 6) + thread] = vertexInterp(isoValue, v[6], v[7], field[6], field[7]);
    vertlist[(gl_WorkGroupSize.x * 7) + thread] = vertexInterp(isoValue, v[7], v[4], field[7], field[4]);
    vertlist[(gl_WorkGroupSize.x * 8) + thread] = vertexInterp(isoValue, v[0], v[4], field[0], field[4]);
    vertlist[(gl_WorkGroupSize.x * 9) + thread] = vertexInterp(isoValue, v[1], v[5], field[1], field[5]);
    vertlist[(gl_WorkGroupSize.x * 10) + thread] = vertexInterp(isoValue, v[2], v[6], field[2], field[6]);
    vertlist[(gl_WorkGroupSize.x * 11) + thread] = vertexInterp(isoValue, v[3], v[7], field[3], field[7]);

    barrier();

    // output triangle vertices
    uint numVerts = uint(imageLoad(numVertsTable, int(cubeindex)).x);

    for (int i = 0; i < numVerts; i += 3)
    {
        uint index = voxelVertsScan[voxel] + i;

        vec3 vOut[3]; // corresponds to the volume range 0 : numVoxels.x, to get this in meters

        uint edge;
        edge = uint(imageLoad(triTex, int((cubeindex * 16) + i)).x);
        vOut[0] = vec3(vertlist[(edge * gl_WorkGroupSize.x) + thread]);

        edge = uint(imageLoad(triTex, int((cubeindex * 16) + i + 1)).x);
        vOut[1] = vec3(vertlist[(edge * gl_WorkGroupSize.x) + thread]);

        edge = uint(imageLoad(triTex, int((cubeindex * 16) + i + 2)).x);
        vOut[2] = vec3(vertlist[(edge * gl_WorkGroupSize.x) + thread]);


        vec3 n = calcNormal(vOut[0], vOut[1], vOut[2]);

        if (index < (maxVerts - 3))
        {
            pos[index] = vec4((vOut[0].x / 128.0f) * voxelSize.x, (vOut[0].y / 128.0f) * voxelSize.x, -(vOut[0].z / 128.0f) * voxelSize.x, 1.0f);
            norm[index] = vec4(n, 0.0f);

            pos[index + 1] = vec4((vOut[1].x / 128.0f) * voxelSize.x, (vOut[1].y / 128.0f) * voxelSize.x, -(vOut[1].z / 128.0f) * voxelSize.x, 1.0f);
            norm[index + 1] = vec4(n, 0.0f);

            pos[index + 2] = vec4((vOut[2].x / 128.0f) * voxelSize.x, (vOut[2].y / 128.0f) * voxelSize.x, -(vOut[2].z / 128.0f) * voxelSize.x, 1.0f);
            norm[index + 2] = vec4(n, 0.0f);
        }
    }

    return true;

}



void main()
{
    // do subroutines of 
    // 1. classify voxel
    // 2. compact voxels
    // 3. generate triangles
    //for (int y = 0; y < 128; y++)
    //{
    //    for (int x = 0; x < 64; x++)
    //    {
    //        imageStore(testImage, ivec2(x, y), vec4(0.5, 0.1, 0.7, 1));

    //    }

    //}

    bool done = marchingCubesSubroutine();


}