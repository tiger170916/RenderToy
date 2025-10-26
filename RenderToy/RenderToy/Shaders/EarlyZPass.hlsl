#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"

// Param1: uniform cb
// Param2: mesh cb
#define EarlyZPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \

cbuffer cbUniformFrameConstants : register(b0)
{
    UniformFrameConstants gUniformFrameConstants;
};


cbuffer cbMeshInstances : register(b1)
{
    MeshInstanceConstants MeshInstances[MAX_MESH_INSTANCE_NUM];
};

[RootSignature(EarlyZPassRootsignature)]
MeshPsIn VertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshPsIn output;
    
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    
    output.pos = pos;

    return output;
}