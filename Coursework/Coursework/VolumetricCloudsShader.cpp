#include "VolumetricCloudsShader.h"

VolumetricCloudsShader::VolumetricCloudsShader(ID3D11Device* device, HWND hwnd)
	: BaseShader(device, hwnd)
{
	initShader(L"cloud_vs.cso", L"cloud_ps.cso");
}

VolumetricCloudsShader::~VolumetricCloudsShader()
{
	// Release the sampler state.
	if (depthSampleState)
	{
		depthSampleState->Release();
		depthSampleState = 0;
	}

	if (noiseSampleState)
	{
		noiseSampleState->Release();
		noiseSampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	if (cameraBuffer)
	{
		cameraBuffer->Release();
		cameraBuffer = 0;
	}

	if (boundingBoxBuffer)
	{
		boundingBoxBuffer->Release();
		boundingBoxBuffer = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}

void VolumetricCloudsShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	D3D11_BUFFER_DESC matrixBufferDesc;

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description for the camera depth texture.
	D3D11_SAMPLER_DESC depthSamplerDesc;
	depthSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	depthSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	depthSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	depthSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	depthSamplerDesc.BorderColor[0] = 1.0f;
	depthSamplerDesc.BorderColor[1] = 1.0f;
	depthSamplerDesc.BorderColor[2] = 1.0f;
	depthSamplerDesc.BorderColor[3] = 1.0f;
	depthSamplerDesc.MipLODBias = 0.0f;
	depthSamplerDesc.MaxAnisotropy = 1;
	depthSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	depthSamplerDesc.MinLOD = 0;
	depthSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&depthSamplerDesc, &depthSampleState);

	// Create a texture sampler state description for the 3D Perlin-Worley noise texture.
	D3D11_SAMPLER_DESC noiseSamplerDesc;
	noiseSamplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	noiseSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	noiseSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	noiseSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	noiseSamplerDesc.MipLODBias = 0.0f;
	noiseSamplerDesc.MaxAnisotropy = 1;
	noiseSamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	noiseSamplerDesc.MinLOD = 0;
	noiseSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&noiseSamplerDesc, &noiseSampleState);

	// Setup camera buffer (for vertex shader)
	D3D11_BUFFER_DESC cameraBufferDesc;
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = sizeof(CameraBufferType);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraBufferDesc.MiscFlags = 0;
	cameraBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&cameraBufferDesc, NULL, &cameraBuffer);

	// Setup bounding box buffer (for pixel shader)
	D3D11_BUFFER_DESC boundingBoxBufferDesc;
	boundingBoxBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	boundingBoxBufferDesc.ByteWidth = sizeof(BoundingBoxVolumeBufferType);
	boundingBoxBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	boundingBoxBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	boundingBoxBufferDesc.MiscFlags = 0;
	boundingBoxBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&boundingBoxBufferDesc, NULL, &boundingBoxBuffer);

}

void VolumetricCloudsShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& worldMatrix, const XMMATRIX& viewMatrix, const XMMATRIX& projectionMatrix, XMFLOAT3 minBounds, XMFLOAT3 maxBounds, ID3D11ShaderResourceView* shapeNoiseTexture, XMFLOAT3 cameraPos, ShadowMap* cameraDepthMap)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Send camera data to vertex shader
	{
		result = deviceContext->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CameraBufferType* cameraPtr = (CameraBufferType*)mappedResource.pData;
		cameraPtr->cameraPosition = cameraPos;
		cameraPtr->padding = 0.0f;
		deviceContext->Unmap(cameraBuffer, 0);
		deviceContext->VSSetConstantBuffers(1, 1, &cameraBuffer);
	}

	// Send bounding box data to pixel shader
	{
		result = deviceContext->Map(boundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		BoundingBoxVolumeBufferType* boxData = (BoundingBoxVolumeBufferType*)mappedResource.pData;
		boxData->boundsMin = minBounds;
		boxData->boundsMax = maxBounds;
		boxData->padding0 = boxData->padding1 = 0.f;
		deviceContext->Unmap(boundingBoxBuffer, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &boundingBoxBuffer);
	}

	ID3D11ShaderResourceView* cameraDepthTexture = cameraDepthMap->getDepthMapSRV();

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &cameraDepthTexture);	// I initially had 'PSGetShaderResources' instead of 'PSSetShaderResources'. So I spent 5 hours trying to figure out why the brick texture was being passed into this stupid shader.
	deviceContext->PSSetSamplers(0, 1, &depthSampleState);
	deviceContext->PSSetShaderResources(1, 1, &shapeNoiseTexture);
	deviceContext->PSSetSamplers(1, 1, &noiseSampleState);
}
