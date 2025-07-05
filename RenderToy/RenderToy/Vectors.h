#pragma once

typedef struct FVector3
{
	float X;
	float Y;
	float Z;

	static FVector3 One()
	{
		return FVector3(1.0f, 1.0f, 1.0f);
	}

	static FVector3 Zero()
	{
		return FVector3(0.0f, 0.0f, 0.0f);
	}

} FRotation, FTranslation, FScale;


struct Transform
{
	FTranslation Translation;
	FRotation Rotation;
	FScale Scale;

	static Transform Identity()
	{
		return Transform{
			.Translation = FTranslation::Zero(), 
			.Rotation = FRotation::Zero(),
			.Scale = FScale::One() };
	}
};