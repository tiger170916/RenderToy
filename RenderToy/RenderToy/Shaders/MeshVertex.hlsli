cbuffer cbUniformFrameConstants : register(b0)
{
    float4x4 gView;
    float4x4 gProjection;
    float4 gCameraPosition;
};

struct MeshInstanceConstants
{
    float4x4 TransformMatrix;
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

