#pragma once

#include "ResourceLoader.h"
#include "ResourceHeaders.h"
#include "ResourceCompilerTileImpl.h"
#include "ResourceCompilerMeshesImpl.h"
#include "ResourceCompilerTexturesImpl.h"
#include <memory>
#include <fstream>

using namespace ResourceCompilerModule;

/// <summary>
/// Resource loader class implementation
/// </summary>
class ResourceLoaderImpl : public ResourceLoader
{
private:
	ResourceType m_type = ResourceType::RESOURCE_TYPE_NONE;

	std::shared_ptr<TileDataImpl> m_tileData;

	std::shared_ptr<MeshesDataImpl> m_meshesData;

	std::shared_ptr<TexturesDataImpl> m_texturesData;

	BinaryHeader m_header = {};

	bool m_loadedFile = false;

public:
	// Implementation interfaces
	virtual ResourceType GetResourceType() const override { return m_type; }

	virtual TileData* GetTileData() override;

	virtual MeshesData* GetMeshesData() override;

	virtual TexturesData* GetTexturesData() override;

	bool LoadFile(const char* filePath);

private:
	bool LoadTileData(std::fstream& file);

	bool LoadMeshesData(std::fstream& file);

	bool LoadTexturesData(std::fstream& file);
};