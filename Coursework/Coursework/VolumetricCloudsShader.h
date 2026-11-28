#pragma once

#include "BaseShader.h"
#include "ShadowMap.h"

using namespace std;
using namespace DirectX;

class VolumetricCloudsShader : public BaseShader
{
private:

	// Vertex Shader

	struct BoundingBoxVolumeBufferType
	{
		XMFLOAT4 boundsMin;
		XMFLOAT4 boundsMax;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	// Pixel/Fragment Shader
	//...

public:
	VolumetricCloudsShader(ID3D11Device* device, HWND hwnd);
	~VolumetricCloudsShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 minBounds, XMFLOAT3 maxBounds, ID3D11ShaderResourceView* shapeNoiseTexture, XMFLOAT3 cameraPos, ShadowMap* cameraDepthMap);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* noiseSampleState;
	ID3D11SamplerState* depthSampleState;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* boundingBoxBuffer;
};

