#include "LightingHeader.hlsli"

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


struct MeshConstants
{
    float4x4 WorldTransform;
};

cbuffer cbMeshInstances                         : register(b1)
{
    MeshConstants MeshInstances[64];
}

cbuffer cbLightTransformInstances               : register(b2)
{    
    LightBuffer Lights;
};

RWTexture2D <uint> depthAtlas                  : register(u0);

struct SimpleMeshVsInput
{
    float3 pos : POSITION;
};

struct SimpleMeshGsInput
{
    float4 pos :SV_POSITION;
        
    uint Instance : InstanceID;
};

struct SimpleMeshPsInput
{
    float4 pos : SV_Position;
    
    uint LightIndex : LIGHT_INDEX;
};



[RootSignature(ShadowPassRootsignature)]
SimpleMeshGsInput VertexShaderMain(SimpleMeshVsInput vertexIn, uint instanceID : SV_InstanceID)
{
    SimpleMeshGsInput output;
    
    output.pos = float4(vertexIn.pos, 1.0f);
    //mul(float4(vertexIn.pos, 1.0f), MeshInstances[instanceID].WorldTransform);
    output.Instance = instanceID;

    return output;
}

[RootSignature(ShadowPassRootsignature)]
[maxvertexcount(50)]
void GeometryShaderMain(triangle SimpleMeshGsInput input[3], inout TriangleStream<SimpleMeshPsInput> OutputStream)
{
   // InstanceID();
    for (uint i = 0; i < Lights.NumLights[0]; i++)
    {
        SimpleMeshPsInput output0;
        SimpleMeshPsInput output1;
        SimpleMeshPsInput output2;
        output0.pos = mul(input[0].pos, MeshInstances[input[0].Instance].WorldTransform);
        output1.pos = mul(input[1].pos, MeshInstances[input[1].Instance].WorldTransform);
        output2.pos = mul(input[2].pos, MeshInstances[input[2].Instance].WorldTransform);
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
void PixelShaderMain(SimpleMeshPsInput vertexOut)
{
    
    float z = vertexOut.pos.z; //vertexOut.pos.z;
    uint offsetX = Lights.LightInstances[vertexOut.LightIndex].ShadowBufferOffsetX;
    uint offsetY = Lights.LightInstances[vertexOut.LightIndex].ShadowBufferOffsetY;
    
    uint x = offsetX + (uint) floor(vertexOut.pos.x);
    uint y = offsetY + (uint) floor(vertexOut.pos.y);
    
    uint zUint = asuint(z);
    uint originalUint;
    uint a;
    InterlockedMin(depthAtlas[uint2(x, y)], zUint, originalUint);
}