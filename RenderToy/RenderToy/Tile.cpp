#include "Tile.h"
#include "ExtensionFactory.h"


Tile::Tile(std::string tileName, MaterialManager* materialManager, TextureManager2* texManager, float bboxMinX, float bboxMinY, float bboxMaxX, float bboxMaxY)
	: m_tileName(tileName), m_materialManager(materialManager), m_textureManager(texManager), m_bboxMinX(bboxMinX), m_bboxMinY(bboxMinY), m_bboxMaxX(bboxMaxX), m_bboxMaxY(bboxMaxY)
{
}

bool Tile::LoadTileContentsFromFile(std::filesystem::path filePath)
{
	std::filesystem::path rootDir = filePath.parent_path();
	
	bool loadSucc = false;
	std::fstream file(filePath, std::ios::in | std::ios::binary);
	if (file.is_open())
	{
		ResourceCompilerModule::BinaryHeader binaryHeader = {};
		// Only accepts binary type headers
		if (ReadHeader(&file, binaryHeader) && binaryHeader.BinaryType == ResourceCompilerModule::BinaryType::BINARY_TYPE_TILE)
		{
			ResourceCompilerModule::StaticMeshHeader* staticMeshHeaders = nullptr;
			ResourceCompilerModule::StaticMeshInstanceHeader* instanceHeaders = nullptr;
			ResourceCompilerModule::LightExtensionHeader* lightExtensionHeaders = nullptr;

			uint32_t numStaticMeshes = 0;
			uint32_t numInstances = 0;
			uint32_t numLightExtensions = 0;

			loadSucc = GetStaticMeshInstances(&file, binaryHeader, &instanceHeaders, numInstances) &&
							GetStaticMeshes(&file, binaryHeader, &staticMeshHeaders, numStaticMeshes) &&
							GetLightExtensions(&file, binaryHeader, &lightExtensionHeaders, numLightExtensions);
			
			
			if (loadSucc)
			{
				for (uint32_t i = 0; i < numStaticMeshes; i++)
				{
					ResourceCompilerModule::StaticMeshHeader& staticMeshHeader = staticMeshHeaders[i];
					std::unique_ptr<StaticMeshWrapper> staticMeshWrapper = std::make_unique<StaticMeshWrapper>();
					staticMeshWrapper->isFromeFile = true;
					staticMeshWrapper->filePath = rootDir / staticMeshHeader.FileName;
					staticMeshWrapper->pStaticMesh = std::make_unique<StaticMesh2>(staticMeshHeader.MeshName);

					// Get instances of this mesh
					for (uint32_t instanceIdx = 0; instanceIdx < staticMeshHeader.InstanceCount; instanceIdx++)
					{
						ResourceCompilerModule::StaticMeshInstanceHeader& instanceHeader = instanceHeaders[instanceIdx + staticMeshHeader.InstanceIndex];

						Transform transform = {};
						memcpy(&transform.Rotation, instanceHeader.Rotation, sizeof(FRotator));
						memcpy(&transform.Translation, instanceHeader.Position, sizeof(FTranslation));
						memcpy(&transform.Scale, instanceHeader.Scale, sizeof(FScale));
						staticMeshWrapper->pStaticMesh->AddInstance(transform);
						// Create light extension if it has
						if (instanceHeader.HasLightExtension)
						{
							ResourceCompilerModule::LightExtensionHeader& lightExtHeader = lightExtensionHeaders[instanceHeader.LightExtensionIndex];
							ExtensionFactory::SpawnLightExtension(
								staticMeshWrapper->pStaticMesh.get(), 
								instanceIdx,
								lightExtHeader.Offset,
								lightExtHeader.Rotation,
								lightExtHeader.Color,
								lightExtHeader.Intensity,
								lightExtHeader.AttenuationRadius,
								lightExtHeader.AspectRatio,
								lightExtHeader.Fov);
						}
						
					}

					m_staticMeshes.push_back(std::move(staticMeshWrapper));
				}
			}

			if (numStaticMeshes > 0)
			{
				delete[] staticMeshHeaders;
			}
			if (numInstances > 0)
			{
				delete[] instanceHeaders;
			}
			if (numLightExtensions > 0)
			{
				delete[] lightExtensionHeaders;
			}
		}
	}

	file.close();

	return loadSucc;
}

