#include "LightingHeader.hlsli"

#define LightShaftPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u2, numDescriptors = 1)" \
    "),"

struct MeshVertexIn
{
    float3 pos : POSITION0;
};

struct MeshVertexOut
{
    float4 pos : SV_Position;
};

cbuffer cbLightTransformInstances : register(b0)
{
    LightBuffer Lights;
};

RWTexture2D<float4> colorBuffer : register(u0);

RWTexture2D<float4> positionBuffer : register(u1);

RWTexture2D<float> lightFrustumDepthBuffer : register(u2);

[RootSignature(LightShaftPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn)
{
    MeshVertexOut output;
    
    output.pos = float4(vertexIn.pos, 1.0f);

    return output;
}

[RootSignature(LightShaftPassRootsignature)]
void PixelShaderMain(MeshVertexOut vertexOut)
{
    uint2 screenCoord = (uint2) floor(vertexOut.pos.xy);
    
    float pixelDepth = positionBuffer[screenCoord].z;
    float lightFrustumDepth = lightFrustumDepthBuffer[screenCoord];
    
    if (pixelDepth >= lightFrustumDepth)
    {
        return;
    }
    
    
}