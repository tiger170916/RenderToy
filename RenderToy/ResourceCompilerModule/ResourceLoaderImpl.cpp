#include "ResourceLoaderImpl.h"
#include "ResourceHeaders.h"
#include "InternalStructs.h"
#include <map>

TileData* ResourceLoaderImpl::GetTileData()
{
	if (!m_loadedFile)
	{
		return nullptr;
	}

	if (m_header.BinaryType != BinaryType::BINARY_TYPE_TILE)
	{
		return nullptr;
	}

	return m_tileData.get();
}

MeshesData* ResourceLoaderImpl::GetMeshesData()
{
	if (!m_loadedFile)
	{
		return nullptr;
	}

	if (m_header.BinaryType != BinaryType::BINARY_TYPE_MESHES)
	{
		return nullptr;
	}

	return (MeshesData*)m_meshesData.get();
}

TexturesData* ResourceLoaderImpl::GetTexturesData()
{
	if (!m_loadedFile)
	{
		return nullptr;
	}

	if (m_header.BinaryType != BinaryType::BINARY_TYPE_TEXTURES)
	{
		return nullptr;
	}

	return (TexturesData*)m_texturesData.get();
}

ResourceType ResourceLoaderImpl::ReadResourceType(const char* filePath)
{
	std::fstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		return ResourceType::RESOURCE_TYPE_NONE;
	}

	BinaryHeader header = {};
	file.read((char*)&header, sizeof(BinaryHeader));

	if (header.MagicNum != 0x12345678)
	{
		// Broken file
		file.close();
		return ResourceType::RESOURCE_TYPE_NONE;
	}

	switch (header.BinaryType)
	{
	case BinaryType::BINARY_TYPE_TILE:
		return ResourceType::RESOURCE_TYPE_TILE;
	case BinaryType::BINARY_TYPE_MESHES:
		return ResourceType::RESOURCE_TYPE_MESH;
	case BinaryType::BINARY_TYPE_TEXTURES:
		return ResourceType::RESOURCE_TYPE_TEXTURE;
	}

	// Unsupported type.
	return ResourceType::RESOURCE_TYPE_NONE;
}

bool ResourceLoaderImpl::LoadFile(const char* filePath)
{
	std::fstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.read((char*)&m_header, sizeof(BinaryHeader));

	if (m_header.MagicNum != 0x12345678)
	{
		// Broken file
		file.close();
		return false;
	}

	switch (m_header.BinaryType)
	{
	case BinaryType::BINARY_TYPE_TILE:
		m_type = ResourceType::RESOURCE_TYPE_TILE;
		m_loadedFile = LoadTileData(file);
		break;
	case BinaryType::BINARY_TYPE_MESHES:
		m_type = ResourceType::RESOURCE_TYPE_MESH;
		m_loadedFile = LoadMeshesData(file);
		break;
	case BinaryType::BINARY_TYPE_TEXTURES:
		m_type = ResourceType::RESOURCE_TYPE_TEXTURE;
		break;
	default:
		// Unsupport binary type
		file.close();
		return false;
	}

	file.close();

	return m_loadedFile;
}

