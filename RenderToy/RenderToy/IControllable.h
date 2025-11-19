#pragma once
#include "Includes.h"

// Fixed axis and action bindings for now
struct InputStruct
{
	float MouseXAxis = 0.0f;
	float MouseYAxis = 0.0f;

	float ForwardAxis = 0.0f;
	float RightAxis = 0.0f;

	bool SwitchCamera = false;
	bool SwitchCharacter = false;
};

class IControllable
{
public:
	virtual void ProcessInput(InputStruct inputStruct) = 0;
};