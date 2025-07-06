cbuffer cbUniformFrameConstants : register(b0)
{
    float4x4 gView;
    float4x4 gProjection;
};

struct MeshInstanceConstants
{
    float4x4 TransformMatrix;
};

cbuffer cbMeshInstanceConstants : register(b1)
{
    MeshInstanceConstants MeshInstances[56];
    
};

struct MeshVertexIn
{
    float3 pos : POSITION;
};

struct MeshVertexOut
{
    float4 pos : SV_Position;
};

