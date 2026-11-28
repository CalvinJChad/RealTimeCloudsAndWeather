// texture vertex shader
// Basic shader for rendering textured geometry

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer BoundingBoxVolumeBuffer : register(b1)
{
    float4 boundsMin;   // x, y, z, 1
    float4 boundsMax;   // x, y, z, 1
};

cbuffer CameraBuffer : register(b2)
{
    float3 cameraPosition;
    float cameraPadding;
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
    float3 worldPosition : TEXCOORD1;           // Position of the vertex in world space
    float3 cameraWorldPosition : TEXCOORD2;     // The position of the camera in world space
    float3 viewVector : TEXCOORD3;              // The view vector of the camera
    float3 boundMinWorldPosition : TEXCOORD4;   // Position of the minimum bounds of the bounding box volume in world space
    float3 boundMaxWorldPosition : TEXCOORD5;   // Position of the maximum bounds of the bounding box volume in world space
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
    // The position of the camera should already been in regards to world space so we don't need to do any multiplications.
    output.cameraWorldPosition = cameraPosition;
    // Get the position of the lower and uppper bounds of the box volume in world space.
    float4 minWorldPos = mul(boundsMin, worldMatrix);
    output.boundMinWorldPosition = minWorldPos.xyz / minWorldPos.w;
    float4 maxWorldPos = mul(boundsMax, worldMatrix);
    output.boundMaxWorldPosition = maxWorldPos.xyz / minWorldPos.w;
    
    // Calculate the view vector of the camera
    output.viewVector = normalize(output.worldPosition.xyz - cameraPosition.xyz);

    return output;
}