void Tile::LoadMeshFromFile(
	std::filesystem::path filePath,
	ResourceCompilerModule::StaticMeshDefinitionHeader* staticMeshDefinitionHeaders, ResourceCompilerModule::MeshPartHeader* meshPartHeaders,
	ResourceCompilerModule::MaterialHeader* materialHeaders,
	uint32_t numStaticMeshDefinitions,
	uint32_t numMeshParts,
	uint32_t numMaterials)
{
	for (auto& staticMeshWrapper : m_staticMeshes)
	{
		if (!staticMeshWrapper->isFromeFile)
		{
			continue;
		}

		try {
			if (std::filesystem::equivalent(staticMeshWrapper->filePath, filePath))
			{
				// Try to find the parts in the file
				for (uint32_t meshDefIdx = 0; meshDefIdx < numStaticMeshDefinitions; meshDefIdx++)
				{
					ResourceCompilerModule::StaticMeshDefinitionHeader& meshDefHeader = staticMeshDefinitionHeaders[meshDefIdx];
					if (meshDefHeader.MeshName == staticMeshWrapper->pStaticMesh->GetName())
					{
						// found the mesh definition.
						for (uint32_t meshPartIdx = meshDefHeader.PartsIdx; meshPartIdx < meshDefHeader.PartsIdx + meshDefHeader.NumParts; meshPartIdx++)
						{
							ResourceCompilerModule::MeshPartHeader& meshPartHeader = meshPartHeaders[meshPartIdx];
							staticMeshWrapper->meshPartOffsets.push_back(meshPartHeader.MeshDataOffset);
							staticMeshWrapper->meshPartSizes.push_back(meshPartHeader.MeshDataSize);
							staticMeshWrapper->materialNames.push_back(materialHeaders[meshPartHeader.MaterialIdx].MaterialName);
						}

						break;
					}
				}
			}
		}
		catch (std::exception)
		{
			continue;
		}
	}
}

bool Tile::StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder)
{
	if (m_streamedIn)
	{
		return true;
	}

	if (!cmdBuilder)
	{
		return false;
	}

	for (auto& mesh : m_staticMeshes)
	{
		std::fstream meshFile(mesh->filePath, std::ios::in | std::ios::binary);
		if (meshFile.is_open())
		{
			mesh->pStaticMesh->LoadFromBinary(graphicsContext, m_materialManager, m_textureManager, cmdBuilder, &meshFile, mesh->meshPartOffsets, mesh->meshPartSizes, mesh->materialNames);
			meshFile.close();
		}
	}

	m_streamedIn = true;

	return true;
}

bool Tile::StreamOut()
{
	return true;
}

bool Tile::CleanUpAfterStreamIn()
{
	return true;
}

bool Tile::IsPointInTile(float x, float y)
{
	if (x >= m_bboxMinX && x <= m_bboxMaxX &&
		y >= m_bboxMinY && y <= m_bboxMaxY)
	{
		return true;
	}

	return false;
}

void Tile::GetBBox(float& outXMin, float& outXMax, float& outYMin, float& outYMax)
{
	outXMin = m_bboxMinX;
	outXMax = m_bboxMaxX;
	outYMin = m_bboxMinY;
	outYMax = m_bboxMaxY;
}

void Tile::GetAllStaticMeshes(std::vector<StaticMesh2*>& outStaticMeshes)
{
	for (auto& mesh : m_staticMeshes)
	{
		outStaticMeshes.push_back(mesh->pStaticMesh.get());
	}
}

bool Tile::UpdateBuffersForFrame()
{
	if (!m_streamedIn)
	{
		return false;
	}

	for (auto& staticMesh : m_staticMeshes)
	{
		staticMesh->pStaticMesh->UpdateBuffersForFrame();
	}

	return true;
}