// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{

}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Initalise scene variables.

	// Load textures
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	
	// Initialise shaders
	textureShader = make_unique<TextureShader>(renderer->getDevice(), hwnd);
	worleyComputeShader = make_unique<WorleyComputeShader>(renderer->getDevice(), hwnd, numCellsPerAxis, numCellsPerAxis, numCellsPerAxis);

	// Initialise meshes
	planeMesh = make_unique<PlaneMesh>(renderer->getDevice(), renderer->getDeviceContext());

}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	firstPass();
	computePass();
	finalPass();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::firstPass()
{
}

void App1::computePass()
{
	worleyComputeShader->setShaderParameters(renderer->getDeviceContext());
	worleyComputeShader->compute(renderer->getDeviceContext(), numCellsPerAxis / Num_Threads_Per_Group.x, numCellsPerAxis / Num_Threads_Per_Group.y, numCellsPerAxis / Num_Threads_Per_Group.z);
	worleyComputeShader->unbind(renderer->getDeviceContext());
}

void App1::finalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render objects in scene
	{
		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		planeMesh->sendData(renderer->getDeviceContext());
		textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
		textureShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());
	}


	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

