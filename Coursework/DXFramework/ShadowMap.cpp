#include "ShadowMap.h"
#include <stdexcept>

ShadowMap::ShadowMap(ID3D11Device* device, int mWidth, int mHeight)
	: mDepthMapDSV(nullptr), mDepthMapSRV(nullptr), depthMap(nullptr)
{
	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
	// the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mWidth;
	texDesc.Height = mHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// Use hit resukt to check if we've failed to create any of the necessary views.
	HRESULT hr;

	//ID3D11Texture2D* depthMap = 0;
	device->CreateTexture2D(&texDesc, 0, &depthMap);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = device->CreateDepthStencilView(depthMap, &dsvDesc, &mDepthMapDSV);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create depth stencil view. (ShadowMap.cpp)");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(depthMap, &srvDesc, &mDepthMapSRV);
	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create shader resource view. (ShadowMap.cpp)");
	}

	// Setup the viewport for rendering.
	viewport.Width = (float)mWidth;
	viewport.Height = (float)mHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//NULL render target
	//renderTargets[0] = { nullptr };
}

ShadowMap::~ShadowMap()
{
	if (mDepthMapDSV)
	{
		mDepthMapDSV->Release();
		mDepthMapDSV = nullptr;
	}

	if (mDepthMapSRV)
	{
		mDepthMapSRV->Release();
		mDepthMapSRV = nullptr;
	}
	
	//if (renderTargets)
	//{
	//	for (auto* renderTarget : renderTargets)
	//	{
	//		if (renderTarget)
	//		{
	//			renderTarget->Release();
	//			renderTarget = nullptr;
	//		}
	//	}

	//	//delete[] renderTargets;
	//}

	if (depthMap)
	{
		depthMap->Release();
		depthMap = nullptr;
	}
}

void ShadowMap::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc)
{
	dc->RSSetViewports(1, &viewport);

	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
	ID3D11RenderTargetView* nullRTV[1] = { nullptr };
	dc->OMSetRenderTargets(1, nullRTV, mDepthMapDSV);

	dc->ClearDepthStencilView(mDepthMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
}
