
/*
    Texture pixel/fragment shader for rendering a  2D slice of the generated 3D Worley noise
*/

// Defining different Active Channel Types because 5.0 doesn't support enums.
#define ACTIVE_CHANNEL_R 0U
#define ACTIVE_CHANNEL_G 1U
#define ACTIVE_CHANNEL_B 2U
#define ACTIVE_CHANNEL_A 3U

// Define whether we include the alpha channel value in the greyscale
#define INCLUDE_ALPHA_IN_GREYSCALE

// Buffer for Worley noise viewer debugging.
cbuffer ViewWorleySliceBuffer : register(b0)
{
    bool enableGreyscale;
    bool enableShowAllChannels;
    float sliceDepth;
    uint activeChannelType;
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
    
    float3 sampleLocation = float3((float2) input.tex, sliceDepth);
    
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
    float4 output = texture0.Sample(Sampler0, sampleLocation);
    
    if (!enableShowAllChannels)
    {
        float channelValue = 0.f;
        float4 channelMultiValue = (float4) 0.f;
        
        // Get specific channel.
        switch (activeChannelType)
        {
            case ACTIVE_CHANNEL_R:
                channelValue = output.r;
                channelMultiValue = float4(1.f, 0.f, 0.f, 0.f);
                break;
            case ACTIVE_CHANNEL_G:
                channelValue = output.g;
                channelMultiValue = float4(0.f, 1.f, 0.f, 0.f);
                break;
            case ACTIVE_CHANNEL_B:
                channelValue = output.b;
                channelMultiValue = float4(0.f, 0.f, 1.f, 0.f);
                break;
            case ACTIVE_CHANNEL_A:
                channelValue = output.a;
                channelMultiValue = float4(0.f, 0.f, 0.f, 1.f);
                break;
            default:
                break;
        }
        
        // If we're wanting to greyscale based on a specific channel,
        // just set the other channels to have the same channel value.
        if (enableGreyscale)
        {
            channelMultiValue = (float4) 1.0f;
        }
        
        output = channelValue * channelMultiValue;
    }
    else if (enableGreyscale)
    {
        float greyScaleValue = 0.f;
        
#ifdef INCLUDE_ALPHA_IN_GREYSCALE
        // Apply greyscale (including alpha channel).
        greyScaleValue = (output.r + output.g + output.b + output.a) / 4.f;
        output = (float4) greyScaleValue;
#else
        greyScaleValue = (output.r + output.g + output.b) / 3.f;
        output = float4((float3) greyScaleValue, output.a);
#endif   
    }
    
    return output;
}