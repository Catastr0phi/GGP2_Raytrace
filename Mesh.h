#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"
#include <string>


class Mesh 
{
// Private data
private:
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vbView{};

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;

	D3D12_INDEX_BUFFER_VIEW ibView{};

	int vertexCount;
	int indexCount;

	std::string name;

	MeshRayTracingData rayTracingData;

	void CreateBuffers(Vertex vertices[], unsigned int indices[], int newVertexCount, int newIndexCount);

// Public methods
public:
	Mesh(Vertex vertices[], unsigned int indices[], int vertexCount, int indexCount, std::string newName);
	Mesh(const char* objFile, std::string newName);
	~Mesh();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	int GetIndexCount();
	int GetVertexCount();
	std::string GetName();
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	const MeshRayTracingData& getRayTracingData();
};

struct MeshRayTracingData
{
	D3D12_GPU_DESCRIPTOR_HANDLE IndexBufferSRV {};
	D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferSRV{};
	Microsoft::WRL::ComPtr<ID3D12Resource> BLAS;
};