#include "MeshVertex.hlsli"

#define EarlyZPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \

[RootSignature(EarlyZPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn, uint instanceID : SV_InstanceID)
{
    MeshVertexOut output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(vertexIn.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    pos = mul(pos, gViewProjectionMatrix);
    //pos = mul(pos, gProjection);
    
    output.pos = pos;

    return output;
}