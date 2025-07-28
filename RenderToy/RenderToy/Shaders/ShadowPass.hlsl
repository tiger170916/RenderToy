#define ShadowPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u0, numDescriptors = 1)" \
    ")," \


struct LightConstants
{
    float4x4 LightTransform;
    
    uint BufferOffsetX;
    
    uint BufferOffsetY;
};

struct MeshConstants
{
    float4x4 WorldTransform;
};

cbuffer cbLightTransformInstances               : register(b0)
{
    uint NumLightTransforms;
    
    LightConstants LightInstances[50];
};

cbuffer cbMeshInstances                         : register(b1)
{
    MeshConstants MeshInstances[64];
}

RWTexture2D <float> depthAtlas                 : register(u0);

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
    for (uint i = 0; i < NumLightTransforms; i++)
    {
        SimpleMeshPsInput output0;
        SimpleMeshPsInput output1;
        SimpleMeshPsInput output2;
        output0.pos = mul(input[0].pos, LightInstances[i].LightTransform);
        output1.pos = mul(input[1].pos, LightInstances[i].LightTransform);
        output2.pos = mul(input[2].pos, LightInstances[i].LightTransform);
        
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
    
    vertexOut.pos /= vertexOut.pos.w;
    uint offsetX = LightInstances[vertexOut.LightIndex].BufferOffsetX;
    uint offsetY = LightInstances[vertexOut.LightIndex].BufferOffsetY;
    
    uint x = offsetX + (uint) floor(vertexOut.pos.x);
    uint y = offsetY + (uint) floor(vertexOut.pos.y);

    if (vertexOut.pos.z < depthAtlas[float2(x, y)])
    {
        depthAtlas[float2(x, y)] = vertexOut.pos.z;
    }
}