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
    uint4 NumLights;
    
    LightConstants LightInstances[50];
};

RWTexture2D <float> depthAtlas                  : register(u0);

struct SimpleMeshVsInput
{
    float3 pos : POSITION;
};

struct SimpleMeshGsInput
{
    float4 pos :SV_POSITION;
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
    
    output.pos = mul(float4(vertexIn.pos, 1.0f), MeshInstances[instanceID].WorldTransform);
    
    return output;
}

[RootSignature(ShadowPassRootsignature)]
[maxvertexcount(50)]
void GeometryShaderMain(triangle SimpleMeshGsInput input[3], inout TriangleStream<SimpleMeshPsInput> OutputStream)
{
    for (uint i = 0; i < NumLights[0]; i++)
    {
        SimpleMeshPsInput output0;
        SimpleMeshPsInput output1;
        SimpleMeshPsInput output2;
        output0.pos = mul(input[0].pos, LightInstances[i].Transform);
        output1.pos = mul(input[1].pos, LightInstances[i].Transform);
        output2.pos = mul(input[2].pos, LightInstances[i].Transform);
        //output0.pos = input[0].pos;
        //output1.pos = input[1].pos;
        //output2.pos = input[2].pos;
        
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
    
    float z = vertexOut.pos.z / vertexOut.pos.w;
    uint offsetX = LightInstances[vertexOut.LightIndex].ShadowBufferOffsetX;
    uint offsetY = LightInstances[vertexOut.LightIndex].ShadowBufferOffsetY;
    
    uint x = offsetX + (uint) floor(vertexOut.pos.x);
    uint y = offsetY + (uint) floor(vertexOut.pos.y);

    if (z < depthAtlas[uint2(x, y)])
    {
        depthAtlas[uint2(x, y)] = z;
    }
}