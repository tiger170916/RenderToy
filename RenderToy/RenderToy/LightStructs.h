#pragma once

#include "Includes.h"

struct LightConstants
{
	XMFLOAT4X4 Transform;

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
    UINT Pad0[2];
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