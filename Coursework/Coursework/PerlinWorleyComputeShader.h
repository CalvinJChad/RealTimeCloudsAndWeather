#pragma once

#include "DXF.h"
#include "BaseShader.h"

using namespace std;
using namespace DirectX;

class PerlinWorleyComputeShader : public BaseShader
{
public:
	PerlinWorleyComputeShader(ID3D11Device* device, HWND hwnd, int w, int h, int d);
	~PerlinWorleyComputeShader();

	void createOutputUAV();
	void setShaderParameters(ID3D11DeviceContext* dc);
	void unbind(ID3D11DeviceContext* dc);

	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	// texture set
	ID3D11Texture3D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;
	int sDepth;
};