bool ResourceLoaderImpl::LoadTileData(std::fstream& file)
{
	
	// Read Strings
	std::vector<std::string> stringTable;
	if (m_header.NumStrings > 0)
	{
		uint32_t* stringStartingIndices = new uint32_t[m_header.NumStrings];
		uint32_t* stringSizes = new uint32_t[m_header.NumStrings];
		char* stringSection = new char[m_header.StringSectionSize];

		file.seekg(m_header.StringStartingPositionsOffset);
		file.read((char*)stringStartingIndices, m_header.NumStrings * sizeof(uint32_t));

		file.seekg(m_header.StringSizesOffset);
		file.read((char*)stringSizes, m_header.NumStrings * sizeof(uint32_t));

		file.seekg(m_header.StringSectionOffset);
		file.read(stringSection, m_header.StringSectionSize);

		for (uint32_t i = 0; i < m_header.NumStrings; i++)
		{
			uint32_t startingPos = stringStartingIndices[i];
			uint32_t strSize = stringSizes[i];
			std::string str(stringSection + startingPos, stringSection + startingPos + strSize);
			stringTable.push_back(str);
		}

		delete[] stringStartingIndices;
		delete[] stringSizes;
		delete[] stringSection;
	}

	// Load Playable Character Objects
	std::vector<PlayableCharacterHeader> playableCharactersInternal;
	if (m_header.NumPlayableCharacters > 0)
	{
		PlayableCharacterHeader* pPlayableCharacterHeader = new PlayableCharacterHeader[m_header.NumPlayableCharacters];

		file.seekg(m_header.PlayableCharactersOffset);
		file.read((char*)pPlayableCharacterHeader, m_header.NumPlayableCharacters * sizeof(PlayableCharacterHeader));
		playableCharactersInternal = std::vector<PlayableCharacterHeader>(pPlayableCharacterHeader, pPlayableCharacterHeader + m_header.NumPlayableCharacters);

		delete[] pPlayableCharacterHeader;
	}

	// Load NonePlayable Character Objects
	std::vector<NonePlayableCharacterHeader> nonePlayableCharactersInternal;
	if (m_header.NumPlayableCharacters > 0)
	{
		NonePlayableCharacterHeader* pNonePlayableCharacterHeader = new NonePlayableCharacterHeader[m_header.NumNonePlayableCharacters];

		file.seekg(m_header.NonePlayableCharactersOffset);
		file.read((char*)pNonePlayableCharacterHeader, m_header.NumNonePlayableCharacters * sizeof(NonePlayableCharacterHeader));
		nonePlayableCharactersInternal = std::vector<NonePlayableCharacterHeader>(pNonePlayableCharacterHeader, pNonePlayableCharacterHeader + m_header.NumNonePlayableCharacters);

		delete[] pNonePlayableCharacterHeader;
	}

	// Load Static Mesh Components
	std::vector<StaticMeshComponentHeader> staticMeshComponentHeadersInternal;
	if (m_header.NumStaticMeshComponents > 0)
	{
		StaticMeshComponentHeader* pStaticMeshComponentHeader = new StaticMeshComponentHeader[m_header.NumStaticMeshComponents];
		file.seekg(m_header.StaticMeshComponentsOffset);
		file.read((char*)pStaticMeshComponentHeader, m_header.NumStaticMeshComponents * sizeof(StaticMeshComponentHeader));
		staticMeshComponentHeadersInternal = std::vector<StaticMeshComponentHeader>(pStaticMeshComponentHeader, pStaticMeshComponentHeader + m_header.NumStaticMeshComponents);
		delete[] pStaticMeshComponentHeader;
	}

	// Load Static Mesh Instances
	std::vector<StaticMeshInstanceHeader> staticMeshHeadersInternal;
	if (m_header.NumStaticMeshInstances > 0)
	{
		StaticMeshInstanceHeader* pStaticMeshInstanceHeader = new StaticMeshInstanceHeader[m_header.NumStaticMeshInstances];
		file.seekg(m_header.StaticMeshInstancesOffset);
		file.read((char*)pStaticMeshInstanceHeader, m_header.NumStaticMeshInstances * sizeof(StaticMeshInstanceHeader));
		staticMeshHeadersInternal = std::vector<StaticMeshInstanceHeader>(pStaticMeshInstanceHeader, pStaticMeshInstanceHeader + m_header.NumStaticMeshInstances);
		delete[] pStaticMeshInstanceHeader;
	}

	// Load Light extensions
	std::vector<LightExtensionHeader> lightExtensionHeadersInternal;
	if (m_header.NumLightExtensions > 0)
	{
		LightExtensionHeader* pLightExtensionHeader = new LightExtensionHeader[m_header.NumLightExtensions];
		file.seekg(m_header.LightExtensionsOffset);
		file.read((char*)pLightExtensionHeader, m_header.NumLightExtensions * sizeof(LightExtensionHeader));
		lightExtensionHeadersInternal = std::vector<LightExtensionHeader>(pLightExtensionHeader, pLightExtensionHeader + m_header.NumLightExtensions);
		delete[] pLightExtensionHeader;
	}

	// Load Camera Arm Component headers
	std::vector<CameraArmComponentHeader> cameraArmComponentHeadersInternal;
	if (m_header.NumCameraArmComponents > 0)
	{
		CameraArmComponentHeader* pCameraArmComponentHeader = new CameraArmComponentHeader[m_header.NumCameraArmComponents];
		file.seekg(m_header.CameraArmComponentsOffset);
		file.read((char*)pCameraArmComponentHeader, m_header.NumCameraArmComponents * sizeof(CameraArmComponentHeader));
		cameraArmComponentHeadersInternal = std::vector<CameraArmComponentHeader>(pCameraArmComponentHeader, pCameraArmComponentHeader + m_header.NumCameraArmComponents);
		delete[] pCameraArmComponentHeader;
	}

	// Load Camera Component headers
	std::vector<CameraComponentHeader> cameraComponentHeadersInternal;
	if (m_header.NumCameraComponents > 0)
	{
		CameraComponentHeader* pCameraComponentHeader = new CameraComponentHeader[m_header.NumCameraComponents];
		file.seekg(m_header.CameraComponentsOffset);
		file.read((char*)pCameraComponentHeader, m_header.NumCameraComponents * sizeof(CameraComponentHeader));
		cameraComponentHeadersInternal = std::vector<CameraComponentHeader>(pCameraComponentHeader, pCameraComponentHeader + m_header.NumCameraComponents);
		delete[] pCameraComponentHeader;
	}

	file.close();

	// Create Playable and NonePlayable characters
	std::vector<std::shared_ptr<PlayableCharacterObjectImpl>> playableCharacters;
	std::vector<std::shared_ptr<NonePlayableCharacterObjectImpl>> nonePlayableCharacters;
	std::vector<std::string> playableCharacterRootComponentNames;
	std::vector<std::string> nonePlayableCharacterRootComponentNames;
	for (size_t i = 0; i < playableCharactersInternal.size(); i++)
	{
		std::shared_ptr<PlayableCharacterObjectImpl> playableCharacter = std::make_shared<PlayableCharacterObjectImpl>(stringTable[playableCharactersInternal[i].NameStringIdx]);
		playableCharacters.push_back(playableCharacter);
		playableCharacterRootComponentNames.push_back(stringTable[playableCharactersInternal[i].RootComponentNameIdx]);
	}

	for (size_t i = 0; i < nonePlayableCharactersInternal.size(); i++)
	{
		std::shared_ptr<NonePlayableCharacterObjectImpl> nonePlayableCharacter = std::make_shared<NonePlayableCharacterObjectImpl>(stringTable[nonePlayableCharactersInternal[i].NameStringIdx]);
		nonePlayableCharacters.push_back(nonePlayableCharacter);
		nonePlayableCharacterRootComponentNames.push_back(stringTable[nonePlayableCharactersInternal[i].RootComponentNameIdx]);
	}

	std::map<std::string, std::shared_ptr<SceneObjectComponentImpl>> allSceneObjectComponents;
	std::map<std::string, uint32_t> allSceneObjectHeaderComponentNameStartingIdx;
	std::map<std::string, uint32_t> allSceneObjectHeaderComponentNameNumbers;

	// Create Static Mesh Components
	std::vector<std::shared_ptr<StaticMeshComponent>> staticMeshComponents;
	for (size_t i = 0; i < staticMeshComponentHeadersInternal.size(); i++)
	{
		std::string fileName = stringTable[staticMeshComponentHeadersInternal[i].FileNameStringIdx];
		std::string name = stringTable[staticMeshComponentHeadersInternal[i].NameStringIdx];
		std::shared_ptr<StaticMeshComponentImpl> staticMeshComponent = std::make_shared<StaticMeshComponentImpl>(name, fileName);

		uint32_t instanceStartIdx = staticMeshComponentHeadersInternal[i].InstanceIndex;
		uint32_t instanceCount = staticMeshComponentHeadersInternal[i].InstanceCount;
		// Create static mesh instances
		for (uint32_t instanceIdx = instanceStartIdx; instanceIdx < instanceStartIdx + instanceCount; instanceIdx++)
		{
			const StaticMeshInstanceHeader& instanceHeader = staticMeshHeadersInternal[instanceIdx];
			std::shared_ptr<StaticMeshInstanceImpl> staticMeshInstance 
				= std::make_shared<StaticMeshInstanceImpl>(
					instanceHeader.Position[0], instanceHeader.Position[1], instanceHeader.Position[2],
					instanceHeader.Rotation[0], instanceHeader.Rotation[1], instanceHeader.Rotation[2],
					instanceHeader.Scale[0], instanceHeader.Scale[1], instanceHeader.Scale[2]);

			if (instanceHeader.HasLightExtension)
			{
				// Create light extension if it exists for this instance
				const LightExtensionHeader& lightExtHeader = lightExtensionHeadersInternal[instanceHeader.LightExtensionIndex];
				std::shared_ptr<LightExtensionImpl> lightExt
					= std::make_shared<LightExtensionImpl>(
						lightExtHeader.LightType,
						lightExtHeader.Offset[0], lightExtHeader.Offset[1], lightExtHeader.Offset[2],
						lightExtHeader.Rotation[0], lightExtHeader.Rotation[1], lightExtHeader.Rotation[2],
						lightExtHeader.Color[0], lightExtHeader.Color[1], lightExtHeader.Color[2],
						lightExtHeader.Intensity,
						lightExtHeader.AttenuationRadius,
						lightExtHeader.AspectRatio,
						lightExtHeader.Fov);
				staticMeshInstance->AttachLightExtension(lightExt);
			}

			staticMeshComponent->AddInstance(staticMeshInstance);
		}

		allSceneObjectComponents[name] = staticMeshComponent;
		allSceneObjectHeaderComponentNameStartingIdx[name] = staticMeshComponentHeadersInternal[i].ComponentsNameStartingIdx;
		allSceneObjectHeaderComponentNameNumbers[name] = staticMeshComponentHeadersInternal[i].NumComponents;

		staticMeshComponents.push_back(staticMeshComponent);
	}

	// Create Camera Arm Components
	std::vector<std::shared_ptr<CameraArmComponent>> cameraArmComponents;
	for (size_t i = 0; i < cameraArmComponentHeadersInternal.size(); i++)
	{
		const CameraArmComponentHeader& cameraArmHeaderInternal = cameraArmComponentHeadersInternal[i];
		std::string name = stringTable[cameraArmHeaderInternal.NameStringIdx];
		std::shared_ptr<CameraArmComponentImpl> cameraArmComponent = std::make_shared<CameraArmComponentImpl>(name, cameraArmHeaderInternal.Length);

		allSceneObjectComponents[name] = cameraArmComponent;
		allSceneObjectHeaderComponentNameStartingIdx[name] = cameraArmComponentHeadersInternal[i].ComponentsNameStartingIdx;
		allSceneObjectHeaderComponentNameNumbers[name] = cameraArmComponentHeadersInternal[i].NumComponents;

		cameraArmComponents.push_back(cameraArmComponent);
	}

	// Create Camera Components
	std::vector<std::shared_ptr<CameraComponent>> cameraComponents;
	for (size_t i = 0; i < cameraArmComponentHeadersInternal.size(); i++)
	{
		const CameraComponentHeader& cameraHeaderInternal = cameraComponentHeadersInternal[i];

		std::string name = stringTable[cameraHeaderInternal.NameStringIdx];
		std::shared_ptr<CameraComponentImpl> cameraComponent = std::make_shared<CameraComponentImpl>(name);
	
		allSceneObjectComponents[name] = cameraComponent;
		allSceneObjectHeaderComponentNameStartingIdx[name] = cameraComponentHeadersInternal[i].ComponentsNameStartingIdx;
		allSceneObjectHeaderComponentNameNumbers[name] = cameraComponentHeadersInternal[i].NumComponents;

		cameraComponents.push_back(cameraComponent);
	}

	// Resolve scene object component's children
	for (auto& pair : allSceneObjectComponents)
	{
		
		uint32_t componentNamesStartingIdx = allSceneObjectHeaderComponentNameStartingIdx[pair.first];
		uint32_t componentNamesNumber = allSceneObjectHeaderComponentNameNumbers[pair.first];
		for (uint32_t i = componentNamesStartingIdx; i < componentNamesStartingIdx + componentNamesNumber; i++)
		{
			std::string componentName = stringTable[i];
			if (allSceneObjectComponents.contains(componentName))
			{
				pair.second->AddComponent(allSceneObjectComponents[componentName]);
			}
		}
	}

	// Resolve root components for scene objects, and push them to the scene graph
	std::shared_ptr<SceneGraphImpl> sceneGraph = std::make_shared<SceneGraphImpl>();
	for (size_t i = 0; i < playableCharacters.size(); i++)
	{
		std::string rootComponentName = playableCharacterRootComponentNames[i];
		if (allSceneObjectComponents.contains(rootComponentName))
		{
			playableCharacters[i]->SetRootComponent(allSceneObjectComponents[rootComponentName]);
		}
		sceneGraph->AddPlayableCharacterObject(playableCharacters[i]);
	}

	for (size_t i = 0; i < nonePlayableCharacters.size(); i++)
	{
		std::string rootComponentName = nonePlayableCharacterRootComponentNames[i];
		if (allSceneObjectComponents.contains(rootComponentName))
		{
			nonePlayableCharacters[i]->SetRootComponent(allSceneObjectComponents[rootComponentName]);
		}
		sceneGraph->AddNonePlayableCharacterObject(nonePlayableCharacters[i]);

	}

	// Resource components

	m_tileData = std::make_shared<TileDataImpl>(m_header.TileName, m_header.BboxMinX, m_header.BboxMinY, m_header.BboxMaxX, m_header.BboxMaxY);
	m_tileData->SetSceneGraph(sceneGraph);

	return true;
}

