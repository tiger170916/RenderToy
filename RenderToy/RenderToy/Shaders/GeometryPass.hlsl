#include "MeshVertex.hlsli"

#define GeometryPassRootsignature \
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

Texture2D<float4> depthBuffer  : register(t0);

SamplerState      pointSampler : register(s0);

[RootSignature(GeometryPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn, uint instanceID : SV_InstanceID)
{
    MeshVertexOut output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(vertexIn.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    pos = mul(pos, gView);
    pos = mul(pos, gProjection);
    
    output.pos = pos;

    return output;
}

[RootSignature(GeometryPassRootsignature)]
PS_OUTPUT PixelShaderMain(MeshVertexOut vertexOut)
{
    PS_OUTPUT output;
    
    float4 pos = vertexOut.pos / vertexOut.pos.w;
    // Early out if the shading point is behind
    if (depthBuffer.SampleLevel(pointSampler, pos.xy, 0).x < pos.z)
    {
        return output;
    }
    
    output.Diffuse = float4(0.5f, 0.5f, 0.5f, 1.0f);
    return output;
}