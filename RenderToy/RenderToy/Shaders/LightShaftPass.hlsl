#include "LightingHeader.hlsli"

#define LightShaftPassRootsignature \
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
    "DescriptorTable(" \
        "UAV(u1, numDescriptors = 1)" \
    ")," \
    "DescriptorTable(" \
        "UAV(u2, numDescriptors = 1)" \
    "),"\
    "DescriptorTable(" \
        "UAV(u3, numDescriptors = 1)" \
    "),"

static float sigma_absorbtion = 0.002f;
static float sigma_scattering = 0.002f;

struct MeshVertexIn
{
    float2 pos : POSITION0;
};

struct MeshVertexOut
{
    float4 pos : SV_Position;
};

cbuffer cbUniformFrameConstants : register(b0)
{
    float4x4 gView;
    float4x4 gProjection;
    float4x4 gInvProjectionMatrix;
    float4x4 gViewProjectionMatrix;
    float4x4 gInvViewProjectionMatrix;
    
    float4 gCameraPosition;
    
    float gPixelStepScale;
    float3 Pad0;
};

cbuffer cbLightTransformInstances : register(b1)
{
    LightBuffer Lights;
};

RWTexture2D<float4> colorBuffer : register(u0);

RWTexture2D<float4> positionBuffer : register(u1);

RWTexture2D<float4> lightFrustumPositionBuffer : register(u2);

RWTexture2D<float> lightAtlas : register(u3);

[RootSignature(LightShaftPassRootsignature)]
MeshVertexOut VertexShaderMain(MeshVertexIn vertexIn)
{
    MeshVertexOut output;
    
    output.pos = float4(vertexIn.pos, 0.1f, 1.0f);

    return output;
}

[RootSignature(LightShaftPassRootsignature)]
void PixelShaderMain(MeshVertexOut vertexOut)
{
    uint texWidth;
    uint texHeight;
    positionBuffer.GetDimensions(texWidth, texHeight);
    uint2 screenCoord = (uint2) floor(vertexOut.pos.xy);
    
    
    float testVal = positionBuffer[screenCoord].w;
    float4 backPosWorld = float4(positionBuffer[screenCoord].xyz, 1.0f);
    float4 backPosView = mul(backPosWorld, gView);
    float stepSize = 0.05f;
    
    if (backPosWorld.w <= 0.001f)
    {
        return;
    }
    
    //float2 ndc = ((float2)screenCoord / float2(texWidth, texHeight)) * 2.0f - 1.0f;
    float2 ndc = float2((float) screenCoord.x / (float) texWidth * 2.0f - 1.0f, 1.0f - (float) screenCoord.y / (float)texHeight * 2.0f);
    
    
    float4 farPoint = mul(float4(ndc.x, ndc.y, 1.0f, 1.0f), gInvViewProjectionMatrix);
    float4 nearPoint = mul(float4(ndc.x, ndc.y, 0.01f, 1.0f), gInvViewProjectionMatrix);
    farPoint /= farPoint.w;
    nearPoint /= nearPoint.w;
    float3 rayMarchDir = normalize(farPoint.xyz - gCameraPosition.xyz);
    
    
    float3 L = float3(0.0f, 0.0f, 0.0f);
    float transmission = 1.0f;
    
    int numSamples = 2400;
    float3 rayMarchPos = nearPoint.xyz;
    for (int i = 0; i < numSamples; i++)
    {
        if (testVal >= 0.01f)
        {
            float4 samplePosView = mul(float4(rayMarchPos, 1.0f), gView);
            if (samplePosView.z >= backPosView.z)
            {
                break;
            }
        }
        
        transmission *= exp(-stepSize * (sigma_absorbtion + sigma_scattering));
        if (transmission <= 0.01)
        {
            break; // early out if no light can reach the starting point form here on
        }
            
        for (int lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
        {
            const float4x4 lightTransform = Lights.LightInstances[lightIdx].Transform;
        
            float4 shadowNcdPos = mul(float4(rayMarchPos, 1.0f), lightTransform);
            shadowNcdPos /= shadowNcdPos.w;
            
        
            if (pow(shadowNcdPos.x, 2.0f) + pow(shadowNcdPos.y, 2.0f) > 1.0f)
            {
                continue;
            }
        
            if (shadowNcdPos.x < -1.0f || shadowNcdPos.x > 1.0f ||
                shadowNcdPos.y < -1.0f || shadowNcdPos.y > 1.0f ||
                shadowNcdPos.z < 0.0f || shadowNcdPos.z > 1.0f)
            {
                continue;
            }
        
                // Transform to texture space.
            float shadowU = (shadowNcdPos.x + 1.0) / 2.0;
            float shadowV = (1.0 - shadowNcdPos.y) / 2.0;
                
            int shadowUTex = (int) floor(shadowU * Lights.LightInstances[lightIdx].ShadowBufferSize);
            int shadowVTex = (int) floor(shadowV * Lights.LightInstances[lightIdx].ShadowBufferSize);
            uint2 shadowUvInt = uint2(shadowUTex + Lights.LightInstances[lightIdx].ShadowBufferOffsetX, shadowVTex + Lights.LightInstances[lightIdx].ShadowBufferOffsetY);
            
            float shadowMapDepth = lightAtlas[shadowUvInt];
                
            if (shadowMapDepth <= shadowNcdPos.z)
            {
                continue;
            }
                
            float lightDist = distance(Lights.LightInstances[lightIdx].Position.xyz, rayMarchPos);
                
            float3 Li = exp(-lightDist * (sigma_absorbtion + sigma_absorbtion)) * float3(0.9, 0.9, 0.9) * stepSize;
            Li *= transmission * sigma_scattering;
            L += Li;
          
        }
            
        rayMarchPos += stepSize * rayMarchDir;
    }
        
    float toneMap = 1.0f / 2.2f;
    L = L / (L + float3(1.0f, 1.0f, 1.0f));
    L = pow(L, float3(toneMap, toneMap, toneMap));
        
    colorBuffer[screenCoord] += float4(L, 1.0f);    
}