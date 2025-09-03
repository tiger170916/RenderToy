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
		uint32_t TileNameStrIndex;
		uint32_t SizeNameStr;
		float BboxMinX;
		float BboxMinY;
		float BboxMaxX;
		float BboxMaxY;

		uint32_t NumStaticMeshes;
		uint32_t NumStaticMeshInstances;
		uint32_t NumLightExtensions;

		uint32_t MeshHeadersOffset;
		uint32_t StaticMeshInstancesOffset;
		uint32_t LightExtensionsOffset;
		uint32_t StringsOffset;

		uint32_t StringTotalLength;
	};

	struct StaticMeshHeader
	{
		int32_t MeshNameIndex;
		int32_t MeshNameSize;
		int32_t MeshDataOffset;
		int32_t MeshDataSize;
		int32_t InstanceCount;
		int32_t InstanceIndex;
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

	struct MeshVertexDx
	{
		float position[3];
		float UV0[2];
		float Normal[3];
	};
}