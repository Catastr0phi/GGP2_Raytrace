#include "ShaderStructs.hlsli"
#include "PBRFuncs.hlsli"

cbuffer ExternalData : register(b0)
{
    Light lights[MAX_LIGHTS];
    uint4 packedTextures[4];
    float2 uvScale;
    float2 uvOffset;
    float3 camPosition;
    int lightCount;
}

SamplerState Sampler : register(s0);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    // Unpack the textures
    uint textures[16] = (uint[16])packedTextures;
    
    Texture2D albedoTexture = ResourceDescriptorHeap[textures[0]];
    Texture2D normalTexture = ResourceDescriptorHeap[textures[1]];
    Texture2D roughnessTexture = ResourceDescriptorHeap[textures[2]];
    Texture2D metalTexture = ResourceDescriptorHeap[textures[3]];
    
    input.uv = input.uv * uvScale + uvOffset;
    
    float4 surfaceColor = pow(albedoTexture.Sample(Sampler, input.uv), 2.2);
    
    // Unpack normal map
    float3 unpackedNormal = normalTexture.Sample(Sampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    // Normalize input vectors
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    
    // Calculate TBN
    input.tangent = normalize(input.tangent - input.normal * dot(input.tangent, input.normal));
    float3 B = cross(input.tangent, input.normal);
    float3x3 TBN = float3x3(input.tangent, B, input.normal);
    
    // Transform normal from map
    input.normal = mul(unpackedNormal, TBN);
    
    float3 totalLight = float3(0, 0, 0);
    
    // Roughness and metallic
    float roughness = roughnessTexture.Sample(Sampler, input.uv).r;
    float metalness = metalTexture.Sample(Sampler, input.uv).r;
    
    // Specular color determination 
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
    
        // Light calculation
    float3 V = normalize(camPosition - input.worldPos);
    
    for (int i = 0; i < lightCount; i++)
    {
        float3 lightResult = 0;
        
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                lightResult = directionalLight(lights[i], input.normal, V, surfaceColor.xyz, lights[i].Direction, roughness, specularColor, metalness);
                break;
            case LIGHT_TYPE_POINT:
                lightResult = pointLight(lights[i], input.normal, V, surfaceColor.xyz, input.worldPos, roughness, specularColor, metalness);
                break;
            case LIGHT_TYPE_SPOT:
                lightResult = spotLight(lights[i], input.normal, V, surfaceColor.xyz, input.worldPos, roughness, specularColor, metalness);
                break;
        }
        
        totalLight += lightResult;
    }

    	
    return float4(pow(totalLight, 1.0f / 2.2f), 1);
}