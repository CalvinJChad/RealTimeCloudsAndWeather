// Colour shader.h
// Simple shader example.
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;


class BaseDepthShader : public BaseShader
{

public:

	BaseDepthShader(ID3D11Device* device, HWND hwnd);
	~BaseDepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
};

