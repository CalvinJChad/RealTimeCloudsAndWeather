// Cloud density pixel/fragment shader
// Fragment shader for rendering clouds (currently only density calculations & not lighting calculations)

// =====================================================================================
// Code from Sebastian Lague's 'Coding Adventure: Clouds' https://github.com/SebLague/Clouds
// =====================================================================================

// Buffer for bounding box volume attributes.
cbuffer BoundingBoxVolumeBuffer : register(b0)
{
    float3 boundsMin;
    float padding0;
    float3 boundsMax;
    float padding1;
};

// Texture and sampler registers
//Texture2D mainTexture : register(t0);
//SamplerState mainSampler : register(s0);
Texture2D cameraDepthTexture : register(t0);
SamplerState depthSampler : register(s0);
Texture3D shapeNoiseTexture : register(t1);
SamplerState shapeNoiseSampler : register(s1);

// Variables that I need to a buffer at some point (ImGui values)
const float3 cloudOffset;
const float cloudScale;
const float densityThreshold;
const float densityMultiplier;
const int numSteps = 1;

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;       // Position of the vertex in the world
    float3 cameraWorldPosition : TEXCOORD2; // The position of the camera in world space
    float3 viewVector : TEXCOORD3;          // The view vector of the camera
};

float2 getProjectiveCoords(float4 position)
{
    // Calculate the projected texture coordinates.
    float2 projTex = position.xy / position.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
float2 rayBoxDst(float3 boundsMin, float3 boundsMax, float3 rayOrigin, float3 invRaydir)
{
    // Adapted from: http://jcgt.org/published/0007/03/04/
    float3 t0 = (boundsMin - rayOrigin) * invRaydir;
    float3 t1 = (boundsMax - rayOrigin) * invRaydir;
    float3 tmin = min(t0, t1);
    float3 tmax = max(t0, t1);
                
    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection

    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection

    // CASE 3: ray misses box (dstA > dstB)

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return float2(dstToBox, dstInsideBox);
}

float sampleDensity(float3 position)
{
    float3 uvw = position * cloudScale * 0.001f + cloudOffset * 0.01f;
    float4 shape = shapeNoiseTexture.Sample(shapeNoiseSampler, uvw);
    float density = max(0, shape.r - densityThreshold) * densityMultiplier;
    return density;
}

float4 main(InputType input) : SV_TARGET
{
    // Get the dimensions of the Output Texture.
    uint tWidth, tHeight, tDepth;
    shapeNoiseTexture.GetDimensions(tWidth, tHeight, tDepth);
    
    float2 uvDepthPos = getProjectiveCoords(input.position);
    float depthValue = cameraDepthTexture.Sample(depthSampler, uvDepthPos).r;
    
    float3 rayOrigin = input.cameraWorldPosition;
    float3 rayDirection = input.viewVector;
    
    float2 rayBoxInfo = rayBoxDst(boundsMin, boundsMax, rayOrigin, rayDirection);
    float dstToBox = rayBoxInfo.x;
    float dstInBox = rayBoxInfo.y;
    
    float dstTravelled = 0.f;
    float stepSize = dstInBox / numSteps;
    //float dstLimit = min()
    
    bool rayHitBox = dstInBox > 0.f && dstToBox < depthValue;
    if (rayHitBox)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f);

}