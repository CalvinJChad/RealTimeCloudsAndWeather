
/*
    Texture pixel/fragment shader for rendering a  2D slice of the generated 3D Worley noise
*/

// Buffer for Worley noise viewer debugging.
cbuffer ViewWorleySliceBuffer : register(b0)
{
    bool enableGreyscale;
    bool enableShowAllChannels;
    float sliceDepth;
    float padding;
};

// Texture and sampler registers
Texture3D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};


float4 main(InputType input) : SV_TARGET
{
    // Get the dimensions of the Output Texture.
    uint width, height, depth;
    texture0.GetDimensions(width, height, depth);
    
    float3 sampleLocation = float3((float2) input.tex, depth * sliceDepth);
    
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    return texture0.Sample(Sampler0, sampleLocation);
}