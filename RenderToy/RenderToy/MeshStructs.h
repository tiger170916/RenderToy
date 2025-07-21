#pragma once
#include "Includes.h"

struct MeshVertexDx
{
	float position[3];
};

struct MeshInstanceConstants
{
	XMFLOAT4X4 TransformMatrix;
};