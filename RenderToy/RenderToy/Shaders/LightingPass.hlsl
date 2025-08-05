#include "MeshVertex.hlsli"
#include "LightingHeader.hlsli"

#define LightingPassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "CBV(b2, numDescriptors = 1)" \
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
        "UAV(u0, numDescriptors = 1)" \
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

cbuffer cbLightTransformInstances           : register(b2)
{
    LightBuffer Lights;
};

Texture2D<float4> diffuseBuffer             : register(t0);

Texture2D<float4> metallicRoughnessBuffer   : register(t1);

Texture2D<float4> normalBuffer              : register(t2);

Texture2D<float4> worldPosBuffer            : register(t3);

RWTexture2D<float> lightMapAtlas            : register(u0);

SamplerState      pointSampler              : register(s0);

[RootSignature(LightingPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn, uint instanceID : SV_InstanceID)
{
    MeshVertexOut output;
    
    output.pos = float4(vertexIn.pos, 1.0f);

    return output;
}

float NormalDistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float denominator = (NdotH2 * (a2 - 1.0) + 1.0);
    denominator = 3.1415 * denominator * denominator;
	
    return a2 / denominator;
}

float GeometrySchlickGGXSub(float3 N, float3 V, float roughness)
{
    float k = ((roughness + 1) * (roughness + 1)) / 8.0f;
    float NDotV = max(dot(N, V), 0.0f);
    
    return NDotV / (NDotV * (1 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float sub1 = GeometrySchlickGGXSub(N, V, roughness);
    float sub2 = GeometrySchlickGGXSub(N, L, roughness);
    return sub1 * sub2;
}

float3 fresnelSchlick(float3 N, float3 H, float3 baseColor, float metallic)
{
    float3 F0 = lerp(float3(0.04f, 0.04f, 0.04f), baseColor, metallic);
    float NDotH = max(dot(N, H), 0.0f);
    return F0 + (1.0 - F0) * pow(clamp(1.0 - NDotH, 0.0f, 1.0f), 5.0);
}

[RootSignature(LightingPassRootsignature)]
float4 PixelShaderMain(MeshVertexOut vertexOut) : SV_Target0
{
    float width;
    float height;
    diffuseBuffer.GetDimensions(width, height);
    
    float2 uv = vertexOut.pos.xy / float2(width, height) / vertexOut.pos.w;

    float3 baseColor = diffuseBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float2 metallicRoughness = metallicRoughnessBuffer.SampleLevel(pointSampler, uv, 0).xy;
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;
    //roughness = 0.6f;
    //metallic = 0.1;
    float3 normal = normalBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float3 worldPos = worldPosBuffer.SampleLevel(pointSampler, uv, 0).xyz;

    
    const float3 pointLightPosition = float3(20.0f, 3.5f, -6.0f);
    const float3 radiance = float3(110.0f, 110.0f, 100.0f);
    const float3 cameraPos = float3(0.0f, 3.0f, -12.5f);
    
    float3 v = normalize(cameraPos - worldPos);
    float3 l = normalize(pointLightPosition - worldPos);
    float3 h = normalize(v + l);
    
    
    // Cook-torrance brdf
    float NDF = NormalDistributionGGX(normal, h, roughness);
    float Geometry = GeometrySmith(normal, v, l, roughness);
    float3 Fresnel = fresnelSchlick(v, h, baseColor, metallic);
    
        
    float3 kS = Fresnel;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= (1.0 - metallic);
    
    
    float3 numerator = NDF * Geometry * Fresnel;
    
    float denominator = 4.0 * max(dot(normal, v), 0.0) * max(dot(normal, l), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
            
    float NdotL = max(dot(normal, l), 0.0);
    float dist = distance(pointLightPosition, worldPos) / 2.7;
    float attenuation = 1.0 / (dist * dist);
    float3 Lo = (kD * baseColor / 3.14f + specular) * radiance * attenuation * NdotL;
    
    float3 color = Lo;//+baseColor * 0.01;
    float toneMap = 1.0f / 2.2f;
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(toneMap, toneMap, toneMap));
    
    return float4(color, 1.0f);
}