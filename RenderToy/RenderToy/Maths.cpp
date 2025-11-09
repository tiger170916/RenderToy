#include "Maths.h"

void Maths::Normalize(FVector3& vec3)
{
	float divider = sqrt(vec3.X * vec3.X + vec3.Y * vec3.Y + vec3.Z * vec3.Z);
	vec3 /= divider;
}

FVector3 Maths::CrossProduct(const FVector3& v1, const FVector3& v2)
{
	FVector3 result = {};
	result.X = v1.Y * v2.Z - v1.Z * v2.Y;
	result.Y = v1.Z * v2.X - v1.X * v2.Z;
	result.Z = v1.X * v2.Y - v1.Y * v2.X;

	return result;
}