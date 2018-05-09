#version 430
layout(local_size_x = 1024) in;

layout(binding = 9) coherent buffer block1
{
    uint input_data[];
};

layout(binding = 10) coherent buffer block2
{
    uint output_data[];
};

layout(binding = 11) coherent buffer block3
{
    uint PrefixSumsByGroup[];
};

//shared float shared_data[gl_WorkGroupSize.x * 2];

const uint n = gl_WorkGroupSize.x * 2;

shared uint fastTempArr[n];

subroutine void CalculatePrefixSum();
subroutine uniform CalculatePrefixSum getPrefixSum;

subroutine(CalculatePrefixSum)
void resetSumsArray()
{
    // make sure on very large arrays this indeed gets wiped
    PrefixSumsByGroup[gl_LocalInvocationID.x] = 0;
}

subroutine(CalculatePrefixSum)
void forEachGroup()
{
    barrier();

    // doubled because each thread deals with 2 items, so the shared data size is double the 
    // work group size, and everything dealing with indices also doubles them, so just make a 
    // doubled variable up front
    uint doubleGroupThreadIndex = gl_LocalInvocationID.x * 2;
    uint doubleGlobalThreadIndex = gl_GlobalInvocationID.x * 2;

    // Note: Two elements per thread.
    fastTempArr[doubleGroupThreadIndex] = input_data[doubleGlobalThreadIndex];
    fastTempArr[doubleGroupThreadIndex + 1] = input_data[doubleGlobalThreadIndex + 1];

    // called simply "offset" in the GPU Gems article, this is a multiplier that works in 
    // conjunction with the thread number to calculate which index pairs are being considered on 
    // each loop by each thread
    uint indexMultiplierDueToDepth = 1;

    // going up divides pair count in half with each level
    for (uint dataPairs = gl_WorkGroupSize.x * 2 >> 1; dataPairs > 0; dataPairs >>= 1)
    {
        // wait for other threads in the group to catch up
        barrier();

        // one pair per thread
        // Note: Going up the tree will require fewer pair operations at each level.  Local 
        // thread ID 0 will always be doing something, but higher thread numbers will start 
        // sitting out until the "going down the tree" loop
        if (gl_LocalInvocationID.x < dataPairs)
        {
            uint lesserIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 1)) - 1;
            uint greaterIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 2)) - 1;

            fastTempArr[greaterIndex] += fastTempArr[lesserIndex];
        }

        // this is used in the "going down" loop, so do this even if the thread didn't do 
        // anything on this iteration
        indexMultiplierDueToDepth *= 2;
    }

    // only one thread should do these (prevents unnecessary writes)
    if (doubleGroupThreadIndex == 0)
    {
        // write the work group sum to the group sums buffer
        // Note: After the "going up" loop finishes, the last item in the shared memory array 
        // has the sum of all items in the entire array.  The following "going down" loop will 
        // change the data into a prefix-only sums array, so record the entire sum while it is 
        // still available.
        PrefixSumsByGroup[gl_WorkGroupID.x] = fastTempArr[n - 1];

        // this is just part of the algorithm; I don't have an intuitive explanation
        fastTempArr[(gl_WorkGroupSize.x * 2) - 1] = 0;
    }

    // undo the last loop's indexMultiplierDueToDepth
    // Note: After the last loop, indexMultiplierDueToDepth had been multiplied by 2 as many 
    // times as dataPairs had been divided by 2, so it is now equivalent to ITEMS_PER_WORK_GROUP 
    // (assuming that it is a power of 2).  Divide by 2 so that it can be used to calculate the 
    // indices of the first data pair off the root.


    // going down multiplies pair count in half with each level
    for (uint dataPairs = 1; dataPairs < gl_WorkGroupSize.x * 2; dataPairs *= 2)
    {
        indexMultiplierDueToDepth >>= 1;

        // wait for the other threads in the group to catch up
        barrier();

        // once again, group thread 0 is always working, but the others may need to sit out for 
        // a few loops
        if (gl_LocalInvocationID.x < dataPairs)
        {
            uint lesserIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 1)) - 1;
            uint greaterIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 2)) - 1;

            // this is a swap and a sum, so need a temporary value
            uint temp = fastTempArr[lesserIndex];
            fastTempArr[lesserIndex] = fastTempArr[greaterIndex];
            fastTempArr[greaterIndex] += temp;
        }

    }

    // write the data back, two elements per thread, but wait for all the group threads to 
    // finish their loops first
    barrier();

    output_data[doubleGlobalThreadIndex] = fastTempArr[doubleGroupThreadIndex];
    output_data[doubleGlobalThreadIndex + 1] = fastTempArr[doubleGroupThreadIndex + 1];

    //PrefixSumsWithinGroup[doubleGlobalThreadIndex] = fastTempArr[doubleGroupThreadIndex];
    //PrefixSumsWithinGroup[doubleGlobalThreadIndex + 1] = fastTempArr[doubleGroupThreadIndex + 1];
}

