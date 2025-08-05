#define LIGHT_TYPE_POINT			0
#define LIGHT_TYPE_SPOT				1


struct LightConstants
{
    float4x4 Transform;

    float4 Position;

    float4 Intensity;

    //
    uint LightUid;
    uint LightParentUid;
    uint ShadowBufferOffsetX;
    uint ShadowBufferOffsetY;

    //
    uint ShadowBufferSize;
    uint LightType;
    uint2 Pad0[2];
};
