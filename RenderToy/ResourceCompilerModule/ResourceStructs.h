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

	struct TileHeader
	{
		std::uint32_t MagicNum;
		uint32_t Version;
		char TileName[512];

		float BboxMinX;
		float BboxMinY;
		float BboxMaxX;
		float BboxMaxY;

		uint32_t NumStaticMeshes;
		uint32_t NumStaticMeshInstances;
		uint32_t NumLightExtensions;
		uint32_t NumMeshParts;

		uint32_t MeshHeadersOffset;
		uint32_t StaticMeshInstancesOffset;
		uint32_t LightExtensionsOffset;
		uint32_t MeshPartsOffset;
	};

	struct StaticMeshHeader
	{
		char MeshName[512];
		uint32_t InstanceCount;
		uint32_t InstanceIndex;
		uint32_t MeshPartCount;
		uint32_t MeshPartIndex;
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

		char BaseColorTextureName[512];
		uint32_t BaseColorTextureOffset;
		uint32_t BaseColorTextureWidth;
		uint32_t BaseColorTextureHeight;
		uint32_t BaseColorTextureDataSize;
		uint32_t BaseColorNumChannels;

		char MetallicTextureName[512];
		uint32_t MetallicTextureOffset;
		uint32_t MetallicTextureWidth;
		uint32_t MetallicTextureHeight;
		uint32_t MetallicTextureDataSize;
		uint32_t MetallicNumChannels;
		
		char RoughnessTextureName[512];
		uint32_t RoughnessTextureOffset;
		uint32_t RoughnessTextureWidth;
		uint32_t RoughnessTextureHeight;
		uint32_t RoughnessTextureDataSize;
		uint32_t RoughnessNumChannels;

		char NormalTextureName[512];
		uint32_t NormalTextureOffset;
		uint32_t NormalTextureWidth;
		uint32_t NormalTextureHeight;
		uint32_t NormalTextureDataSize;
		uint32_t NormalNumChannels;
	};

	struct MeshVertexDx
	{
		float position[3];
		float UV0[2];
		float Normal[3];
	};
}