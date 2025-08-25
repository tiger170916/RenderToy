#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"

#define OUTLINE_THICKNESS        10
// Param1: uniform cb
// Param2: mesh cb
#define SihouetteRenderRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," 

cbuffer cbUniformFrameConstants : register(b0)
{
    UniformFrameConstants gUniformFrameConstants;
};

cbuffer cbMeshInstances : register(b1)
{
    MeshInstanceConstants MeshInstances[MAX_MESH_INSTANCE_NUM];
};

[RootSignature(SihouetteRenderRootsignature)]
MeshPsInSimple DepthStencilVertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshPsInSimple output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    
    output.pos = pos;

    return output;
}

[RootSignature(SihouetteRenderRootsignature)]
MeshGsInSihouette SihouetteVertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshGsInSihouette output;
    
    // Transform point to homogeneous space.
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);

    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    output.pos = pos;
    
    float4 normal = mul(float4(input.normal, 0.0f), MeshInstances[instanceID].TransformMatrix);
    normal = mul(normal, gUniformFrameConstants.View);
    output.normalIA = normal.xyz;
    output.Instance = instanceID;
    
    return output;
}

[RootSignature(SihouetteRenderRootsignature)]
[maxvertexcount(50)]
void SihouetteGeometryShaderMain(triangle MeshGsInSihouette input[3], inout TriangleStream<MeshPsInSimple> OutputStream)
{
    MeshPsInSimple output0;
    MeshPsInSimple output1;
    MeshPsInSimple output2;
    
    output0.pos = float4(input[0].pos.x + input[0].normalIA.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               input[0].pos.y + input[0].normalIA.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, input[0].pos.zw);
    output1.pos = float4(input[1].pos.x + input[1].normalIA.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               input[1].pos.y + input[1].normalIA.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, input[1].pos.zw);
    output2.pos = float4(input[2].pos.x + input[2].normalIA.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               input[2].pos.y + input[2].normalIA.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, input[2].pos.zw);
    

    // Spawn extruded triangle.
    OutputStream.Append(output0);
    OutputStream.Append(output1);
    OutputStream.Append(output2);

    OutputStream.RestartStrip();
}

[RootSignature(SihouetteRenderRootsignature)]
float4 SihouettePixelShaderMain(MeshPsInSimple input) : SV_Target0
{
    return float4(0.3f, 0.89f, 0.55f, 0.0f);
}