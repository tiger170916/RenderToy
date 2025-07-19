#include "MeshVertex.hlsli"

#define LightingPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "SRV(t0, numDescriptors = 1)" \
    ")," \
    "StaticSampler(s0, "\
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT" \
    ")"

struct PS_OUTPUT
{
    float4 Diffuse : SV_Target0;
};

Texture2D<float4> diffuseBuffer : register(t0);

SamplerState      pointSampler  : register(s0);

[RootSignature(LightingPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn, uint instanceID : SV_InstanceID)
{
    MeshVertexOut output;
    
    output.pos = float4(vertexIn.pos, 1.0f);

    return output;
}

[RootSignature(LightingPassRootsignature)]
float4 PixelShaderMain(MeshVertexOut vertexOut) : SV_Target0
{
    float width;
    float height;
    diffuseBuffer.GetDimensions(width, height);
    
    float2 uv = vertexOut.pos.xy / float2(width, height) / vertexOut.pos.w;

    return diffuseBuffer.SampleLevel(pointSampler, uv, 0);
}