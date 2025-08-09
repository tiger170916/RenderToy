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
    "DescriptorTable(" \
        "UAV(u1, numDescriptors = 1)" \
    ")," \
    "StaticSampler(s0, "\
        "addressU = TEXTURE_ADDRESS_CLAMP," \
        "addressV = TEXTURE_ADDRESS_CLAMP," \
        "addressW = TEXTURE_ADDRESS_CLAMP," \
        "filter = FILTER_MIN_MAG_MIP_LINEAR" \
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

RWTexture2D<float4> testMap         : register(u1);

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
    
    float2 uv = vertexOut.pos.xy / float2(width, height);

    float3 baseColor = diffuseBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float2 metallicRoughness = metallicRoughnessBuffer.SampleLevel(pointSampler, uv, 0).xy;
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;

    float3 normal = normalBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float3 worldPos = worldPosBuffer.SampleLevel(pointSampler, uv, 0).xyz;

    const float3 cameraPos = gCameraPosition.xyz;
    float3 v = normalize(cameraPos - worldPos);
    
    float3 LoTotal = float3(0.0f, 0.0f, 0.0f);
    for (uint lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
    {
        const float4x4 lightTransform = Lights.LightInstances[lightIdx].Transform;
        float4 shadowViewPos = mul(float4(worldPos, 1.0f), lightTransform);
        shadowViewPos /= shadowViewPos.w;
        
        if (pow(shadowViewPos.x, 2.0f) + pow(shadowViewPos.y, 2.0f) > 1.0f)
        {
            continue;
        }
        
        if (shadowViewPos.x < -1.0f || shadowViewPos.x > 1.0f ||
        shadowViewPos.y < -1.0f || shadowViewPos.y > 1.0f ||
        shadowViewPos.z < 0.0f || shadowViewPos.z > 1.0f)
        {
            continue;
        }
        
        // Transform to texture space.
        //float2 shadowUv = float2(0.5f, 0.5f) * (shadowViewPos.xy + float2(0.5f, 0.5f));
        float shadowU = (shadowViewPos.x + 1.0) / 2.0;
        float shadowV = (1.0 - shadowViewPos.y) / 2.0;
        
        const float3 pointLightPosition = Lights.LightInstances[lightIdx].Position.xyz;
        const float3 radiance = Lights.LightInstances[lightIdx].Intensity.xyz;
        float3 l = normalize(pointLightPosition - worldPos);
        
        // To texture coordinate
        int shadowUTex = (int)floor(shadowU * Lights.LightInstances[lightIdx].ShadowBufferSize);
        int shadowVTex = (int)floor(shadowV * Lights.LightInstances[lightIdx].ShadowBufferSize);
        
        float d = dot(normal, l);
        float bias = max(0.02 * (1.0f - d), 0.02);
        
        // calculate pcf
        float shadow = 0.0f;
        int sampleCount = 0;
        for (int m = -5; m <= 5; m++)
        {
            int u = shadowUTex + m;
            if (u < 0 || u >= Lights.LightInstances[lightIdx].ShadowBufferSize)
            {
                continue;
            }
            
            for (int n = -5; n <= 5; n++)
            {
                int v = shadowVTex + n;
                if (v < 0 || v >= Lights.LightInstances[lightIdx].ShadowBufferSize)
                {
                    continue;
                }
                
                uint2 shadowUvInt = uint2(u + Lights.LightInstances[lightIdx].ShadowBufferOffsetX, v + Lights.LightInstances[lightIdx].ShadowBufferOffsetY);
                float shadowMapDepth = lightMapAtlas[shadowUvInt];
                
                //shadow += shadowViewPos.z - bias > shadowMapDepth ? 1.0f : 0.0f;
                if (shadowViewPos.z - bias > shadowMapDepth)
                {
                    shadow += 1.0f;
                }
                
                sampleCount += 1;
            }
        }
        
        shadow = shadow / (float) sampleCount;
        
        if (shadow >= 1.0f)
        {
            continue;
        }
        
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
        float dist = distance(pointLightPosition, worldPos);
        float attenuation = 1.0 / (dist * dist);
        LoTotal += ((kD * baseColor / 3.14f + specular) * radiance * attenuation * NdotL) * (1.0f - shadow);
    }
   
        
    float3 color = LoTotal + baseColor * 0.0025f;
    float toneMap = 1.0f / 2.2f;
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(toneMap, toneMap, toneMap));
    
    return float4(color, 1.0f);
}