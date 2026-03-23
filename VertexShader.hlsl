#include "ShaderStructs.hlsli"

cbuffer ExternalData : register(b0)
{
    matrix world;
    matrix view;
    matrix proj;
    matrix worldInvTranspose;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Create world-view-projection matrix from camera matrices
    matrix wvp = mul(proj, mul(view, world));
    output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
    output.uv = input.uv;
    output.normal = normalize(mul((float3x3) worldInvTranspose, input.normal));
    output.tangent = normalize(mul((float3x3) world, input.tangent));
    output.worldPos = mul(world, float4(input.localPosition, 1.0f)).xyz;
	return output;
}