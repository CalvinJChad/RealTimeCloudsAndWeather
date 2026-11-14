// Compute shader for generating 3D Worley noise.
// Note: using Shader Model 5.0 to allow for Texture3D output.
      // For some reason, 5.1 doesn't work.

#define N 8

RWTexture3D<float4> gOutput : register(u0);

// Define the thread group size (e.g. 8x8x8 threads per group)
[numthreads(N, N, N)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Get the dimensions of the Output Texture.
    uint width, height, depth;
    gOutput.GetDimensions(width, height, depth);
    
    // Testing to make sure this compute shader works.
    // For now, just set the Output Texture to a default value.
    gOutput[dispatchThreadID] = float4(1.0f, 1.0f, 1.0f, 1.0f);
}