// Cloud density vertex shader
// Shader for rendering 3D Perlin-Worley noise inside of a volume

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer BoundingBoxVolumeBuffer : register(b1)
{
    float4 boundsMin;   // Local minimum bounds (x, y, z, 1)
    float4 boundsMax;   // Local maximum bounds (x, y, z, 1)
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;                           // Position of the vertex in world space
    // World space min/max positions are set to nointerpolation as they are constant for all vertices (don't need interpolating).
    nointerpolation float3 boundMinWorldPosition : TEXCOORD2;   // Position of the minimum bounds of the bounding box volume in world space
    nointerpolation float3 boundMaxWorldPosition : TEXCOORD3;   // Position of the maximum bounds of the bounding box volume in world space
};

OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    
    // Get the world position of the vertex (for density calculations).
    output.worldPosition = mul(input.position, worldMatrix).xyz;
    // Get the position of the lower and uppper bounds of the box volume in world space.
    float4 minWorldPos = mul(boundsMin, worldMatrix);
    output.boundMinWorldPosition = minWorldPos.xyz / minWorldPos.w;
    float4 maxWorldPos = mul(boundsMax, worldMatrix);
    output.boundMaxWorldPosition = maxWorldPos.xyz / maxWorldPos.w;

    return output;
}