#pragma once

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"
#include "RayTracing.h"

#include <DirectXMath.h>
#include <random>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));

	CreateGeometry();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down

	Graphics::WaitForGPU();
}

// --------------------------------------------------------

// Loads the two basic shaders, then creates the root signature

// and pipeline state object for our very basic demo.

// --------------------------------------------------------

// --------------------------------------------------------
// Creates the geometry we're going to draw
// TODO: Split this into multiple specialized functions
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f) },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f) },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };


	// Create the two buffers

	vertexBuffer = Graphics::CreateStaticBuffer(sizeof(Vertex), ARRAYSIZE(vertices), vertices);

	indexBuffer = Graphics::CreateStaticBuffer(sizeof(unsigned int), ARRAYSIZE(indices), indices);


	// Set up the views

	vbView.StrideInBytes = sizeof(Vertex);

	vbView.SizeInBytes = sizeof(Vertex) * ARRAYSIZE(vertices);

	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();


	ibView.Format = DXGI_FORMAT_R32_UINT;

	ibView.SizeInBytes = sizeof(unsigned int) * ARRAYSIZE(indices);

	ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();

	// Create meshes
	std::shared_ptr<Mesh> cube = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str(), "Cube");
	std::shared_ptr<Mesh> sphere = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str(), "Sphere");
	std::shared_ptr<Mesh> cylinder = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str(), "Cylinder");
	std::shared_ptr<Mesh> torus = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str(), "Torus");
	std::shared_ptr<Mesh> helix = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str(), "Helix");

	meshes.push_back(cube);
	meshes.push_back(sphere);
	meshes.push_back(cylinder);
	meshes.push_back(torus);
	meshes.push_back(helix);

	std::shared_ptr<Material> floorMat = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT4(0.7f, 0.7f, 0.8f,0.1f));
	entities.push_back(std::make_shared<GameEntity>(cube, floorMat));

	entities[0].get()->GetTransform()->Scale(100, 100, 100);
	entities[0].get()->GetTransform()->MoveAbsolute(0, -110, 0);

	// Random setup
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> randDouble(0, 1);
	std::uniform_int_distribution<> randInt(0, 4);

	for (int i = 0; i < 20; i++) {
		std::shared_ptr<Material> newMat = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT4((float)randDouble(gen), (float)randDouble(gen), (float)randDouble(gen), (float)randDouble(gen)));

		entities.push_back(std::make_shared<GameEntity>(meshes[randInt(gen)], newMat));

		entities[i + 1].get()->GetTransform()->MoveAbsolute((float)((randDouble(gen) * 2.0 - 1) * 20), -5, 5 + (float)((randDouble(gen) * 2.0 - 1) * 20));
		//(randDouble(gen)/2 - 1) * 20
	}

	RayTracing::CreateEntityDataBuffer(entities);
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);

	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList(0);

	// Create camera
	cam = std::make_shared<Camera>(XMFLOAT3(0.0f, 0.0f, -1.0f), 5.0f, 1.0f, XM_PIDIV2, Window::AspectRatio());

	// Create lights
	Light dirLight1 = {};
	dirLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight1.Direction = XMFLOAT3(0.0f, -0.8f, 1.0f);
	dirLight1.Color = XMFLOAT3(1.0f, 0.3f, 0.6f);
	dirLight1.Intensity = 1.0f;

	Light dirLight2 = {};
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Direction = XMFLOAT3(0.3f, 0.0f, 0.8f);
	dirLight2.Color = XMFLOAT3(0.1f, 1.0f, 0.1f);
	dirLight2.Intensity = 1.0f;

	Light dirLight3 = {};
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Direction = XMFLOAT3(0.0f, -1.0f, -0.2f);
	dirLight3.Color = XMFLOAT3(0.3f, 0.3f, 0.9f);
	dirLight3.Intensity = 1.0f;

	Light spotLight = {};
	spotLight.Type = LIGHT_TYPE_SPOT;
	spotLight.Position = XMFLOAT3(2.0f, 3.0f, 1.0f);
	spotLight.Direction = XMFLOAT3(0.0f, 0.2f, 1.0f);
	spotLight.SpotInnerAngle = XM_PI / 8;
	spotLight.SpotOuterAngle = XM_PIDIV4;
	spotLight.Color = XMFLOAT3(1.0f, 1.0f, 0.0f);
	spotLight.Intensity = 1.0f;
	spotLight.Range = 100.0f;

	Light pointLight = {};
	pointLight.Type = LIGHT_TYPE_POINT;
	pointLight.Position = XMFLOAT3(2.0f, 10.0f, 3.0f);
	pointLight.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	pointLight.Intensity = 1.0f;
	pointLight.Range = 100.0f;

	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	lights.push_back(spotLight);
	lights.push_back(pointLight);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize the viewport and scissor rectangle

	{

		// Set up the viewport so we render into the correct

		// portion of the render target

		viewport = {};

		viewport.TopLeftX = 0;

		viewport.TopLeftY = 0;

		viewport.Width = (float)Window::Width();

		viewport.Height = (float)Window::Height();

		viewport.MinDepth = 0.0f;

		viewport.MaxDepth = 1.0f;


		// Define a scissor rectangle that defines a portion of

		// the render target for clipping. This is different from

		// a viewport in that it is applied after the pixel shader.

		// We need at least one of these, but we're rendering to

		// the entire window, so it'll be the same size.

		scissorRect = {};

		scissorRect.left = 0;

		scissorRect.top = 0;

		scissorRect.right = Window::Width();

		scissorRect.bottom = Window::Height();

	}

	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());

	cam->UpdateProjectionMatrix(Window::AspectRatio());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	cam->Update(deltaTime);

	for (int i = 1; i < entities.size(); i++) {
		entities[i].get()->GetTransform()->Rotate(i * deltaTime,0,i * deltaTime);
		entities[i].get()->GetTransform()->MoveAbsolute(DirectX::XMFLOAT3(0, (float)sin(totalTime) * i * deltaTime, 0));
	}
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame

	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

		for (int i = 0; i < entities.size(); i++) 
		{
			RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
			RayTracing::Raytrace(cam, currentBackBuffer);
		}
	// Present

	{
		// Must occur BEFORE present

		Graphics::CloseAndExecuteCommandList();


		// Present the current back buffer and move to the next one

		bool vsync = Graphics::VsyncState();

		Graphics::SwapChain->Present(

			vsync ? 1 : 0,

			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		Graphics::AdvanceSwapChainIndex();

		Graphics::WaitForGPU();
		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