subroutine(CalculatePrefixSum)
void forEveryGroup()
{
    // the PrefixSumsByGroup array is only one work group's worth of data, so the starting 
    // index is implicitly 0 and there is no need for a workGroupStartingIndex
    uint doubleGroupThreadIndex = gl_LocalInvocationID.x * 2;
    uint doubleGlobalThreadIndex = gl_GlobalInvocationID.x * 2;

    uint indexMultiplierDueToDepth = 1;
    for (int dataPairs = int(n) >> 1; dataPairs > 0; dataPairs >>= 1)
    {
        barrier();

        if (gl_LocalInvocationID.x < dataPairs)
        {
            uint lesserIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 1)) - 1;
            uint greaterIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 2)) - 1;

            PrefixSumsByGroup[greaterIndex] += PrefixSumsByGroup[lesserIndex];
        }
        indexMultiplierDueToDepth *= 2;
    }

    // only one thread should do these (prevents unnecessary writes)
    if (doubleGroupThreadIndex == 0)
    {
        //totalNumberOfOnes = PrefixSumsByGroup[n - 1];
        PrefixSumsByGroup[n - 1] = 0;
    }
    indexMultiplierDueToDepth >>= 1;

    for (int dataPairs = 1; dataPairs < int(n); dataPairs *= 2)
    {
        barrier();

        if (gl_LocalInvocationID.x < dataPairs)
        {
            uint lesserIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 1)) - 1;
            uint greaterIndex = (indexMultiplierDueToDepth * (doubleGroupThreadIndex + 2)) - 1;

            uint temp = PrefixSumsByGroup[lesserIndex];
            PrefixSumsByGroup[lesserIndex] = PrefixSumsByGroup[greaterIndex];
            PrefixSumsByGroup[greaterIndex] += temp;
        }

        indexMultiplierDueToDepth >>= 1;
    }
}

subroutine(CalculatePrefixSum)
void forFinalIncrementalSum()
{
    // prefixsumbygroup can be larger than 1024, so we have to use globalinvocationid.x
    uint doubleGlobalThreadIndex = gl_GlobalInvocationID.x * 2;
    output_data[doubleGlobalThreadIndex] = output_data[doubleGlobalThreadIndex] + PrefixSumsByGroup[gl_WorkGroupID.x];
    output_data[doubleGlobalThreadIndex + 1] = output_data[doubleGlobalThreadIndex + 1] + PrefixSumsByGroup[gl_WorkGroupID.x];

    //for (int i = int(gl_LocalInvocationID.x * n); i < int(gl_LocalInvocationID.x * n) + 2048; i++)
    //{
    //    output_data[i] = output_data[i] + PrefixSumsByGroup[gl_GlobalInvocationID.x];
    //    //output_data[i + 1] = output_data[i + 1] + PrefixSumsByGroup[gl_LocalInvocationID.x];
    //}


}


void main()
{
    getPrefixSum();
}
