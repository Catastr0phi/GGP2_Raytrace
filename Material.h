#pragma once

#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <d3d12.h>
#include <wrl/client.h>

const UINT32 MAX_TEXTURES = 16;

class Material
{
	// Private variables
private:
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	UINT32 textureIndices[MAX_TEXTURES];
	UINT32 filledIndices = 0;

	// Public functions
public:
	// Constructor
	Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState,
		DirectX::XMFLOAT3 tint,
		DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
		DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();

	/// <summary>
	/// Expected order: Albedo, Normal, Roughness, Metalness
	/// </summary>
	UINT32 GetTextureIndex(UINT32 vectorIndex);

	UINT32* GetTextureIndices();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 newTint);
	void SetUVScale(DirectX::XMFLOAT2 newScale);
	void SetUVOffset(DirectX::XMFLOAT2 newOffset);
	void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> newPState);

	/// <summary>
	/// Sets a new texture index at the end of the vector
	/// Expected order: Albedo, Normal, Roughness, Metalness
	/// </summary>
	void SetTextureIndex(UINT32 index);

	/// <summary>
	/// Set an existing texture index to a new index
	/// Expected order: Albedo, Normal, Roughness, Metalness
	/// </summary>
	/// <param name="index">New texture index</param>
	/// <param name="vectorIndex">Texture position within storage vector</param>
	void SetTextureIndex(UINT32 index, UINT32 vectorIndex);
};