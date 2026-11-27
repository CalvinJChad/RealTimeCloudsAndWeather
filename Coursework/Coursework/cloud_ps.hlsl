// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
//Texture2D mainTexture : register(t0);
//SamplerState mainSampler : register(s0);
//Texture2D cameraDepthTexture : register(t1);
//SamplerState depthSampler : register(s1);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};


float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    return float4(0.0f , 0.0f, 0.0f, 1.0f);
}