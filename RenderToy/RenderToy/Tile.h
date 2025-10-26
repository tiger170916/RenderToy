#pragma once
#include "Includes.h"
#include "ResourceCompilerModuleApi.h"
#include "StaticMesh2.h"
#include "SceneObject.h"
#include "InstancedStaticMesh.h"
#include "IStreamable.h"
#include "Vectors.h"
#include "MaterialManager.h"
#include "TextureManager2.h"

/// <summary>
/// Tile class
/// </summary>
class Tile : public IStreamable
{
private:
	std::filesystem::path m_binaryPath;

private:
	// Bounding box
	float m_bboxMinX = 0.0f;
	float m_bboxMinY = 0.0f;
	float m_bboxMaxX = 0.0f;
	float m_bboxMaxY = 0.0f;

	std::string m_tileName;
	
	// All scene objects
	std::vector<std::unique_ptr<SceneObject>> m_sceneObjects;

	std::vector<IMesh*> m_activeMeshes;

	MaterialManager* m_materialManager = nullptr;
	TextureManager2* m_textureManager = nullptr;


public:
	Tile(std::string tileName, MaterialManager* materialManager, TextureManager2* texManager, float bboxMinX, float bboxMinY, float bboxMaxX, float bboxMaxY);

	bool LoadTileContentsFromFile(std::filesystem::path filePath);

	bool IsPointInTile(float x, float y);

	/// <summary>
	/// Load mesh metadata from this file stream
	/// </summary>
	void LoadMeshFromFile(
		std::filesystem::path filePath, 
		ResourceCompilerModule::StaticMeshDefinitionHeader* staticMeshDefinitionHeaders, ResourceCompilerModule::MeshPartHeader* meshPartHeaders,
		ResourceCompilerModule::MaterialHeader* materialHeaders,
		uint32_t numStaticMeshDefinitions,
		uint32_t numMeshParts,
		uint32_t numMaterials);

public:
	// IStreamable interface
	virtual bool StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder) override;

	virtual bool StreamOut() override;

	virtual bool CleanUpAfterStreamIn() override;

	void GetBBox(float& outXMin, float& outXMax, float& outYMin, float& outYMax);

	inline const std::vector<IMesh*>& GetAllMeshes() { return m_activeMeshes; }

	bool UpdateBuffersForFrame();
};