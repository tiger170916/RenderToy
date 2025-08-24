#include "GlobalHeader.hlsli"
#include "MeshHeader.hlsli"
#include "LightingHeader.hlsli"

static float sigma_absorbtion = 0.00005f;
static float sigma_scattering = 0.00015f;
static float asymettric_coeff = 0.95f;

// arg0: uniform cb
// arg1: lights cb
// arg2: colorBuffer
// arg3: positionBuffer
// arg4: lightFrustumPositionBuffer
// arg5: lightAtlas
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


cbuffer cbUniformFrameConstants                 : register(b0)
{
    UniformFrameConstants gUniformFrameConstants;
};

cbuffer cbLightTransformInstances               : register(b1)
{
    LightBuffer Lights;
};

RWTexture2D<float4> colorBuffer                 : register(u0);

RWTexture2D<float4> positionBuffer              : register(u1);

RWTexture2D<float4> lightFrustumPositionBuffer  : register(u2);

RWTexture2D<float> lightAtlas                   : register(u3);


[RootSignature(LightShaftPassRootsignature)]
MeshPsInSimple VertexShaderMain(MeshVsInScreenQuad input)
{
    MeshPsInSimple output;
    
    output.pos = float4(input.pos, 0.1f, 1.0f);

    return output;
}

float hash(float2 uv)
{
    return frac(sin(uv.x) * sin(uv.y) * 125545.545);
}

