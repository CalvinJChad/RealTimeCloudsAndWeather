
/*
    Compute shader for generating 3D Perlin-Worley noise.
    Generated Perlin-Worley noise values are stored in the R channel.
    GBA channels store Worley noise at increasing frequencies.

    To be used in volumetric clouds.
*/

// =====================================================================================
// Code from NadirRoGue's RenderEngine https://github.com/NadirRoGue/RenderEngine
// and from Sebastien Hillarie 3d noise generator https://github.com/sebh/TileableVolumeNoise
// =====================================================================================

#include "perlinNoiseUtil.hlsli"
#include "worleyNoiseUtil.hlsli"

#define NUM_THREADS 8

#define CLAMPRESULT

RWTexture3D<float4> gOutput : register(u0);

static const float frequenceMul[6u] = { 2.0, 8.0, 14.0, 20.0, 26.0, 32.0 };

// ======================================================================

float remap(float originalValue, float originalMin, float originalMax, float newMin, float newMax)
{
    return newMin + (((originalValue - originalMin) / (originalMax - originalMin)) * (newMax - newMin));
}

float4 stackable3DNoise(uint3 pixel, uint w, uint h, uint d)
{
    float3 coord = float3(float(pixel.x) / w, float(pixel.y) / h, float(pixel.z) / d);

	// Perlin FBM noise
    int octaveCount = 3;
    float frequency = 8.0;
    float perlinNoise = perlinNoise3D(coord, frequency, octaveCount);

    float PerlinWorleyNoise = 0.0f;
	{
        float cellCount = 4.0;
        float worleyNoise0 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[0]));
        float worleyNoise1 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[1]));
        float worleyNoise2 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[2]));
        float worleyNoise3 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[3]));
        float worleyNoise4 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[4]));
        float worleyNoise5 = (1.0 - worleyNoise3D(coord, cellCount * frequenceMul[5])); // half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

		// PerlinWorley noise as described p.101 of GPU Pro 7
        float worleyFBM = worleyNoise0 * 0.625f + worleyNoise1 * 0.25f + worleyNoise2 * 0.125f;

        PerlinWorleyNoise = remap(perlinNoise, 0.0, 1.0, worleyFBM, 1.0);
    }

    float cellCount = 4.0;
    float worleyNoise0 = (1.0 - worleyNoise3D(coord, cellCount * 1.0));
    float worleyNoise1 = (1.0 - worleyNoise3D(coord, cellCount * 2.0));
    float worleyNoise2 = (1.0 - worleyNoise3D(coord, cellCount * 4.0));
    float worleyNoise3 = (1.0 - worleyNoise3D(coord, cellCount * 8.0));
    float worleyNoise4 = (1.0 - worleyNoise3D(coord, cellCount * 16.0));
	//float worleyNoise5 = (1.0f - Tileable3dNoise::WorleyNoise(coord, cellCount * 32));	
	//cellCount=2 -> half the frequency of texel, we should not go further (with cellCount = 32 and texture size = 64)

	// Three frequency of Worley FBM noise
    float worleyFBM0 = worleyNoise1 * 0.625f + worleyNoise2 * 0.25f + worleyNoise3 * 0.125f;
    float worleyFBM1 = worleyNoise2 * 0.625f + worleyNoise3 * 0.25f + worleyNoise4 * 0.125f;
	//float worleyFBM2 = worleyNoise3*0.625f + worleyNoise4*0.25f + worleyNoise5*0.125f;
    float worleyFBM2 = worleyNoise3 * 0.75f + worleyNoise4 * 0.25f;
	// cellCount=4 -> worleyNoise5 is just noise due to sampling frequency=texel frequency. So only take into account 2 frequencies for FBM
#ifdef CLAMPRESULT
    return clamp(float4(PerlinWorleyNoise * PerlinWorleyNoise, worleyFBM0, worleyFBM1, worleyFBM2), 0.0, 1.0);
#else
	return float4(PerlinWorleyNoise * PerlinWorleyNoise, worleyFBM0, worleyFBM1, worleyFBM2);
#endif
}

// Define the thread group size (e.g. 8x8x8 threads per group)
[numthreads(NUM_THREADS, NUM_THREADS, NUM_THREADS)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // Get the dimensions of the Output Texture.
    uint width, height, depth;
    gOutput.GetDimensions(width, height, depth);
    
    //int3 pixel = dispatchThreadID.xyz;
    
    gOutput[dispatchThreadID] = stackable3DNoise(dispatchThreadID, width, height, depth);
}