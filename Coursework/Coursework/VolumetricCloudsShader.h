#pragma once

#include "BaseShader.h"
#include "ShadowMap.h"

using namespace std;
using namespace DirectX;

struct CloudData;

class VolumetricCloudsShader : public BaseShader
{
private:

	// Vertex Shader

	struct BoundingBoxVolumeBufferType
	{
		XMFLOAT4 boundsMin;
		XMFLOAT4 boundsMax;
	};

	// Pixel/Fragment Shader

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	struct InverseMatrixBufferType
	{
		XMMATRIX invProjection;
	};

	struct CloudBufferType
	{
		XMFLOAT3 cloudOffset;
		float cloudScale;
		float densityThreshold;
		float densityMultiplier;
		int numSteps;
		float padding;
	};

public:

	VolumetricCloudsShader(ID3D11Device* device, HWND hwnd);
	~VolumetricCloudsShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 minBounds, XMFLOAT3 maxBounds, ID3D11ShaderResourceView* shapeNoiseTexture, XMFLOAT3 cameraPos, ShadowMap* cameraDepthMap, CloudData cloudData);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* noiseSampleState;
	ID3D11SamplerState* depthSampleState;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* boundingBoxBuffer;
	ID3D11Buffer* inverseMatrixBuffer;
	ID3D11Buffer* cloudBuffer;
};

