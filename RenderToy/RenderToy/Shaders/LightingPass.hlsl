#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"
#include "LightingHeader.hlsli"

// arg0: uniform cb
// arg1: mesh cb
// arg2: lights cb
// arg3: diffuseBuffer : register(t0);
// arg4: metallicRoughnessBuffer : register(t1);
// arg5: normalBuffer : register(t2);
// arg6: worldPosBuffer : register(t3);
// arg7: emissionBuffer : register(t4);
// arg8: lightMapAtlas : register(u0);
// arg9: testMap : register(u1);
// arg10: pointSampler (static)
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
        "SRV(t4, numDescriptors = 1)" \
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

cbuffer cbUniformFrameConstants             : register(b0)
{
    UniformFrameConstants gUniformFrameConstants;
};

cbuffer cbMeshInstances                     : register(b1)
{
    MeshInstanceConstants MeshInstances[MAX_MESH_INSTANCE_NUM];
};

cbuffer cbLightTransformInstances           : register(b2)
{
    LightBuffer Lights;
};

Texture2D<float4> diffuseBuffer             : register(t0);

Texture2D<float4> metallicRoughnessBuffer   : register(t1);

Texture2D<float4> normalBuffer              : register(t2);

Texture2D<float4> worldPosBuffer            : register(t3);

Texture2D<float4> emissionBuffer            : register(t4);

RWTexture2D<float> lightMapAtlas            : register(u0);

RWTexture2D<float4> testMap                 : register(u1);

SamplerState      pointSampler              : register(s0);

[RootSignature(LightingPassRootsignature)]
MeshPsInSimple VertexShaderMain(MeshVsInScreenQuad input, uint instanceID : SV_InstanceID)
{
    MeshPsInSimple output;
    
    output.pos = float4(input.pos, 0.1f, 1.0f);

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
float4 PixelShaderMain(MeshPsInSimple input) : SV_Target0
{
    float width;
    float height;
    diffuseBuffer.GetDimensions(width, height);
    
    float2 uv = input.pos.xy / float2(width, height);

    float3 baseColor = diffuseBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float2 metallicRoughness = metallicRoughnessBuffer.SampleLevel(pointSampler, uv, 0).xy;
    float metallic = metallicRoughness.x;
    float roughness = metallicRoughness.y;
    
    //return float4(baseColor, 1.0f);

    float3 normal = normalBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float3 worldPos = worldPosBuffer.SampleLevel(pointSampler, uv, 0).xyz;
    float3 emission = emissionBuffer.SampleLevel(pointSampler, uv, 0).xyz;

    const float3 cameraPos = gUniformFrameConstants.CameraPosition.xyz;
    float3 v = normalize(cameraPos - worldPos);
    
    float3 LoTotal = float3(0.0f, 0.0f, 0.0f);
    for (uint lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
    {
        const float4x4 lightTransform = Lights.LightInstances[lightIdx].Transform;
        const float4x4 lightViewMatrix = Lights.LightInstances[lightIdx].ViewMatrix;
        
        float4 shadowNcdPos = mul(float4(worldPos, 1.0f), lightTransform);
        shadowNcdPos /= shadowNcdPos.w;
        
        //if (pow(shadowNcdPos.x, 2.0f) + pow(shadowNcdPos.y, 2.0f) > 1.0f)
        //{
         //   continue;
        //}
        
        /*if (shadowNcdPos.x < -1.0f || shadowNcdPos.x > 1.0f ||
        shadowNcdPos.y < -1.0f || shadowNcdPos.y > 1.0f ||
        shadowNcdPos.z < 0.0f || shadowNcdPos.z > 1.0f)
        {
            continue;
        }*/
        
        
        // Transform to texture space.
        float shadowU = (shadowNcdPos.x + 1.0) / 2.0;
        float shadowV = (1.0 - shadowNcdPos.y) / 2.0;
        
        const float3 pointLightPosition = Lights.LightInstances[lightIdx].Position.xyz;
        const float3 radiance = Lights.LightInstances[lightIdx].Intensity.xyz;
        float3 l = normalize(pointLightPosition - worldPos);
        
        // To texture coordinate
        int shadowUTex = (int)floor(shadowU * Lights.LightInstances[lightIdx].ShadowBufferSize);
        int shadowVTex = (int)floor(shadowV * Lights.LightInstances[lightIdx].ShadowBufferSize);
        
        float d = dot(normal, l);
        
        if (d <= 0.0f)
        {
            continue;
        }
        
        float lightNearPlane = Lights.LightInstances[lightIdx].NearPlane;
        float lightFarPlane = Lights.LightInstances[lightIdx].FarPlane;
        

        float4 shadowViewPosition = mul(float4(worldPos, 1.0f), lightViewMatrix);
        float shadowZ = shadowViewPosition.z /= shadowViewPosition.w;
        /*if (shadowZ > lightFarPlane || shadowZ < lightNearPlane)
        {
            continue;
        }*/
        
        float bias = max(lerp(0.0f, 0.02f, (shadowZ - lightNearPlane) / (lightFarPlane - lightNearPlane)) * 0.05f * (1.0f - d), 0.003f);
        
        // calculate pcf
        float shadow = 0.0f;
      
        uint2 shadowUvInt = uint2(shadowUTex + Lights.LightInstances[lightIdx].ShadowBufferOffsetX, shadowVTex + Lights.LightInstances[lightIdx].ShadowBufferOffsetY);
        //float shadowMapDepth = lightMapAtlas[shadowUvInt];
                
        for (int m = -3; m <= 3; m++)
        {
            int u = shadowUTex + m;
            if (u < 0 || u >= Lights.LightInstances[lightIdx].ShadowBufferSize)
            {
                continue;
            }
            
            for (int n = -3; n <= 3; n++)
            {
                int v = shadowVTex + n;
                if (v < 0 || v >= Lights.LightInstances[lightIdx].ShadowBufferSize)
                {
                    continue;
                }
                
                uint2 shadowUvInt = uint2(u + Lights.LightInstances[lightIdx].ShadowBufferOffsetX, v + Lights.LightInstances[lightIdx].ShadowBufferOffsetY);
                float shadowMapDepth = lightMapAtlas[shadowUvInt];
                
                if (shadowNcdPos.z - bias > shadowMapDepth)
                {
                    shadow += 1.0f;
                }
            }
        }
        
        shadow = shadow / 9.0f;
        
        /*if (shadow >= 1.0f)
        {
            continue;
        }*/
        
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
   
        
    float3 color = LoTotal /*+ baseColor * 0.0025f*/ + emission;
    float toneMap = 1.0f / 2.2f;
    color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(toneMap, toneMap, toneMap));
    
    return float4(color, 1.0f);
}