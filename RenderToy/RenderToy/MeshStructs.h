#pragma once
#include "Includes.h"

struct MeshVertexDx
{
	float position[3];

	float UV0[2];

	float Normal[3];
};

struct MeshVertexSimpleDx
{
	float position[3];
};

struct FullScreenVertexDx
{
	float position[2];
};

struct MeshInstanceConstants
{
	XMFLOAT4X4 TransformMatrix;

	uint32_t Uid[4];

	float LightEmission[4];
};