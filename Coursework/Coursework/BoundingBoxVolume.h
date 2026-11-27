/**
* \class Bounding Box Volume
*
* \brief Simple box volume object
*
* Inherits from Base Mesh, Builds a simple box with texture coordinates and normals.
*
* \using code from Paul Robertson's Cube Mesh as an example
*/

#include "BaseMesh.h"

using namespace DirectX;

#pragma once
class BoundingBoxVolume : public BaseMesh
{
public:
	/** \brief Initialises and builds a volume box
	*
	* @param device is the renderer device
	* @param device context is the renderer device context
	* @param depth is the depth of the box volume (not including scale matrix)
	* @param height is the height of the box volume (not including scale matrix)
	* @param depth is the depth of the box volume (not including scale matrix)
	*/
	BoundingBoxVolume(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width = 1.0f, float height = 1.0f, float depth = 1.0f);
	~BoundingBoxVolume();

	/// Transfers mesh data to the GPU.
	void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) override;

	void updateVolume(ID3D11Device* device, float width, float height, float depth);

	inline XMFLOAT3 getMinBounds() const { return boundsMin; };
	inline XMFLOAT3 getMaxBounds() const { return boundsMax; }

protected:
	void initBuffers(ID3D11Device* device);
	const int resolution = 1;

	// Define the depth and height of the box
	float volumeWidth;
	float volumeHeight;
	float volumeDepth;
	// Define the lower bound (bottom left of front face) and upper bound (top right of front face) of the box
	XMFLOAT3 boundsMin;
	XMFLOAT3 boundsMax;
};

