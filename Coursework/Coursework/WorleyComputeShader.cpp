#include "WorleyComputeShader.h"

WorleyComputeShader::WorleyComputeShader(ID3D11Device* device, HWND hwnd, int w, int h, int d)
	: BaseShader(device, hwnd)
{
	sWidth = w;
	sHeight = h;
	sDepth = d;
	initShader(L"worleyNoise_cs.cso", NULL);
}

WorleyComputeShader::~WorleyComputeShader()
{
	if (srv)
	{
		srv->Release();
		srv = 0;
	}
	if (uav)
	{
		uav->Release();
		uav = 0;
	}
	if (m_tex)
	{
		m_tex->Release();
		m_tex = 0;
	}
	if (srv)
	{
		srv->Release();
		srv = 0;
	}
	if (m_srvTexOutput)
	{
		m_srvTexOutput->Release();
		m_srvTexOutput = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void WorleyComputeShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile);
	createOutputUAV();
}

void WorleyComputeShader::createOutputUAV()
{
	D3D11_TEXTURE3D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = sWidth;
	textureDesc.Height = sHeight;
	textureDesc.Depth = sDepth;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_tex = 0;
	renderer->CreateTexture3D(&textureDesc, 0, &m_tex);

	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	//descUAV.Texture2D.MipSlice = 0;
	descUAV.Texture3D.MipSlice = 0;
	descUAV.Texture3D.WSize = sDepth;
	renderer->CreateUnorderedAccessView(m_tex, &descUAV, &m_uavAccess);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	/*srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;*/
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;
	renderer->CreateShaderResourceView(m_tex, &srvDesc, &m_srvTexOutput);
}

void WorleyComputeShader::setShaderParameters(ID3D11DeviceContext* dc)
{
	/*dc->CSSetShaderResources(0, 1, &texture1);*/
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);
}

void WorleyComputeShader::unbind(ID3D11DeviceContext* dc)
{
	/*ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);*/

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}
