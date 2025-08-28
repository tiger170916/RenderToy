#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"

#define OUTLINE_THICKNESS        6.0f
// Param1: uniform cb
// Param2: mesh cb
#define SilhouetteRenderRootsignature \
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

[RootSignature(SilhouetteRenderRootsignature)]
MeshPsInSimple DepthStencilVertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshPsInSimple output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    
    output.pos = pos;

    return output;
}

[RootSignature(SilhouetteRenderRootsignature)]
MeshGsInSilhouette SilhouetteVertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshGsInSilhouette output;
    
    // Transform point to homogeneous space.
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);

    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);
    output.pos = pos;
    
    float4 normal = mul(float4(input.normal, 1.0f), MeshInstances[instanceID].InvTransformMatrix);
    normal = mul(normal, gUniformFrameConstants.VectorView);
    if (normal.w != 0.0f)
    {
        normal = normal / normal.w;
    }
    
    output.normalIA = normalize(normal.xyz);
    output.Instance = instanceID;
    
    return output;
}

[RootSignature(SilhouetteRenderRootsignature)]
[maxvertexcount(50)]
void SilhouetteGeometryShaderMain(triangle MeshGsInSilhouette input[3], inout TriangleStream<MeshPsInSimple> OutputStream)
{
    MeshPsInSimple output0;
    MeshPsInSimple output1;
    MeshPsInSimple output2;
    
    output0.pos = input[0].pos / input[0].pos.w;
    output1.pos = input[1].pos / input[1].pos.w;
    output2.pos = input[2].pos / input[2].pos.w;
    
    float2 normal0_screen = input[0].normalIA.xy;
    float2 normal1_screen = input[1].normalIA.xy;
    float2 normal2_screen = input[2].normalIA.xy;
    if (length(normal0_screen) > 0.0f)
    {
        normal0_screen = normalize(normal0_screen);
    }
    if (length(normal1_screen) > 0.0f)
    {
        normal1_screen = normalize(normal1_screen);
    }
    if (length(normal2_screen) > 0.0f)
    {
        normal2_screen = normalize(normal2_screen);
    }
    
    output0.pos = float4(output0.pos.x + normal0_screen.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               output0.pos.y + normal0_screen.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, output0.pos.zw);
    
    output1.pos = float4(output1.pos.x + normal1_screen.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               output1.pos.y + normal1_screen.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, output1.pos.zw);
    
    output2.pos = float4(output2.pos.x + normal2_screen.x * gUniformFrameConstants.PixelWidthInNdc * OUTLINE_THICKNESS,
                               output2.pos.y + normal2_screen.y * gUniformFrameConstants.PixelHeightInNdc * OUTLINE_THICKNESS, output2.pos.zw);

    // Spawn extruded triangle.
    OutputStream.Append(output0);
    OutputStream.Append(output1);
    OutputStream.Append(output2);

    OutputStream.RestartStrip();
}

[RootSignature(SilhouetteRenderRootsignature)]
float4 SilhouettePixelShaderMain(MeshPsInSimple input) : SV_Target0
{
    return float4(0.92f, 0.58f, 0.20f, 0.0f);
}