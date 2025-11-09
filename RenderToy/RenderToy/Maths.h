#pragma once
#include "Includes.h"
#include "Vectors.h"

namespace Maths
{
	void Normalize(FVector3& vec3);

	FVector3 CrossProduct(const FVector3& v1, const FVector3& v2);
}