bool ResourceLoaderImpl::LoadMeshesData(std::fstream& file)
{
	m_meshesData = std::make_shared<MeshesDataImpl>();

	m_header.NumMeshDefinitions;
	m_header.MeshDefinitionsOffset;
	
	if (m_header.NumMeshDefinitions == 0)
	{
		return true;
	}

	StaticMeshDefinitionHeader* pMeshDefinitionHeaders = new StaticMeshDefinitionHeader[m_header.NumMeshDefinitions];
	MeshPartHeader* pMeshPartHeaders = new MeshPartHeader[m_header.NumMeshParts];
	MaterialHeader* pMaterialHeaders = new MaterialHeader[m_header.NumMaterials];
	// Read mesh definitions
	file.seekg(m_header.MeshDefinitionsOffset);
	file.read((char*)pMeshDefinitionHeaders, m_header.NumMeshDefinitions * sizeof(StaticMeshDefinitionHeader));
	// Read mesh part headers
	file.seekg(m_header.MeshPartsOffset);
	file.read((char*)pMeshPartHeaders, m_header.NumMeshParts * sizeof(MeshPartHeader));
	// Read material headers
	file.seekg(m_header.MaterialsOffset);
	file.read((char*)pMaterialHeaders, m_header.NumMaterials * sizeof(MaterialHeader));

	std::vector<std::shared_ptr<Material>> materials;
	for (uint32_t i = 0; i < m_header.NumMaterials; i++)
	{
		MaterialHeader& materialHeader = pMaterialHeaders[i];
		std::shared_ptr<MaterialImpl> material = std::make_shared<MaterialImpl>(materialHeader.MaterialName);
		material->SetBaseColorTextureName(materialHeader.BaseColorTextureName);
		material->SetBaseColorTextureFile(materialHeader.BaseColorTextureFile);
		material->SetMetallicTextureName(materialHeader.MetallicTextureName);
		material->SetMetallicTextureFile(materialHeader.MetallicTextureFile);
		material->SetRoughnessTextureName(materialHeader.RoughnessTextureName);
		material->SetRoughnessTextureFile(materialHeader.RoughnessTextureFile);
		material->SetNormalTextureName(materialHeader.NormalTextureName);
		material->SetNormalTextureFile(materialHeader.NormalTextureFile);

		materials.push_back(material);
	}

	for (uint32_t i = 0; i < m_header.NumMeshDefinitions; i++)
	{
		StaticMeshDefinitionHeader& meshDefHeader = pMeshDefinitionHeaders[i];
		// Process each mesh definition as needed
		
		std::shared_ptr<MeshImpl> mesh = std::make_shared<MeshImpl>(meshDefHeader.MeshName);
		for (uint32_t j = meshDefHeader.PartsIdx; j < meshDefHeader.PartsIdx + meshDefHeader.NumParts; j++)
		{
			MeshPartHeader& meshPartHeader = pMeshPartHeaders[j];
			std::shared_ptr<MeshPartImpl> meshPart = std::make_shared<MeshPartImpl>(meshPartHeader.MeshDataOffset, meshPartHeader.MeshDataSize, meshPartHeader.NumVertices);
		
			meshPart->SetMaterial(materials[meshPartHeader.MaterialIdx]);
		}

		m_meshesData->AddMesh(mesh);
	}

	delete[] pMeshDefinitionHeaders;
	delete[] pMeshPartHeaders;
	delete[] pMaterialHeaders;

	return true;
}

bool ResourceLoaderImpl::LoadTexturesData(std::fstream& file)
{
	m_texturesData = std::make_shared<TexturesDataImpl>();

	if (m_header.NumTextures == 0)
	{
		return true;
	}

	// Read texture headers
	TextureHeader* pTextureHeaders = new TextureHeader[m_header.NumTextures];

	// Read mesh definitions
	file.seekg(m_header.TexturesOffset);
	file.read((char*)pTextureHeaders, m_header.NumTextures * sizeof(TextureHeader));

	for (uint32_t i = 0; i < m_header.NumTextures; i++)
	{
		TextureHeader& textureHeader = pTextureHeaders[i];
		std::shared_ptr<TextureImpl> texture = std::make_shared<TextureImpl>(
			textureHeader.TextureName,
			textureHeader.TextureOffset,
			textureHeader.TextureDataSize,
			textureHeader.TextureWidth,
			textureHeader.TextureHeight,
			textureHeader.NumChannels);

		m_texturesData->AddTexture(texture);
	}

	delete[] pTextureHeaders;

	return true;
}