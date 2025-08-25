#pragma once
#include "Includes.h"

struct UniformFrameConstants
{
	XMFLOAT4X4  ViewMatrix;
	XMFLOAT4X4  InvViewMatrix;
	XMFLOAT4X4  ProjectionMatrix;
	XMFLOAT4X4  InvProjectionMatrix;
	XMFLOAT4X4  ViewProjectionMatrix;
	XMFLOAT4X4  InvViewProjectionMatrix;

	float		CameraPostion[4];
	float		ForwardVector[4];

	float		PixelStepScale;
	float		Pad[3];

	uint32_t RenderTargetWidth;
	uint32_t RenderTargetHeight;
	float PixelWidthInNdc;
	float PixelHeightInNdc;
};