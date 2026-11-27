#pragma once

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class VolumetricCloudsShader : public BaseShader
{
public:
	VolumetricCloudsShader(ID3D11Device* device, HWND hwnd);
	~VolumetricCloudsShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, XMFLOAT3 minBounds, XMFLOAT3 maxBounds);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleStateDepth;
};

