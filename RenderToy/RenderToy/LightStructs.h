#pragma once

#include "Includes.h"

struct LightConstants
{
	XMFLOAT4X4 Transform;

    XMFLOAT4X4 ViewMatrix;

    float Position[4];

    float Intensity[4];

    //
    UINT LightUid;
    UINT LightParentUid;
    UINT ShadowBufferOffsetX;
    UINT ShadowBufferOffsetY;

    //
    UINT ShadowBufferSize;
    UINT LightType;
    UINT ParentUid;
    UINT Pad0;

    float NearPlane;
    float FarPlane;
    float Pad1[2];
};


struct LightConstantsDx
{
    UINT NumLights[4];

    LightConstants Lights[50];
};

struct LightViewFrustumVertexDx
{
    float position[3];
};

struct LightViewFrustumConstantsDX
{
    XMFLOAT4X4 Transform;
};