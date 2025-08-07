#pragma once
#include "Includes.h"

struct UniformFrameConstants
{
	XMFLOAT4X4 ViewMatrix;
	XMFLOAT4X4 ProjectionMatrix;
	float CameraPostion[4];
};