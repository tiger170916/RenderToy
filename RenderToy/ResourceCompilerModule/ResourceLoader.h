#pragma once
#include "ResourceCompilerTile.h"
#include "ResourceCompilerMeshes.h"
#include "ResourceCompilerTextures.h"

namespace ResourceCompilerModule {

	/// <summary>
	/// Resource Type enum
	/// </summary>
	enum class ResourceType
	{
		RESOURCE_TYPE_NONE = 0,
		RESOURCE_TYPE_TILE = 1,
		RESOURCE_TYPE_MESH = 2,
		RESOURCE_TYPE_TEXTURE = 3,
		RESOURCE_TYPE_MAX = RESOURCE_TYPE_TEXTURE,
	};

	/// <summary>
	/// Resource loader interface
	/// </summary>
	class ResourceLoader
	{
	public:
		virtual ResourceType GetResourceType() const = 0;

		virtual ResourceCompilerModule::TileData* GetTileData() = 0;

		virtual ResourceCompilerModule::MeshesData* GetMeshesData() = 0;

		virtual ResourceCompilerModule::TexturesData* GetTexturesData() = 0;
	};
}