
#define MAX_MESH_INSTANCE_NUM           64

struct MeshInstanceConstants
{
    float4x4 TransformMatrix;
    
    uint4 Uid;
    
    float4 LightEmission;
};

// Full version of mesh vertex structures
struct MeshVsIn
{
    float3 pos : POSITION0;
    
    float2 uv : TEXCOORD0;
};

struct MeshPsIn
{
    float4 pos : SV_Position;
    
    float4 worldPos : POSITION0;
    
    float2 uv : TEXCOORD0;
    
    uint instanceId : SV_InstanceID;
};


// Simple version of mesh vertex structures
struct MeshVsInSimple
{
    float3 pos : POSITION;
};

struct MeshPsInSimple
{
    float4 pos : SV_Position;
};


// vertex structure for screen quad
struct MeshVsInScreenQuad
{
    float2 pos : POSITION0;
};


// vertex structures for shadow pass
struct MeshGsInShadow
{
    float4 pos : SV_POSITION;
        
    uint Instance : InstanceID;
};

struct MeshPsInShadow
{
    float4 pos : SV_Position;
    
    uint LightIndex : LIGHT_INDEX;
};

//