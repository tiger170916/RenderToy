#pragma once
#include "Includes.h"

struct MeshVertex
{
	float position[3];
};

struct MeshInstanceConstants
{
	XMMATRIX TransformMatrix;
};