float HenyeyGreensteinPhase(float3 lightDir, float3 scatterInDir, float asymmetric)
{
    float a_square = asymmetric * asymmetric;
    return ONE_OVER_4PI * (1.0f - a_square) / pow((1.0f + a_square - 2.0f * asymmetric * dot(lightDir, scatterInDir)), 1.5f);
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

void EvalLight(
    float3 particlePosWorld,
    float shadingPointViewZ,
    float particleDist,
    float particlePdf,
    float distDelta,  // distance from the last step to current (stepsize)
    LightConstants light,
    float phase,
    inout float3 outL,
    inout float outTransmittance,
    inout bool outEndTracing)
{	        
    if (outTransmittance <= 0.01)
    {
        outEndTracing = true;
        return;
    }
    
    // Get the position of the sample particle in view space.
        float4 particalPosViewSpace = mul(float4(particlePosWorld, 1.0f), gUniformFrameConstants.View);
    particalPosViewSpace /= particalPosViewSpace.w;
    
    if (shadingPointViewZ != 0.0f && particalPosViewSpace.z >= shadingPointViewZ)
    {
        // The sample point is behind the shading point. end the ray marching here.
        outEndTracing = true;
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
    float3 Li =  light.Intensity.xyz * exp((-sigma_absorbtion - sigma_absorbtion) * lightDist) * phase * outTransmittance * distDelta * lightAngle * (1.0f - distAttenuation) * (1.0f - shadow) / particlePdf;
    outL += Li * sigma_scattering;
    outTransmittance *= exp((-sigma_absorbtion - sigma_absorbtion) * distDelta);
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
    inout float3 L,       // light output
    inout float transmittance,  // current transmittance along view direct output
    inout bool endTracing // end tracing flag, set to true if ray marching should be stopped
)
{
    float3 startPos = originPos + startDist * rayDir;
    float3 endPost = originPos + endDist * rayDir;
    float maxDist = endDist - startDist;
    
    // calculate angles and distances for equi-angular sampling
    float delta = dot(light.Position.xyz - startPos, rayDir);
    float D = length(startPos + delta * rayDir - light.Position.xyz);

	// get angle of endpoints
    float thetaA = atan(-delta / D);
    float thetaB = atan((maxDist - delta) / D);
    
    float prevActualDist = startDist;
    for (int i = 0; i < numSamples; i++)
    {
        // equi-angular sampling
        float xi = ((float) i + offset) / (float) numSamples;
        float t = D * tan(lerp(thetaA, thetaB, xi));
        float particleDist = delta + t;
        
        float currentActualDist = particleDist + startDist;
        float distDelta = currentActualDist - prevActualDist;
        distDelta = 0.2f;
        float particlePdf = D / ((thetaB - thetaA) * (D * D + t * t));
        particlePdf *= (float) numSamples;
        
        float3 particalPosWorld = startPos + particleDist * rayDir;
        
        float phase = HenyeyGreensteinPhase(normalize(particalPosWorld.xyz - light.Position.xyz), - rayDir, asymettric_coeff);
        EvalLight(particalPosWorld, shadingPointZ, particleDist, particlePdf, distDelta, light, phase, L, transmittance, endTracing);
        
        if (endTracing)
        {
            return;
        }
        
        prevActualDist = currentActualDist;
    }
}

[RootSignature(LightShaftPassRootsignature)]
void PixelShaderMain(MeshPsInSimple input)
{
    uint texWidth;
    uint texHeight;
    
    positionBuffer.GetDimensions(texWidth, texHeight);
    // Get screen coordinate of the pixel
    uint2 screenSpaceCoord = (uint2) floor(input.pos.xy); 
    // Get normalized screen coordinate of the pixel
    float2 normalizedSpaceCoord = float2((float) screenSpaceCoord.x / (float) texWidth * 2.0f - 1.0f, 1.0f - (float) screenSpaceCoord.y / (float) texHeight * 2.0f);

    // Get near point of the pixel in world space
    float4 nearPoint = mul(float4(normalizedSpaceCoord.x, normalizedSpaceCoord.y, 0.01f, 1.0f), gUniformFrameConstants.InvViewProjectionMatrix);
    nearPoint /= nearPoint.w;
    
    // Get ray direction vector
    float3 rayDir = normalize(nearPoint.xyz - gUniformFrameConstants.CameraPosition.xyz);
    
    float4 shadingPointWorldPos = positionBuffer[screenSpaceCoord];
    float4 shadingPointViewPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    
    // Get the shading point in view space. the float4 vector is zero if there is not shading point at current pixel
    if (shadingPointWorldPos.w > 0.0f)
    {
        shadingPointViewPos = mul(shadingPointWorldPos, gUniformFrameConstants.View);
    }
    
    float3 originPosition = nearPoint.xyz;
    float offset = hash(input.pos.xy);  // random offset to reduce halo artifact
    
    // Light caused by inscattering
    float3 L = float3(0.0f, 0.0f, 0.0f);
    
    float totalTransmittance = 1.0f;
    // Do ray marching per light source
    for (int lightIdx = 0; lightIdx < Lights.NumLights[0]; lightIdx++)
    {
        float transmittance = 1.0f;
        bool endTracing = false;
        LightConstants light = Lights.LightInstances[lightIdx];
        // Range 1 : [0.1, 5.0]
        RayTraceRange(originPosition, rayDir, 0.1f, 5.0f, shadingPointViewPos.z, 10, light, offset, L, transmittance, endTracing);
        
        if (!endTracing)
        {
            // Range 2 : [5.0, 15.0]
            RayTraceRange(originPosition, rayDir, 5.0f, 15.0f, shadingPointViewPos.z, 30, light, offset, L, transmittance, endTracing);
        }
        
        if (!endTracing)
        {
            // Range 3 : [15.0, 25.0]
            RayTraceRange(originPosition, rayDir, 15.0f, 25.0f, shadingPointViewPos.z, 70, light, offset, L, transmittance, endTracing);
        }
        
        if (!endTracing)
        {
            // Range 4 : [25.0, 35.0]
            RayTraceRange(originPosition, rayDir, 25.0f, 35.0f, shadingPointViewPos.z, 70, light, offset, L, transmittance, endTracing);
        }
        
        totalTransmittance += transmittance;
    }
    
    float avgTransmittance = totalTransmittance / 4.0f;
    
    float toneMap = 1.0f / 2.2f;
    // tonu map the scattering light
    L = L / (L + float3(1.0f, 1.0f, 1.0f));
    L = pow(L, float3(toneMap, toneMap, toneMap));
        
    float4 colorOrigin = colorBuffer[screenSpaceCoord];
    float4 mixedColor = colorOrigin * (1.0f - avgTransmittance) + float4(L, 0.0f);
    
    // tone map the mixed color again
    mixedColor.xyz = mixedColor.xyz / (mixedColor.xyz + float3(1.0f, 1.0f, 1.0f));
    mixedColor.xyz = pow(mixedColor.xyz, float3(toneMap, toneMap, toneMap));
    
    colorBuffer[screenSpaceCoord] = mixedColor;
}