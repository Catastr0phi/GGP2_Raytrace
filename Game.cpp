#pragma once

#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"

#include <DirectXMath.h>

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
	CreateRootSigAndPipelineState();

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

void Game::CreateRootSigAndPipelineState()

{

	// Blobs to hold raw shader byte code used in several steps below

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;

	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;


	// Load shaders

	{

		// Read our compiled vertex shader code into a blob

		// - Essentially just "open the file and plop its contents here"

		D3DReadFileToBlob(
			FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());

		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());

	}


	// Input layout

	const unsigned int inputElementCount = 4;

	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};

	{

		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3

		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader

		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic


		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2

		inputElements[1].SemanticName = "TEXCOORD";

		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic


		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3

		inputElements[2].SemanticName = "NORMAL";

		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic


		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3

		inputElements[3].SemanticName = "TANGENT";

		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic

	}


	// Root Signature

	{

		// Define a table of CBV's (constant buffer views)

		D3D12_DESCRIPTOR_RANGE cbvTable = {};

		cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;

		cbvTable.NumDescriptors = 1;

		cbvTable.BaseShaderRegister = 0;

		cbvTable.RegisterSpace = 0;

		cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		// Define the root parameters

		// For vertex shader

		D3D12_ROOT_PARAMETER vertexRP = {};

		vertexRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

		vertexRP.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		vertexRP.DescriptorTable.NumDescriptorRanges = 1;

		vertexRP.DescriptorTable.pDescriptorRanges = &cbvTable;

		// For pixel shader 

		D3D12_ROOT_PARAMETER pixelRP = {};

		pixelRP.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;

		pixelRP.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		pixelRP.DescriptorTable.NumDescriptorRanges = 1;

		pixelRP.DescriptorTable.pDescriptorRanges = &cbvTable;

		// Array of all params
		D3D12_ROOT_PARAMETER rootParams[2] = {vertexRP, pixelRP};

		// Static sampler
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // Register s0
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe the overall the root signature

		D3D12_ROOT_SIGNATURE_DESC rootSig = {};

		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
						D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

		rootSig.NumParameters = ARRAYSIZE(rootParams);

		rootSig.pParameters = rootParams;

		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);

		rootSig.pStaticSamplers = samplers;


		ID3DBlob* serializedRootSig = 0;

		ID3DBlob* errors = 0;


		D3D12SerializeRootSignature(

			&rootSig,

			D3D_ROOT_SIGNATURE_VERSION_1,

			&serializedRootSig,

			&errors);


		// Check for errors during serialization

		if (errors != 0)

		{

			OutputDebugString((wchar_t*)errors->GetBufferPointer());

		}


		// Actually create the root sig

		Graphics::Device->CreateRootSignature(

			0,

			serializedRootSig->GetBufferPointer(),

			serializedRootSig->GetBufferSize(),

			IID_PPV_ARGS(rootSignature.GetAddressOf()));


	}


	// Pipeline state

	{

		// Describe the pipeline state

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};


		// -- Input assembler related ---

		psoDesc.InputLayout.NumElements = inputElementCount;

		psoDesc.InputLayout.pInputElementDescs = inputElements;

		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

		// Overall primitive topology type (triangle, line, etc.) is set here

		// IASetPrimTop() is still used to set list/strip/adj options


		// Root sig

		psoDesc.pRootSignature = rootSignature.Get();


		// -- Shaders (VS/PS) ---

		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();

		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();

		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();

		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();


		// -- Render targets ---

		psoDesc.NumRenderTargets = 1;

		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		psoDesc.SampleDesc.Count = 1;

		psoDesc.SampleDesc.Quality = 0;


		// -- States ---

		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;

		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

		psoDesc.RasterizerState.DepthClipEnable = true;


		psoDesc.DepthStencilState.DepthEnable = true;

		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;


		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;

		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;

		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;


		// -- Misc ---

		psoDesc.SampleMask = 0xffffffff;


		// Create the pipe state object

		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));

	}


	// Set up the viewport and scissor rectangle

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

}


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

	// Create textures
	UINT32 cobbleAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str());
	UINT32 cobbleNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str());
	UINT32 cobbleRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str());
	UINT32 cobbleMetallic = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str());

	UINT32 floorAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_albedo.png").c_str());
	UINT32 floorNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_normals.png").c_str());
	UINT32 floorRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_roughness.png").c_str());
	UINT32 floorMetallic = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/floor_metal.png").c_str());

	UINT32 woodAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/wood_albedo.png").c_str());
	UINT32 woodNormal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/wood_normals.png").c_str());
	UINT32 woodRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/wood_roughness.png").c_str());
	UINT32 woodMetallic = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/wood_metal.png").c_str());

	// Create materials
	std::shared_ptr<Material> cobbleMat = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	cobbleMat->SetTextureIndex(cobbleAlbedo);
	cobbleMat->SetTextureIndex(cobbleNormal);
	cobbleMat->SetTextureIndex(cobbleRoughness);
	cobbleMat->SetTextureIndex(cobbleMetallic);

	std::shared_ptr<Material> floorMat = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	floorMat->SetTextureIndex(floorAlbedo);
	floorMat->SetTextureIndex(floorNormal);
	floorMat->SetTextureIndex(floorRoughness);
	floorMat->SetTextureIndex(floorMetallic);

	std::shared_ptr<Material> woodMat = std::make_shared<Material>(pipelineState, DirectX::XMFLOAT3(1, 1, 1));
	woodMat->SetTextureIndex(woodAlbedo);
	woodMat->SetTextureIndex(woodNormal);
	woodMat->SetTextureIndex(woodRoughness);
	woodMat->SetTextureIndex(woodMetallic);

	// Create entities
	entities.push_back(GameEntity(cube, cobbleMat));
	entities.push_back(GameEntity(sphere, floorMat));
	entities.push_back(GameEntity(helix, woodMat));

	entities[0].GetTransform()->MoveAbsolute(5, 0, 5);
	entities[1].GetTransform()->MoveAbsolute(0, 0, 5);
	entities[2].GetTransform()->MoveAbsolute(-5, 0, 5);

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

	entities[0].GetTransform()->Rotate(DirectX::XMFLOAT3(0, 1 * deltaTime, 0));
	entities[2].GetTransform()->Rotate(DirectX::XMFLOAT3(0, -1 * deltaTime, 0));
	entities[1].GetTransform()->MoveRelative(DirectX::XMFLOAT3(0, (float)sin(totalTime) * deltaTime, 0));
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame

	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];


	// Clearing the render target

	{

		// Transition the back buffer from present to render target

		D3D12_RESOURCE_BARRIER rb = {};

		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		rb.Transition.pResource = currentBackBuffer.Get();

		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;

		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		Graphics::CommandList->ResourceBarrier(1, &rb);


		// Background color (black) for clearing

		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };


		// Clear the RTV

		Graphics::CommandList->ClearRenderTargetView(

			Graphics::RTVHandles[Graphics::SwapChainIndex()],

			color,

			0, 0); // No scissor rectangles


		// Clear the depth buffer, too

		Graphics::CommandList->ClearDepthStencilView(

			Graphics::DSVHandle,

			D3D12_CLEAR_FLAG_DEPTH,

			1.0f, // Max depth = 1.0f

			0, // Not clearing stencil, but need a value

			0, 0); // No scissor rects

	}

	Graphics::CommandList->SetDescriptorHeaps(1, Graphics::CBVSRVDescriptorHeap.GetAddressOf());

	// Rendering here!

	{

		// Set overall pipeline state

		Graphics::CommandList->SetPipelineState(pipelineState.Get());


		// Root sig (must happen before root descriptor table)

		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());


		// Set up other commands for rendering

		Graphics::CommandList->OMSetRenderTargets(
			1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true, &Graphics::DSVHandle);

		Graphics::CommandList->RSSetViewports(1, &viewport);

		Graphics::CommandList->RSSetScissorRects(1, &scissorRect);

		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (int i = 0; i < entities.size(); i++) 
		{
			std::shared_ptr<Material> entMat = entities[i].GetMaterial();

			// Swap to material PSO
			Graphics::CommandList->SetPipelineState(entMat->GetPipelineState().Get());

			// Fill vertex shader data
			VertexShaderExternalData vsData = {};
			vsData.World = entities[i].GetTransform()->GetWorldMatrix();
			vsData.Projection = cam->GetProjection();
			vsData.View = cam->GetView();
			vsData.WorldInvTranspose = entities[i].GetTransform()->GetWorldInverseTransposeMatrix();

			// Fill buffer and get handle
			D3D12_GPU_DESCRIPTOR_HANDLE cbvHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
				&vsData,
				sizeof(VertexShaderExternalData));

			// Set handle
			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, cbvHandle);

			// Fill pixel shader data
			PixelShaderExternalData psData = {};

			// Fill packed textures array with float4s containing the indices
			// Not doing a float4 array causes alignment issues, since, APPARENTLY, other arrays don't align in cbuffers
			memcpy(psData.lights, &lights[0], sizeof(Light) * MAX_LIGHTS);
			for (int j = 0; j < MAX_TEXTURES / 4; j++) {
				psData.textures[j] = DirectX::XMUINT4(
					entMat->GetTextureIndex(j * 4),
					entMat->GetTextureIndex(j * 4 + 1),
					entMat->GetTextureIndex(j * 4 + 2),
					entMat->GetTextureIndex(j * 4 + 3));
			}
			psData.uvScale = entMat->GetUVScale();
			psData.uvOffset = entMat->GetUVOffset();
			psData.camPosition = cam->GetTransform()->GetPosition();
			psData.lightCount = (int)lights.size();

			// Fill buffer and get handle
			D3D12_GPU_DESCRIPTOR_HANDLE psCBVHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
				&psData,
				sizeof(PixelShaderExternalData));

			// Set handle
			Graphics::CommandList->SetGraphicsRootDescriptorTable(1, psCBVHandle);

			// Get and set buffer views
			D3D12_VERTEX_BUFFER_VIEW vbView = entities[i].GetMesh()->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibView = entities[i].GetMesh()->GetIndexBufferView();

			Graphics::CommandList->IASetVertexBuffers(0,1,&vbView);
			Graphics::CommandList->IASetIndexBuffer(&ibView);

			Graphics::CommandList->DrawIndexedInstanced(entities[i].GetMesh()->GetIndexCount(), 1, 0, 0, 0);
		}
	}
	// Present

	{

		// Transition back to present

		D3D12_RESOURCE_BARRIER rb = {};

		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

		rb.Transition.pResource = currentBackBuffer.Get();

		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;

		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		Graphics::CommandList->ResourceBarrier(1, &rb);


		// Must occur BEFORE present

		Graphics::CloseAndExecuteCommandList();


		// Present the current back buffer and move to the next one

		bool vsync = Graphics::VsyncState();

		Graphics::SwapChain->Present(

			vsync ? 1 : 0,

			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		Graphics::AdvanceSwapChainIndex();

		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
	}
}



