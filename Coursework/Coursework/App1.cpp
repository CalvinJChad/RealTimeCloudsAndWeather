// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "FPCamera.h"

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
	depthShader = make_unique<BaseDepthShader>(renderer->getDevice(), hwnd);
	worleyComputeShader = make_unique<WorleyComputeShader>(renderer->getDevice(), hwnd, numCellsPerAxis, numCellsPerAxis, numCellsPerAxis);
	perlinWorleyComputeShader = make_unique<PerlinWorleyComputeShader>(renderer->getDevice(), hwnd, numCellsPerAxis, numCellsPerAxis, numCellsPerAxis);
	viewWorleySliceShader = make_unique<ViewWorleySliceShader>(renderer->getDevice(), hwnd);

	// Initialise meshes
	planeMesh = make_unique<PlaneMesh>(renderer->getDevice(), renderer->getDeviceContext());
	orthoMesh = make_unique<OrthoMesh>(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);	// Assuming I need to make the ortho mesh the same size as the camera.

	// Initialise RT objects. One needed to store the worley slice view.
	worleySliceViewRT = make_unique<RenderTexture>(renderer->getDevice(), numCellsPerAxis, numCellsPerAxis, 0.1f, 100.f);	// Assuming I make the render texture the same size as the Worley noise texture.
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

	// Make sure ImGui values are properly clamped.
	{
		// Quote from my cmp301 cw source code: 'CAN'T USE STD CLAMP YAAAAAAAAAAAAAAY!'
		/*viewerSliceDepth = clamp(viewerSliceDepth, 0.0f, 1.0f);*/
		if (viewerSliceDepth < 0.0f) viewerSliceDepth = 0.0f;
		else if (viewerSliceDepth > 1.0f) viewerSliceDepth = 1.0f;
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
	// Generate the view matrix based on the camera's position.
	camera->update();

	firstPass();
	computePass();	// Compute Worley noise.

	switch (activeView)
	{
	case App1::ACTIVE_VIEW_SCENE:
		cameraDepthPass();
		finalScenePass();

		break;
	case App1::ACTIVE_VIEW_DEBUG_NOISE:
		worleySlicePass();
		finalPass(worleySliceViewRT.get());

		break;
	}

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();

	return true;
}

void App1::firstPass()
{
}

void App1::computePass()
{
	/*worleyComputeShader->setShaderParameters(renderer->getDeviceContext());
	worleyComputeShader->compute(renderer->getDeviceContext(), numCellsPerAxis / Num_Threads_Per_Group.x, numCellsPerAxis / Num_Threads_Per_Group.y, numCellsPerAxis / Num_Threads_Per_Group.z);
	worleyComputeShader->unbind(renderer->getDeviceContext());*/

	perlinWorleyComputeShader->setShaderParameters(renderer->getDeviceContext());
	perlinWorleyComputeShader->compute(renderer->getDeviceContext(), numCellsPerAxis / Num_Threads_Per_Group.x, numCellsPerAxis / Num_Threads_Per_Group.y, numCellsPerAxis / Num_Threads_Per_Group.z);
	perlinWorleyComputeShader->unbind(renderer->getDeviceContext());
}

void App1::cameraDepthPass()
{
	// Set the render target to be the render to this camera's depth map texture.
	camera->getCameraDepthMap()->BindDsvAndSetNullRenderTarget(renderer->getDeviceContext());

	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render objects in scene
	{
		worldMatrix = XMMatrixTranslation(-50.f, 0.f, -10.f);
		// Render floor
		planeMesh->sendData(renderer->getDeviceContext());
		depthShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix);
		depthShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());
	}

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalScenePass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(backgroundColour.x, backgroundColour.y, backgroundColour.z, backgroundColour.w);

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

}

void App1::worleySlicePass()
{
	//if (!worleyComputeShader || worleyComputeShader->getSRV()) return;

	// Rendering to the Worley slice texture.
	worleySliceViewRT->setRenderTarget(renderer->getDeviceContext());
	worleySliceViewRT->clearRenderTarget(renderer->getDeviceContext(), backgroundColour.x, backgroundColour.y, backgroundColour.z, backgroundColour.w);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();	// ortho matrix for 2D rendering
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());

	viewWorleySliceShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, perlinWorleyComputeShader->getSRV(), viewerGreyscale, viewerShowAllChannels, viewerSliceDepth, activeChannel);
	viewWorleySliceShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::finalPass(RenderTexture* finalRT)
{
	// If passed in render texture is invalid, render the final scene instead.
	if (!finalRT || !finalRT->getShaderResourceView())
	{
		// Doesn't really work cause the SRV will always be valid if the RT is valid.
		finalScenePass();
		return;
	}

	// Render passed in render texture onto an ortho mesh.
	renderer->beginScene(backgroundColour.x, backgroundColour.y, backgroundColour.z, backgroundColour.w);	// Clear the scene. (default blue colour)

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();// ortho matrix for 2D rendering
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);
	orthoMesh->sendData(renderer->getDeviceContext());

	textureShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, finalRT->getShaderResourceView());
	textureShader->render(renderer->getDeviceContext(), orthoMesh->getIndexCount());

	renderer->setZBuffer(true);
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

	// Select view type
	{
		const char* previewValue = viewNames[static_cast<int>(activeView)];
		if (ImGui::BeginCombo("Select View", previewValue)) {
			for (int i = 0; i < IM_ARRAYSIZE(viewNames); i++)
			{
				const bool isSelected = (static_cast<int>(activeView) == i);

				if (ImGui::Selectable(viewNames[i], isSelected))
				{
					activeView = static_cast<Active_View_Type>(i);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation)
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}

	if (ImGui::CollapsingHeader("Debug Worley Noise"))
	{
		// Select active channel.
		const char* previewValue = channelNames[static_cast<int>(activeChannel)];
		if (ImGui::BeginCombo("Render Mode", previewValue)) {
			for (int i = 0; i < IM_ARRAYSIZE(channelNames); i++) {
				const bool isSelected = (static_cast<int>(activeChannel) == i);

				if (ImGui::Selectable(channelNames[i], isSelected)) {
					activeChannel = static_cast<Active_Channel_Type>(i);
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation)
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Checkbox("Viewer Greyscale", &viewerGreyscale);
		ImGui::Checkbox("Viewer Show All Channels", &viewerShowAllChannels);
		ImGui::DragFloat("Viewer Slice Depth", &viewerSliceDepth, 0.01f, 0.0f, 1.0f);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

