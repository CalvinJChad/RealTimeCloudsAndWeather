// Cloud density pixel/fragment shader
// Fragment shader for rendering clouds (currently only density calculations & not lighting calculations)

// =====================================================================================
// Code from Sebastian Lague's 'Coding Adventure: Clouds' https://github.com/SebLague/Clouds
// =====================================================================================

//#define DEBUG_DEPTH

cbuffer CameraBuffer : register(b0)
{
    float3 cameraPosition;  // The position of the camera in world space.
    float cameraPadding;
};

// Need the inverse of the projection matrix used in the camera depth pass
// to calculate the linear depth value.
cbuffer InverseMatrixBuffer : register(b1)
{
    matrix invProjectionMatrix;
};

cbuffer CloudBuffer : register(b2)
{
    float3 cloudOffset;
    float cloudScale;
    float densityThreshold;
    float densityMultiplier;
    int numSteps;
    float padding;
}

// Texture and sampler registers
//Texture2D mainTexture : register(t0);
//SamplerState mainSampler : register(s0);
Texture2D cameraDepthTexture : register(t0);
SamplerState depthSampler : register(s0);
Texture3D shapeNoiseTexture : register(t1);
SamplerState shapeNoiseSampler : register(s1);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 worldPosition : TEXCOORD1;           // Position of the vertex in world space
    float3 boundMinWorldPosition : TEXCOORD2;   // Position of the minimum bounds of the bounding box volume in world space
    float3 boundMaxWorldPosition : TEXCOORD3;   // Position of the maximum bounds of the bounding box volume in world space
};

float2 getUVCoords(float2 pixelScreenPosition)
{
    // Get the dimensions of the Depth Texture.
    uint dWidth, dHeight;
    cameraDepthTexture.GetDimensions(dWidth, dHeight);
    
    float2 uvDepthPos = pixelScreenPosition / float2(dWidth, dHeight);
    return uvDepthPos;
}

// Depth value stored in camera depth texture is nonlinear (in the range [0, 1]).
// To get the linear depth value, we can multiply the native device coordinates (ndc) by the inverse of the projection matrix
// used in the depth pass (should be the same one used in this shader's vs) to linearize the value (get Z value in world space).
// Code from Pascal Rosenkranz's 'prkz.de blog' https://prkz.de/blog/6-linearizing-depth-buffer-samples-in-hlsl
float getLinearDepthValue(float nonLinearDepthValue)
{
    // We are only interested in the depth here
    float4 ndcCoords = float4(0, 0, nonLinearDepthValue, 1.0f);
    // Unproject the vector into (homogenous) view-space vector
    float4 viewCoords = mul(invProjectionMatrix, ndcCoords);
    // Divide by w, which results in actual view-space z value
    float linearDepth = viewCoords.z / viewCoords.w;
    return linearDepth;
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
    float4 shape = shapeNoiseTexture.Sample(shapeNoiseSampler, uvw, 0);
    float density = max(0, shape.r - densityThreshold) * densityMultiplier;
    return density;
}

float4 main(InputType input) : SV_TARGET
{
    // Get the dimensions of the Noise Texture.
    uint nWidth, nHeight, nDepth;
    shapeNoiseTexture.GetDimensions(nWidth, nHeight, nDepth);

    // Get nonlinear depth value from depth map.
    float2 uvDepthPos = getUVCoords(input.position.xy);
    float nonLinearDepthValue = cameraDepthTexture.Sample(depthSampler, uvDepthPos).r;
    float depthValue = getLinearDepthValue(nonLinearDepthValue);
    
#ifdef DEBUG_DEPTH
    return float4(depthValue, depthValue, depthValue, 1.0f);
#endif
    
    float3 rayOrigin = cameraPosition;
    float3 rayDirection = normalize(input.worldPosition - rayOrigin);
    
    float2 rayBoxInfo = rayBoxDst(input.boundMinWorldPosition, input.boundMaxWorldPosition, rayOrigin, (1.0f / rayDirection));
    float dstToBox = rayBoxInfo.x;
    float dstInBox = rayBoxInfo.y;
    
    // Get point of intersection with the cloud volume.
    float3 entryPoint = rayOrigin + rayDirection * dstToBox;
    
    float dstTravelled = 0.f;
    float stepSize = dstInBox / numSteps;
    float dstLimit = min(depthValue - dstToBox, dstInBox);
    
    // March through volume
    float totalDensity = 0.f;
    [unroll(200)]   // Need to specify a max number of times we can step through the cloud volume.
    for (uint i = 0; i < numSteps; i++)
    {
        if (dstTravelled >= dstLimit)
            break;  // Early exit
        
        float3 rayPos = entryPoint + rayDirection * dstTravelled;

        totalDensity += sampleDensity(rayPos);

        dstTravelled += stepSize;
    }
    float transmittance = exp(-totalDensity);
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f) * transmittance;

}