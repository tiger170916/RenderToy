#pragma once

#include "Includes.h"
#include "MaterialManager.h"
#include "TextureManager2.h"
#include "PassType.h"

// Drawable interface
class IDrawable
{
public:
	/// <summary>
	/// Drawable Interface functions
	/// </summary>
	virtual void Draw(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* textureManager, ID3D12GraphicsCommandList* cmdList, PassType passType) = 0;
};