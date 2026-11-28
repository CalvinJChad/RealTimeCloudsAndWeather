// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
// Shaders
#include "TextureShader.h"
#include "WorleyComputeShader.h"
#include "PerlinWorleyComputeShader.h"
#include "ViewWorleySliceShader.h"
#include "BaseDepthShader.h"
#include "VolumetricCloudsShader.h"
// Other stuff
#include "BoundingBoxVolume.h"

using namespace std;

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

	// Pipeline stages
	void firstPass();	/* Currently empty */

	void computePass();	/* Compute worley noise */

	void cameraDepthPass();	/* Render scene from the perspective of camera to get depth value of objects */
	void finalScenePass();	/* Render the scene */

	void worleySlicePass();	/* Render slice of the generated 3D Worley noise texture */
	void finalPass(RenderTexture* finalRT);	/* Renders passed in RT onto ortho mesh. Else, calls 'finalscenepass' */

	enum Active_View_Type : uint32_t
	{
		ACTIVE_VIEW_SCENE = 0,
		ACTIVE_VIEW_DEBUG_NOISE
	};

private:
	// Shaders
	unique_ptr<TextureShader> textureShader;
	unique_ptr<BaseDepthShader> depthShader;
	unique_ptr<WorleyComputeShader> worleyComputeShader;
	unique_ptr<PerlinWorleyComputeShader> perlinWorleyComputeShader;
	unique_ptr<ViewWorleySliceShader> viewWorleySliceShader;
	unique_ptr<VolumetricCloudsShader> volumetricCloudsShader;

	// Meshes
	unique_ptr<CubeMesh> testCube;
	unique_ptr<BoundingBoxVolume> renderVolume;
	unique_ptr<PlaneMesh> planeMesh;
	unique_ptr<OrthoMesh> orthoMesh;

	// Render Textures
	unique_ptr<RenderTexture> worleySliceViewRT;

	// ShadowMap Test
	unique_ptr<ShadowMap> cameraDepthMap;

	// Background
	XMFLOAT4 backgroundColour = backgroundColour = XMFLOAT4(0.39f, 0.58f, 0.92f, 1.0f);

	// Cloud Generation
	const int numCellsPerAxis = 128;

	// ImGui Values
	bool viewerGreyscale = true;
	bool viewerShowAllChannels = false;
	float viewerSliceDepth = 0.0f;
	/* ImGui enum selector values */
	Active_View_Type activeView = ACTIVE_VIEW_SCENE;
	const char* viewNames[2] = { "Scene", "Noise Debugger" };
	Active_Channel_Type activeChannel = ACTIVE_CHANNEL_R;
	const char* channelNames[4] = { "R", "G", "B", "A" };	// Channel names used by ImGui Combo Box for Active Noise Channel.
	/* Render volume values */
	XMFLOAT3 renderVolumePosition = XMFLOAT3(0.f, 10.f, 30.f);
	XMFLOAT3 renderVolumeRotation = XMFLOAT3(0.f, 0.f, 0.f);
	XMFLOAT3 renderVolumeExtent = XMFLOAT3(10.0f, 10.0f, 10.0f);
};

static const XMINT3 Num_Threads_Per_Group = XMINT3(8, 8, 8);	// Make sure this is up-to-date with cs number of threads defenition.

#endif