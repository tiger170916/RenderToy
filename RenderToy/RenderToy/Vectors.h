#pragma once

struct FVector2
{
	float X;
	float Y;

	static FVector2 One()
	{
		return FVector2(1.0f, 1.0f);
	}

	static FVector2 Zero()
	{
		return FVector2(0.0f, 0.0f);
	}

	FVector2& operator+=(const FVector2& rhs) {
		this->X += rhs.X;
		this->Y += rhs.Y;

		return *this;
	}

	FVector2 operator+(const FVector2& rhs) const {
		FVector2 temp = *this;
		temp += rhs;
		return temp;
	}
};

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

	FVector3& operator+=(const FVector3& rhs) {
		this->X += rhs.X;
		this->Y += rhs.Y;
		this->Z += rhs.Z;

		return *this;
	}

	FVector3 operator+(const FVector3& rhs) const {
		FVector3 temp = *this; 
		temp += rhs;  
		return temp;
	}

} FTranslation, FScale;

struct FRotator
{
	float Pitch;
	float Yaw;
	float Roll;

	static FRotator Zero()
	{
		return FRotator(0.0f, 0.0f, 0.0f);
	}
};


struct Transform
{
	FTranslation Translation;
	FRotator Rotation;
	FScale Scale;

	static Transform Identity()
	{
		return Transform{
			.Translation = FTranslation::Zero(), 
			.Rotation = FRotator::Zero(),
			.Scale = FScale::One() };
	}
};