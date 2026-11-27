#include "BoundingBoxVolume.h"

BoundingBoxVolume::BoundingBoxVolume(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width, float height, float depth)
{
	volumeWidth = width;
	volumeHeight = height;
	volumeDepth = depth;
	initBuffers(device);
}

BoundingBoxVolume::~BoundingBoxVolume()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

void BoundingBoxVolume::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
{
    unsigned int stride;
    unsigned int offset;

    // Set vertex buffer stride and offset.
    stride = sizeof(VertexType_Texture);
    offset = 0;

    deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    deviceContext->IASetPrimitiveTopology(top);
}

void BoundingBoxVolume::updateVolume(ID3D11Device* device, float width, float height, float depth)
{
    if (width == volumeWidth && height == volumeHeight && depth == volumeDepth) return;

    volumeWidth = width;
    volumeHeight = height;
    volumeDepth = depth;
    initBuffers(device);    // Need to redefine vertex positions.
}

// Initialise geometry buffers (vertex and index).
// Generate and store box vertex and UV coordinates
void BoundingBoxVolume::initBuffers(ID3D11Device* device)
{
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// 6 vertices per cube face, 6 * 6 = 36 vertices for the box
    vertexCount = indexCount = 36;

	// Generate initial Vertex and UV Positions
    VertexType_Texture vertices[36] =
    {
        // Front Face (-Z)
        { XMFLOAT3(-1,-1,-1), XMFLOAT2(0,1) },  // Min Bounds
        { XMFLOAT3(1, 1,-1), XMFLOAT2(1,0) },
        { XMFLOAT3(-1, 1,-1), XMFLOAT2(0,0) },

        { XMFLOAT3(-1,-1,-1), XMFLOAT2(0,1) },
        { XMFLOAT3(1,-1,-1), XMFLOAT2(1,1) },
        { XMFLOAT3(1, 1,-1), XMFLOAT2(1,0) },


        // Back Face (+Z)
        { XMFLOAT3(1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(-1, 1, 1), XMFLOAT2(1,0) },
        { XMFLOAT3(1, 1, 1), XMFLOAT2(0,0) },   // Max Bounds

        { XMFLOAT3(1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(-1,-1, 1), XMFLOAT2(1,1) },
        { XMFLOAT3(-1, 1, 1), XMFLOAT2(1,0) },


        // Right Face (+X)
        { XMFLOAT3(1,-1,-1), XMFLOAT2(0,1) },
        { XMFLOAT3(1, 1, 1), XMFLOAT2(1,0) },
        { XMFLOAT3(1, 1,-1), XMFLOAT2(0,0) },

        { XMFLOAT3(1,-1,-1), XMFLOAT2(0,1) },
        { XMFLOAT3(1,-1, 1), XMFLOAT2(1,1) },
        { XMFLOAT3(1, 1, 1), XMFLOAT2(1,0) },


        // Left Face (-X)
        { XMFLOAT3(-1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(-1, 1,-1), XMFLOAT2(1,0) },
        { XMFLOAT3(-1, 1, 1), XMFLOAT2(0,0) },

        { XMFLOAT3(-1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(-1,-1,-1), XMFLOAT2(1,1) },
        { XMFLOAT3(-1, 1,-1), XMFLOAT2(1,0) },


        // Top Face (+Y)
        { XMFLOAT3(-1, 1,-1), XMFLOAT2(0,1) },
        { XMFLOAT3(1, 1, 1), XMFLOAT2(1,0) },
        { XMFLOAT3(-1, 1, 1), XMFLOAT2(0,0) },

        { XMFLOAT3(-1, 1,-1), XMFLOAT2(0,1) },
        { XMFLOAT3(1, 1,-1), XMFLOAT2(1,1) },
        { XMFLOAT3(1, 1, 1), XMFLOAT2(1,0) },


        // Bottom Face (-Y)
        { XMFLOAT3(-1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(1,-1,-1), XMFLOAT2(1,0) },
        { XMFLOAT3(-1,-1,-1), XMFLOAT2(0,0) },

        { XMFLOAT3(-1,-1, 1), XMFLOAT2(0,1) },
        { XMFLOAT3(1,-1, 1), XMFLOAT2(1,1) },
        { XMFLOAT3(1,-1,-1), XMFLOAT2(1,0) }
    };

    float widthOffset = volumeWidth * 0.5f;
    float heightOffset = volumeHeight * 0.5f;
    float depthOffset = volumeDepth * 0.5f;

    // Loop through each of the vertices and multiply them by the defined width, height, and depth.
    // Store the indices.
    unsigned long indices[36];
    for (size_t i = 0; i < 36; i++)
    {
        if (volumeWidth != 1.0f || volumeHeight != 1.0f || volumeDepth != 1.0f)
        {
            XMVECTOR v1 = XMLoadFloat3(&vertices[i].position);
            XMVECTOR v2 = XMLoadFloat3(&XMFLOAT3(widthOffset, heightOffset, depthOffset));
            XMVECTOR vResult = XMVectorMultiply(v1, v2);

            XMStoreFloat3(&vertices[i].position, vResult);
        }
        
        indices[i] = i;
    }

    // Get the lower bound (bottom left of front face) and upper bound (top right of front face) of the box.
    boundsMin = vertices[0].position;
    boundsMax = vertices[8].position;

    // Set up the description of the static vertex buffer.
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.ByteWidth = sizeof(VertexType_Texture) * vertexCount;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the vertex data.
    vertexData.pSysMem = vertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;
    // Now create the vertex buffer.
    device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

    // Set up the description of the static index buffer.
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;
    // Give the subresource structure a pointer to the index data.
    indexData.pSysMem = indices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;
    // Create the index buffer.
    device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);
}
