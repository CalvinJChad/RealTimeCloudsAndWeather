#pragma once
#include "BaseShader.h"
class ViewWorleySliceShader : public BaseShader
{
private:
	struct ViewWorleySliceBufferType
	{
		BOOL enableGreyscale;	// c++ bool is 1 byte, whereas hlsl bool is 4.
		BOOL enableShowAllChannels;
		float sliceDepth;
		float padding;
	};

public:
	ViewWorleySliceShader(ID3D11Device* device, HWND hwnd);
	~ViewWorleySliceShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, bool viewerGreyscale, bool viewerShowAllChannels, float viewerSliceDepth);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* viewWorleySliceBuffer;
};

