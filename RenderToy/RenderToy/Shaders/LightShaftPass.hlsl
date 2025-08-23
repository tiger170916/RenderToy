
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
    float4x4 gInvView;
    float4x4 gProjection;
    float4x4 gInvProjectionMatrix;
    float4x4 gViewProjectionMatrix;
    float4x4 gInvViewProjectionMatrix;
    
    float4 gCameraPosition;
    float4 gForwardVector;
    
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

float hash(float2 uv)
{
    return frac(sin(uv.x) * sin(uv.y) * 125545.545);
}

void EquiAngularSample(float3 rayOrigin, float3 rayDir, float3 lightPos, float maxDist, float xi, out float dist, out float pdf)
{
    float delta = dot(lightPos - rayOrigin, rayDir);
	
    float D = length(rayOrigin + delta * rayDir - lightPos);

	// get angle of endpoints
    float thetaA = atan(- delta / D);
    float thetaB = atan((maxDist - delta) / D);
	
	// take sample
    float t = D * tan(lerp(thetaA, thetaB, xi));
    dist = delta + t;
    pdf = D / ((thetaB - thetaA) * (D * D + t * t));
}

float3 EvalLight(int numSamples, float3 rayOrigin, float3 rayDir, float maxDist, float xi, float backPosZ)
{
    float3 L = float3(0.0f, 0.0f, 0.0f);
    float3 irradiance = float3(0.0f, 0.0f, 0.0f);
    for (int lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
    {
        LightConstants light = Lights.LightInstances[lightIdx];
        float particleDist;
        float particlePdf;
        EquiAngularSample(rayOrigin, rayDir,  light.Position.xyz, maxDist, xi, particleDist, particlePdf);
        particlePdf *= (float) numSamples;
        
        float3 particlePos = rayOrigin + particleDist * rayDir;
        
        float4 particalPosView = mul(float4(particlePos, 1.0f), gView);
        particalPosView /= particalPosView.w;
        
        if (backPosZ != 0.0f && particalPosView.z >= backPosZ)
        {
            continue;
        }
        
        float4 shadowNcdPos = mul(float4(particlePos, 1.0f), light.Transform);
        shadowNcdPos /= shadowNcdPos.w;
        
        
        if (shadowNcdPos.z < 0.0f)
        {
            continue;
        }
        
        float lightAngle = pow(shadowNcdPos.x, 2.0f) + pow(shadowNcdPos.y, 2.0f);
        if (lightAngle > 1.0f)
        {
            continue;
        }
        
        // Transform to texture space.
        float shadowU = (shadowNcdPos.x + 1.0) / 2.0;
        float shadowV = (1.0 - shadowNcdPos.y) / 2.0;
                
        int shadowUTex = (int) floor(shadowU * light.ShadowBufferSize);
        int shadowVTex = (int) floor(shadowV * light.ShadowBufferSize);
        //uint2 shadowUvInt = uint2(shadowUTex + light.ShadowBufferOffsetX, shadowVTex + light.ShadowBufferOffsetY);
            
        //float shadowMapDepth = lightAtlas[shadowUvInt];
                
        //if (shadowMapDepth <= shadowNcdPos.z)
        //{
        //    continue;
        //}
        
        float shadow = 0.0f;
        for (int m = -3; m <= 3; m++)
        {
            int u = shadowUTex + m;
            if (u < 0 || u >= light.ShadowBufferSize)
            {
                continue;
            }
            
            for (int n = -3; n <= 3; n++)
            {
                int v = shadowVTex + n;
                if (v < 0 || v >= light.ShadowBufferSize)
                {
                    continue;
                }
                
                uint2 shadowUvInt = uint2(u + light.ShadowBufferOffsetX, v + light.ShadowBufferOffsetY);
                float shadowMapDepth = lightAtlas[shadowUvInt];
                
                if (shadowNcdPos.z > shadowMapDepth)
                {
                    shadow += 1.0f;
                }
            }
        }
        
        shadow = shadow / 9.0f;
        
        if (shadow >= 1.0f)
        {
            continue;
        }

        
        float lightDist = distance(light.Position.xyz, particlePos);
        float distAttenuation = (lightDist / light.FarPlane) * (lightDist / light.FarPlane);
                
        float trans = exp(-(sigma_absorbtion + sigma_absorbtion) * (particleDist + lightDist));
        float3 Li = exp(-lightDist * (sigma_absorbtion + sigma_absorbtion)) * float3(5.0f, 5.0f, 5.0f) * lightAngle * (1.0f - distAttenuation) * (1.0f - shadow) / particlePdf;
        L += Li * sigma_scattering;
    }

    return L;
}

void EvalLight(
    float3 particlePosWorld,
    float shadingPointViewZ,
    float particleDist,
    float particlePdf,
    LightConstants light,
    out float3 L,
    out bool endTracing)
{	        
    endTracing = false;
    // Get the position of the sample particle in view space.
    float4 particalPosViewSpace = mul(float4(particlePosWorld, 1.0f), gView);
    particalPosViewSpace /= particalPosViewSpace.w;
    
    if (shadingPointViewZ != 0.0f && particalPosViewSpace.z >= shadingPointViewZ)
    {
        // The sample point is behind the shading point. end the ray marching here.
        endTracing = true;
        return;
    }
        
    // Get the position of the sample particle in the light ndc space
    float4 particalPosInLightNdc = mul(float4(particlePosWorld, 1.0f), light.Transform);
    particalPosInLightNdc /= particalPosInLightNdc.w;
        
    if (particalPosInLightNdc.z < 0.0f)
    {
        return;
    }
        
    float lightAngle = pow(particalPosInLightNdc.x, 2.0f) + pow(particalPosInLightNdc.y, 2.0f);
    if (lightAngle > 1.0f)
    {
        return;
    }
        
    // Transform to texture space.
    float shadowU = (particalPosInLightNdc.x + 1.0) / 2.0;
    float shadowV = (1.0 - particalPosInLightNdc.y) / 2.0;
                
    // Get shadow atlas texture coordinates.
    int shadowUTex = (int) floor(shadowU * light.ShadowBufferSize);
    int shadowVTex = (int) floor(shadowV * light.ShadowBufferSize);
        
    // Apply pcf shadow flitering
    float shadow = 0.0f;
    for (int m = -3; m <= 3; m++)
    {
        int u = shadowUTex + m;
        if (u < 0 || u >= light.ShadowBufferSize)
        {
            continue;
        }
            
        for (int n = -3; n <= 3; n++)
        {
            int v = shadowVTex + n;
            if (v < 0 || v >= light.ShadowBufferSize)
            {
                continue;
            }
                
            uint2 shadowUvInt = uint2(u + light.ShadowBufferOffsetX, v + light.ShadowBufferOffsetY);
            float shadowMapDepth = lightAtlas[shadowUvInt];
                
            if (particalPosInLightNdc.z > shadowMapDepth)
            {
                shadow += 1.0f;
            }
        }
    }
        
    shadow = shadow / 9.0f;
        
    if (shadow >= 1.0f)
    {
        return;
    }
        
    float lightDist = distance(light.Position.xyz, particlePosWorld);
    float distAttenuation = (lightDist / light.FarPlane) * (lightDist / light.FarPlane);
                
    float trans = exp(-(sigma_absorbtion + sigma_absorbtion) * (particleDist + lightDist));
    float3 Li = exp(-1.0f * (sigma_absorbtion + sigma_absorbtion)) * float3(5.0f, 5.0f, 0.0f);// * lightAngle * (1.0f - distAttenuation) * (1.0f - shadow) / particlePdf;
    L += Li * sigma_scattering;
}

void RayTraceRange(
    float3 originPos,     // Ray starting point
    float3 rayDir,        // ray direction
    float startDist,      // staring distance on the ray
    float endDist,        // ending distance on the ray
    float shadingPointZ,  // depth in view space of the shading point (0.0f if no shading point)
    int numSamples,       // number of samples on range [startDist, endDist]
    LightConstants light, // light source
    float offset,         // random offset to reduce halo artifact    
    out float3 L          // light output
)
{
    L = float3(0.0f, 0.0f, 0.0f);
    float3 startPos = originPos + startDist * rayDir;
    float3 endPost = originPos + endDist * rayDir;
    float maxDist = endDist - startDist;
    
    // calculate angles and distances for equi-angular sampling
    float delta = dot(light.Position.xyz - startPos, rayDir);
    float D = length(startPos + delta * rayDir - light.Position.xyz);

	// get angle of endpoints
    float thetaA = atan(-delta / D);
    float thetaB = atan((maxDist - delta) / D);
    
    for (int i = 0; i < numSamples; i++)
    {
        // equi-angular sampling
        float xi = ((float) i + offset) / (float) numSamples;
        float t = D * tan(lerp(thetaA, thetaB, xi));
        float particleDist = delta + t;
        float particlePdf = D / ((thetaB - thetaA) * (D * D + t * t));
        particlePdf *= (float) numSamples;
        
        float3 particalPosWorld = startPos + particleDist * rayDir;
        
        bool endTracing = false;
        EvalLight(particalPosWorld, shadingPointZ, particleDist, particlePdf, light, L, endTracing);
        
        if (endTracing)
        {
            return;
        }
    }
}

[RootSignature(LightShaftPassRootsignature)]
void PixelShaderMain(MeshVertexOut vertexOut)
{
    /*
    uint texWidth;
    uint texHeight;
    
    positionBuffer.GetDimensions(texWidth, texHeight);
    // Get screen coordinate of the pixel
    uint2 screenSpaceCoord = (uint2) floor(vertexOut.pos.xy); 
    // Get normalized screen coordinate of the pixel
    float2 normalizedSpaceCoord = float2((float) screenSpaceCoord.x / (float) texWidth * 2.0f - 1.0f, 1.0f - (float) screenSpaceCoord.y / (float) texHeight * 2.0f);

    // Get near point of the pixel in world space
    float4 nearPoint = mul(float4(normalizedSpaceCoord.x, normalizedSpaceCoord.y, 0.01f, 1.0f), gInvViewProjectionMatrix);
    nearPoint /= nearPoint.w;
    
    // Get ray direction vector
    float3 rayDir = normalize(nearPoint.xyz - gCameraPosition.xyz);
    
    float4 shadingPointWorldPos = positionBuffer[screenSpaceCoord];
    float4 shadingPointViewPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Get the shading point in view space. the float4 vector is zero if there is not shading point at current pixel
    if (shadingPointWorldPos.w > 0.0f)
    {
        shadingPointViewPos = mul(shadingPointWorldPos, gView);
    }
    
    float3 originPosition = nearPoint.xyz;
    float offset = hash(vertexOut.pos.xy);  // random offset to reduce halo artifact
    
    // Light caused by inscattering
    float3 L = float3(0.0f, 0.0f, 0.0f);
    
    // Do ray marching per light source
    for (int lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
    {
        LightConstants light = Lights.LightInstances[lightIdx];
        // Range 1 : [0.1, 5.0]
        RayTraceRange(originPosition, rayDir, 0.1f, 5.0f, shadingPointViewPos.z, 1, light, offset, L);
        // Range 2 : [5.0, 15.0]
        //RayTraceRange(originPosition, rayDir, 5.0f, 15.0f, shadingPointViewPos.z, 20, light, offset, L);
        // Range 3 : [15.0, 25.0]
        //RayTraceRange(originPosition, rayDir, 15.0f, 25.0f, shadingPointViewPos.z, 20, light, offset, L);
        // Range 4 : [25.0, 35.0]
        //RayTraceRange(originPosition, rayDir, 25.0f, 35.0f, shadingPointViewPos.z, 20, light, offset, L);
    }
    
    float toneMap = 1.0f / 2.2f;
    L = L / (L + float3(1.0f, 1.0f, 1.0f));
    L = pow(L, float3(toneMap, toneMap, toneMap));
        
    float4 color = colorBuffer[screenSpaceCoord];
    color += float4(L, 0.0f);
    colorBuffer[screenSpaceCoord] = color;
        */
    
    uint texWidth;
    uint texHeight;
    
    positionBuffer.GetDimensions(texWidth, texHeight);
    // Get screen coordinate of the pixel
    uint2 screenSpaceCoord = (uint2) floor(vertexOut.pos.xy);
    // Get normalized screen coordinate of the pixel
    float2 normalizedSpaceCoord = float2((float) screenSpaceCoord.x / (float) texWidth * 2.0f - 1.0f, 1.0f - (float) screenSpaceCoord.y / (float) texHeight * 2.0f);
    
    float4 backPosWorld = float4(positionBuffer[screenSpaceCoord].xyz, 1.0f);
    float4 backPosView = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    if (positionBuffer[screenSpaceCoord].w != 0.0f)
    {
        backPosView = mul(backPosWorld, gView);
    } 
    
    float2 ndc = float2((float) screenSpaceCoord.x / (float) texWidth * 2.0f - 1.0f, 1.0f - (float) screenSpaceCoord.y / (float) texHeight * 2.0f);
    
    float4 nearPoint = mul(float4(ndc.x, ndc.y, 0.01f, 1.0f), gInvViewProjectionMatrix);
    nearPoint /= nearPoint.w;
    float3 rayMarchDir = normalize(nearPoint.xyz - gCameraPosition.xyz);
    float4 middlePoint = mul(float4(0.0f, 0.0f, 0.01f, 1.0f), gInvViewProjectionMatrix);
    middlePoint /= middlePoint.w;
    float3 middleDir = normalize(middlePoint.xyz - gCameraPosition.xyz);
   
    float distScale = dot(middleDir, rayMarchDir);
    
    float maxDist = 5.0f / distScale;
    int numSamples = 10;
    float startDist = 0.1f;
    
    float3 rayMarchStart = nearPoint.xyz + rayMarchDir * startDist;
    
    float3 L = 0.0f;
    
    float offset = 0.0f;
    for (int i = 0; i < numSamples; i++)
    {
        float xi = ((float) i + offset) / (float)numSamples;
        L += EvalLight(numSamples, rayMarchStart, rayMarchDir, maxDist, xi, backPosView.z);
    }
    
    numSamples = 25;
    startDist = startDist + maxDist;
    maxDist = 10.0f / distScale;
    rayMarchStart = nearPoint.xyz + rayMarchDir * startDist;
    offset = hash(vertexOut.pos.xy + rayMarchStart.z);
    for (int j = 0; j < numSamples; j++)
    {
        float xi = ((float) j + offset) / (float) numSamples;
        L += EvalLight(numSamples, rayMarchStart, rayMarchDir, maxDist, xi, backPosView.z);
    }
    
    numSamples = 50;
    startDist = startDist + maxDist;
    maxDist = 10.0f / distScale;
    rayMarchStart = nearPoint.xyz + rayMarchDir * startDist;
    offset = hash(vertexOut.pos.xy + rayMarchStart.z);
    for (int k = 0; k < numSamples; k++)
    {
        float xi = ((float) k + offset) / (float) numSamples;
        L += EvalLight(numSamples, rayMarchStart, rayMarchDir, maxDist, xi, backPosView.z);
    }
    
    numSamples = 50;
    startDist = startDist + maxDist;
    maxDist = 10.0f / distScale;
    rayMarchStart = nearPoint.xyz + rayMarchDir * startDist;
    offset = hash(vertexOut.pos.xy + rayMarchStart.z);
    for (int m = 0; m < numSamples; m++)
    {
        float xi = ((float) m + offset) / (float) numSamples;
        L += EvalLight(numSamples, rayMarchStart, rayMarchDir, maxDist, xi, backPosView.z);
    }
    
    
   
        
    
        float toneMap = 1.0f / 2.2f;
    L = L / (L + float3(1.0f, 1.0f, 1.0f));
    L = pow(L, float3(toneMap, toneMap, toneMap));
        
    colorBuffer[screenSpaceCoord] += float4(L, 1.0f);
}