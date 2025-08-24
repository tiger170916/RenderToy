#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"

// arg0: uniform cb
// arg1: mesh cb
// arg2: depthBuffer
// arg3: baseColorTex
// arg4: metallicTex
// arg5: roughnessTex
// arg6: normalTex
// arg7: point sampler (static)
// arg8: linear sampler (static)
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
    "DescriptorTable(" \
        "SRV(t1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "SRV(t2, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "SRV(t3, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "SRV(t4, numDescriptors = 1)" \
    ")," \
    "StaticSampler(s0, "\
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_POINT" \
    "),"\
    "StaticSampler(s1, "\
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR" \
    ")"

struct PS_OUTPUT
{
    float4 BaseColor            : SV_Target0;
    
    float2 MetallicRoughness    : SV_Target1;
    
    float4 Normal               : SV_Target2;
    
    float4 WorldPos             : SV_Target3;
    
    float4 Emission             : SV_Target4;
};

cbuffer cbUniformFrameConstants     : register(b0)
{
    UniformFrameConstants gUniformFrameConstants;
};

cbuffer cbMeshInstances             : register(b1)
{
    MeshInstanceConstants MeshInstances[MAX_MESH_INSTANCE_NUM];
};

Texture2D<float4> depthBuffer       : register(t0);
Texture2D<float4> baseColorTex      : register(t1);
Texture2D<float4> metallicTex       : register(t2);
Texture2D<float4> roughnessTex      : register(t3);
Texture2D<float4> normalTex         : register(t4);


SamplerState      pointSampler      : register(s0);

SamplerState      linearSampler     : register(s1);

[RootSignature(GeometryPassRootsignature)]
MeshPsIn VertexShaderMain(MeshVsIn input, uint instanceID : SV_InstanceID)
{
    MeshPsIn output;
    
    // Transform point to homogeneous space.
    float4 pos = mul(float4(input.pos, 1.0f), MeshInstances[instanceID].TransformMatrix);
    output.worldPos = pos;
    pos = mul(pos, gUniformFrameConstants.ViewProjectionMatrix);

    output.uv = input.uv;
    output.pos = pos;
    output.instanceId = instanceID;
    output.normalIA = input.normal;   // no need, just leave it as is
    
    return output;
}

[RootSignature(GeometryPassRootsignature)]
PS_OUTPUT PixelShaderMain(MeshPsIn input)
{
    PS_OUTPUT output;
    
    float width;
    float height;
    depthBuffer.GetDimensions(width, height);
    
    float2 pixelPos = input.pos.xy;
    float2 screenUv = (float2)pixelPos / float2(width, height);
    
    float z = input.pos.z;
    
    float2 uv = float2(input.uv.x, 1.0f - input.uv.y);
    
    float3 pos_dx = ddx(input.worldPos.xyz);
    float3 pos_dy = ddy(input.worldPos.xyz);
    float2 texC_dx = ddx(input.uv);
    float2 texC_dy = ddy(input.uv);
    
    // Early out if the shading point is behind
    if (depthBuffer.SampleLevel(pointSampler, screenUv, 0).x < z)
    {
        return output;  
    }
    
    float3 T = normalize(pos_dx * texC_dy.y - pos_dy * texC_dx.y);
    float3 B = normalize(pos_dy * texC_dx.x - pos_dx * texC_dy.x);
    float3 N = normalize(cross(T, B));

    float3x3 TBN = float3x3(T, B, N);
    
    
    float metallic = metallicTex.SampleLevel(linearSampler, uv, 0).x;
    float roughness = roughnessTex.SampleLevel(linearSampler, uv, 0).x;
    
    float3 normal3 = normalTex.SampleLevel(linearSampler, uv, 0).xyz;
    normal3 = (normal3 * 2.0f) - 1.0f;
    normal3 = normalize(mul(normal3, TBN));
    
    output.BaseColor = baseColorTex.SampleLevel(linearSampler, float2(input.uv.x, 1.0f - input.uv.y), 0);
    output.MetallicRoughness = float2(metallic, roughness);
    output.Normal = float4(normal3, 0.0f);
    output.WorldPos = input.worldPos;
    output.Emission = MeshInstances[input.instanceId].LightEmission;
    
    return output;
}