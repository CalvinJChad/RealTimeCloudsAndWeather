// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
// Shaders
#include "TextureShader.h"
#include "WorleyComputeShader.h"
#include "ViewWorleySliceShader.h"

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
	void finalScenePass();	/* Render the scene */
	void worleySlicePass();	/* Render slice of the generated 3D Worley noise texture */
	void finalPass(RenderTexture* finalRT);	/* Renders passed in RT onto ortho mesh. Else, calls 'finalscenepass' */

private:
	// Shaders
	unique_ptr<TextureShader> textureShader;
	unique_ptr<WorleyComputeShader> worleyComputeShader;
	unique_ptr<ViewWorleySliceShader> viewWorleySliceShader;

	// Meshes
	unique_ptr<PlaneMesh> planeMesh;
	unique_ptr<OrthoMesh> orthoMesh;

	// Render Textures
	unique_ptr<RenderTexture> worleySliceViewRT;

	// Background
	XMFLOAT4 backgroundColour = backgroundColour = XMFLOAT4(0.39f, 0.58f, 0.92f, 1.0f);

	// Cloud Generation
	const int numCellsPerAxis = 128;

	// ImGui Values
	bool viewerGreyscale = true;
	bool viewerShowAllChannels = false;
	float viewerSliceDepth = 0.0f;
};

#endif