#pragma once

#include <DirectXMath.h>
#include <vector>
#include "Material.h"
#include "Lights.h"

struct VertexShaderExternalData 
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 WorldInvTranspose;
};

struct PixelShaderExternalData 
{
	// Textures stored in a uint4 array and unpacked in the shader for memory alignment
	Light lights[MAX_LIGHTS];
	DirectX::XMUINT4 textures[MAX_TEXTURES/4];
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT3 camPosition;
	int lightCount;
};

struct RaytracingSceneData
{
	DirectX::XMFLOAT4X4 InverseViewProjection;
	DirectX::XMFLOAT3 CameraPosition;
	float pad;
};