#pragma once
#include "BaseShader.h"

enum Active_Channel_Type : uint32_t
{
	ACTIVE_CHANNEL_R = 0,
	ACTIVE_CHANNEL_G,
	ACTIVE_CHANNEL_B,
	ACTIVE_CHANNEL_A
};

class ViewWorleySliceShader : public BaseShader
{
private:
	struct ViewWorleySliceBufferType
	{
		BOOL enableGreyscale;	// c++ bool is 1 byte, whereas hlsl bool is 4.
		BOOL enableShowAllChannels;
		float sliceDepth;
		uint32_t activeChannelType;
	};

public:
	ViewWorleySliceShader(ID3D11Device* device, HWND hwnd);
	~ViewWorleySliceShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, bool viewerGreyscale, bool viewerShowAllChannels, float viewerSliceDepth, Active_Channel_Type activeChannelType);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

	ID3D11Buffer* matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11Buffer* viewWorleySliceBuffer;
};

