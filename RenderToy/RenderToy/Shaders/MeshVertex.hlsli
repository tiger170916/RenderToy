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

struct MeshInstanceConstants
{
    float4x4 TransformMatrix;
    
    uint4 Uid;
    
    float4 LightEmission;
};

cbuffer cbMeshInstanceConstants : register(b1)
{
    MeshInstanceConstants MeshInstances[64];
    
};

struct MeshVertexIn
{
    float3 pos : POSITION0;
    
    float2 uv : TEXCOORD0;
};

struct MeshVertexOut
{
    float4 pos : SV_Position;
    
    float4 worldPos : POSITION0;
    
    float2 uv : TEXCOORD0;
    
    uint instanceId : SV_InstanceID;
};

