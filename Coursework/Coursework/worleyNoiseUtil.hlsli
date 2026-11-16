
/*
    HLSL Header File containing all the necessary buffers and functions for generating 3D Worley noise.
*/

// =====================================================================================
// Code from NadirRoGue's RenderEngine https://github.com/NadirRoGue/RenderEngine
// and from Sebastien Hillarie 3d noise generator https://github.com/sebh/TileableVolumeNoise
// =====================================================================================

float hash(int n)
{
    return frac(sin(float(n) + 1.951) * 43758.5453123);
}

float noise(float3 x)
{
    float3 p = floor(x);
    float3 f = frac(x);

    f = f * f * ((float3) 3.0 - (float3) 2.0 * f);
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