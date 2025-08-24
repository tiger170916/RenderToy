#include "MeshHeader.hlsli"
#include "LightingHeader.hlsli"

// arg0: mesh cb
// arg1: light cb
// arg2: depth atlas
#define ShadowPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b2, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u0, numDescriptors = 1)" \
    ")," \

cbuffer cbMeshInstances                         : register(b1)
{
    MeshInstanceConstants MeshInstances[MAX_MESH_INSTANCE_NUM];
}

cbuffer cbLightTransformInstances               : register(b2)
{    
    LightBuffer Lights;
};

RWTexture2D <uint> depthAtlas                   : register(u0);



[RootSignature(ShadowPassRootsignature)]
MeshGsInShadow VertexShaderMain(MeshVsInSimple input, uint instanceID : SV_InstanceID)
{
    MeshGsInShadow output;
    
    output.pos = float4(input.pos, 1.0f);
    //mul(float4(vertexIn.pos, 1.0f), MeshInstances[instanceID].WorldTransform);
    output.Instance = instanceID;

    return output;
}

[RootSignature(ShadowPassRootsignature)]
[maxvertexcount(50)]
void GeometryShaderMain(triangle MeshGsInShadow input[3], inout TriangleStream<MeshPsInShadow> OutputStream)
{
    for (uint i = 0; i < Lights.NumLights[0]; i++)
    {
        if (Lights.LightInstances[i].LightParentInstanceUid == MeshInstances[input[0].Instance].Uid[0])
        {
            return;
        }
        
        MeshPsInShadow output0;
        MeshPsInShadow output1;
        MeshPsInShadow output2;
        
        output0.pos = mul(input[0].pos, MeshInstances[input[0].Instance].TransformMatrix);
        output1.pos = mul(input[1].pos, MeshInstances[input[0].Instance].TransformMatrix);
        output2.pos = mul(input[2].pos, MeshInstances[input[0].Instance].TransformMatrix);
        output0.pos = mul(output0.pos, Lights.LightInstances[i].Transform);
        output1.pos = mul(output1.pos, Lights.LightInstances[i].Transform);
        output2.pos = mul(output2.pos, Lights.LightInstances[i].Transform);
        
        output0.LightIndex = i;
        output1.LightIndex = i;
        output2.LightIndex = i;
        
        
        // Emit the vertices to form a triangle
        OutputStream.Append(output0);
        OutputStream.Append(output1);
        OutputStream.Append(output2);

        OutputStream.RestartStrip();
    }
}

[RootSignature(ShadowPassRootsignature)]
void PixelShaderMain(MeshPsInShadow input)
{
    
    float z = input.pos.z;
    uint offsetX = Lights.LightInstances[input.LightIndex].ShadowBufferOffsetX;
    uint offsetY = Lights.LightInstances[input.LightIndex].ShadowBufferOffsetY;
    
    uint x = offsetX + (uint) floor(input.pos.x);
    uint y = offsetY + (uint) floor(input.pos.y);
    
    uint zUint = asuint(z);
    uint originalUint;
    InterlockedMin(depthAtlas[uint2(x, y)], zUint, originalUint);
}