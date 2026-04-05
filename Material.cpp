#include "Material.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, DirectX::XMFLOAT4 tint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset)
	: pipelineState(pipelineState),
	colorTint(tint),
	uvScale(uvScale),
	uvOffset(uvOffset)
{
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pipelineState;
}

UINT32 Material::GetTextureIndex(UINT32 vectorIndex)
{
	return textureIndices[vectorIndex];
}

UINT32* Material::GetTextureIndices() 
{
	return &textureIndices[0];
}

void Material::SetColorTint(DirectX::XMFLOAT4 newTint)
{
	colorTint = newTint;
}

void Material::SetUVScale(DirectX::XMFLOAT2 newScale)
{
	uvScale = newScale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 newOffset)
{
	uvOffset = newOffset;
}

void Material::SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> newPState)
{
	pipelineState = newPState;
}

void Material::SetTextureIndex(UINT32 index)
{
	textureIndices[filledIndices] = index;
	filledIndices++;
}

void Material::SetTextureIndex(UINT32 index, UINT32 vectorIndex)
{
	textureIndices[vectorIndex] = index;
}
