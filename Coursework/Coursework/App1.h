// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"
// Shaders
#include "TextureShader.h"
#include "WorleyComputeShader.h"

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
	void computePass();	/* Compute worley noise*/
	void finalPass();	/* Render the scene */

private:
	// Shaders
	unique_ptr<TextureShader> textureShader;
	unique_ptr<WorleyComputeShader> worleyComputeShader;

	// Meshes
	unique_ptr<PlaneMesh> planeMesh;

	// Cloud Generation
	const int numCellsPerAxis = 128;
};

#endif