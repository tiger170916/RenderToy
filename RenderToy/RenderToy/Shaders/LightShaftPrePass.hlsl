#include "GlobalHeader.hlsli"

// arg0: uniform cb
// arg1: light frustum cb
#define LightShaftPrePassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    "),"\
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    "),"

struct LightShaftPrePassVertexIn
{
    float3 pos : POSITION0;
};

struct LightShaftPrePassVertexOut
{
    float4 pos : SV_Position;
    
    float4 worldPos : POSITION0;
};

cbuffer cbUniformFrameConstants : register(b0)
{    
    UniformFrameConstants gUniformFrameConstants;
};

cbuffer cbFrustumMeshConstants  : register(b1)
{
    float4x4 TransformMatrix;
};



[RootSignature(LightShaftPrePassRootsignature)]
LightShaftPrePassVertexOut VertexShaderMain(LightShaftPrePassVertexIn vertexIn)
{
    LightShaftPrePassVertexOut output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(vertexIn.pos, 1.0f), TransformMatrix);
    output.worldPos = pos;
    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    output.pos = pos;
    
    return output;
}

[RootSignature(LightShaftPrePassRootsignature)]
float4 PixelShaderMain(LightShaftPrePassVertexOut vertexOut) : SV_Target0
{
    return vertexOut.worldPos / vertexOut.worldPos.w;
}

[RootSignature(LightShaftPrePassRootsignature)]
float4 DebugPixelShaderMain(LightShaftPrePassVertexOut vertexOut) : SV_Target0
{
    return float4(0.5f, 0.5f, 0.5f, 1.0f);
}