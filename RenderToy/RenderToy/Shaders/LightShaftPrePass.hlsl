

#define LightShaftPrePassRootsignature \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)," \
    "DescriptorTable(" \
        "CBV(b0, numDescriptors = 1)" \
    "),"\
    "DescriptorTable(" \
        "CBV(b1, numDescriptors = 1)" \
    "),"

struct LightShaftPrePassVertexIn
{
    float3 pos : POSITION0;
};

struct LightShaftPrePassVertexOut
{
    float4 pos : SV_Position;
    
    float4 worldPos : POSITION0;
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

cbuffer cbFrustumMeshConstants : register(b1)
{
    float4x4 TransformMatrix;
};



[RootSignature(LightShaftPrePassRootsignature)]
LightShaftPrePassVertexOut VertexShaderMain(LightShaftPrePassVertexIn vertexIn)
{
    LightShaftPrePassVertexOut output;
    
    // Transform point to homogeneous space.
    
    float4 pos = mul(float4(vertexIn.pos, 1.0f), TransformMatrix);
    output.worldPos = pos;
    pos = mul(pos, gView);
    pos = mul(pos, gProjection);
    output.pos = pos;
    
    return output;
}

[RootSignature(LightShaftPrePassRootsignature)]
float4 PixelShaderMain(LightShaftPrePassVertexOut vertexOut) : SV_Target0
{
    /*
    float z = vertexOut.pos.z;
 
    
    uint x = (uint) floor(vertexOut.pos.x);
    uint y = (uint) floor(vertexOut.pos.y);
    
    uint zDepthUint = asuint(z);
    uint originalUint;

    InterlockedMin(depthBuffer[uint2(x, y)], zDepthUint, originalUint);*/
    return vertexOut.worldPos / vertexOut.worldPos.w;

}

[RootSignature(LightShaftPrePassRootsignature)]
float4 DebugPixelShaderMain(LightShaftPrePassVertexOut vertexOut) : SV_Target0
{
    return float4(0.5f, 0.5f, 0.5f, 1.0f);
}