#pragma once
#include <cstdint>

namespace ResourceCompilerModule
{
	enum class LightType
	{
		NONE = 0,
		SPOT_LIGHT = 1,
		POINT_LIGHT = 2,
		AREA_LIGHT = 3,
		DIRECTIONAL_LIGHT = 4,
	};

	enum class BinaryType : uint32_t
	{
		BINARY_TYPE_TILE     = 1,
		BINARY_TYPE_MESHES   = 2,
		BINARY_TYPE_TEXTURES = 3
	};

	struct BinaryHeader
	{
		std::uint32_t MagicNum;
		std::uint32_t Version;

		BinaryType BinaryType;
		char TileName[512];

		float BboxMinX;
		float BboxMinY;
		float BboxMaxX;
		float BboxMaxY;

		uint32_t NumStaticMeshes;
		uint32_t NumStaticMeshInstances;
		uint32_t NumLightExtensions;

		uint32_t NumMeshDefinitions;
		uint32_t NumMeshParts;
		uint32_t NumMaterials;
		uint32_t NumTextures;

		uint32_t MeshHeadersOffset;
		uint32_t StaticMeshInstancesOffset;
		uint32_t LightExtensionsOffset;

		uint32_t MeshDefinitionsOffset;
		uint32_t MeshPartsOffset;
		uint32_t MaterialsOffset;
		uint32_t TexturesOffset;
	};

	struct StaticMeshHeader
	{
		char FileName[512];
		char MeshName[512];
		uint32_t InstanceCount;
		uint32_t InstanceIndex;
	};

	struct StaticMeshDefinitionHeader
	{
		char MeshName[512];
		
		uint32_t NumParts;
		uint32_t PartsIdx;
	};

	struct StaticMeshInstanceHeader
	{
		float Position[3];
		float Rotation[3];
		float Scale[3];
		bool HasLightExtension;
		uint32_t LightExtensionIndex;
	};

	struct LightExtensionHeader
	{
		uint32_t LightType; // Point, Spot, Directional
		float Offset[3];
		float Rotation[3];
		float Color[3];
		float Intensity;
		float AttenuationRadius;
		float AspectRatio;
		float Fov;
	};

	struct MeshPartHeader
	{
		uint32_t MeshDataOffset;
		uint32_t MeshDataSize;
		uint32_t NumVertices;

		uint32_t MaterialIdx;
	};

	struct MaterialHeader
	{
		char MaterialName[512];

		char BaseColorTextureName[512];
		char BaseColorTextureFile[512];

		char MetallicTextureName[512];
		char MetallicTextureFile[512];

		char RoughnessTextureName[512];
		char RoughnessTextureFile[512];

		char NormalTextureName[512];
		char NormalTextureFile[512];
	};

	struct TextureHeader
	{
		char TextureName[512];
		uint32_t TextureOffset;
		uint32_t TextureWidth;
		uint32_t TextureHeight;
		uint32_t TextureDataSize;
		uint32_t NumChannels;

	};

	struct MeshVertexDx
	{
		float position[3];
		float UV0[2];
		float Normal[3];
	};
}