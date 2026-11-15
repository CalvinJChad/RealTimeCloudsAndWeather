
/*
    Compute shader for generating 3D Worley noise.
    Note: using Shader Model 5.0 to allow for Texture3D output.
          For some reason, 5.1 doesn't work.
*/

// =====================================================================================
// Code from NadirRoGue's RenderEngine https://github.com/NadirRoGue/RenderEngine
// and from Sebastien Hillarie 3d noise generator https://github.com/sebh/TileableVolumeNoise
// =====================================================================================

#define NUM_THREADS 8

#define CLAMPRESULT

RWTexture3D<float4> gOutput : register(u0);

uniform float frequenceMul[6u] = { 2.0, 8.0, 14.0, 20.0, 26.0, 32.0 };

float hash(int n)
{
    return frac(sin(float(n) + 1.951) * 43758.5453123);
}

float noise(float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * ((float3)3.0 - (float3)2.0 * f);
    float n = p.x + p.y * 57.0 + 113.0 * p.z;
    
    return lerp(
		lerp(
			lerp(hash(int(n + 0.0)), hash(int(n + 1.0)), f.x),
			lerp(hash(int(n + 57.0)), hash(int(n + 58.0)), f.x),
			f.y),
		lerp(
			lerp(hash(int(n + 113.0)), hash(int(n + 114.0)), f.x),
			lerp(hash(int(n + 170.0)), hash(int(n + 171.0)), f.x),
			f.y),
		f.z);
}

float cells(float3 p, float cellCount)
{
    float3 pCell = p * cellCount;
    float d = 1.0e10;
    
    // Check neighboring cells in a 3x3x3 neighborhood
    for (int xo = -1; xo <= 1; xo++)
    {
        for (int yo = -1; yo <= 1; yo++)
        {
            for (int zo = -1; zo <= 1; zo++)
            {
                float3 tp = floor(pCell) + float3(xo, yo, zo);

                tp = pCell - tp - noise(fmod(tp, cellCount / 1.0)); // Note: hlsl's 'fmod' function acts differently to glsl's 'mod function if the x/y value is negative.

                d = min(d, dot(tp, tp));
            }
        }
    }
    
    d = min(d, 1.0);
    d = max(d, 0.0f);

    return d;
}

float worleyNoise3D(float3 p, float cellCount)
{
    return cells(p, cellCount);
}

float4 stackable3DNoise(int3 pixel)
{
    float3 coord = float3(float(pixel.x) / 32.0, float(pixel.y) / 32.0, float(pixel.z) / 32.0);

	// 3 octaves
    float cellCount = 2.0;
    float worleyNoise0 = (1.0f - worleyNoise3D(coord, cellCount * 1.0));
    float worleyNoise1 = (1.0f - worleyNoise3D(coord, cellCount * 2.0));
    float worleyNoise2 = (1.0f - worleyNoise3D(coord, cellCount * 4.0));
    float worleyNoise3 = (1.0f - worleyNoise3D(coord, cellCount * 8.0));
#ifdef CLAMPRESULT
	float worleyFBM0 = clamp(worleyNoise0*0.625f + worleyNoise1*0.25f + worleyNoise2*0.125f, 0.0, 1.0);
	float worleyFBM1 = clamp(worleyNoise1*0.625f + worleyNoise2*0.25f + worleyNoise3*0.125f, 0.0, 1.0);
	float worleyFBM2 = clamp(worleyNoise2*0.75f + worleyNoise3*0.25f, 0.0, 1.0);
#else
    float worleyFBM0 = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;
    float worleyFBM1 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
    float worleyFBM2 = worleyNoise2 * 0.75f + worleyNoise3 * 0.25f;
#endif
	// cellCount=4 -> worleyNoise4 is just noise due to sampling frequency=texel freque. So only take into account 2 frequencies for FBM

    return float4(worleyFBM0, worleyFBM1, worleyFBM2, 1.0);
}

// Define the thread group size (e.g. 8x8x8 threads per group)
[numthreads(NUM_THREADS, NUM_THREADS, NUM_THREADS)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Get the dimensions of the Output Texture.
    uint width, height, depth;
    gOutput.GetDimensions(width, height, depth);
    
    //int3 pixel = dispatchThreadID.xyz;
    
    gOutput[dispatchThreadID] = stackable3DNoise(dispatchThreadID);
}