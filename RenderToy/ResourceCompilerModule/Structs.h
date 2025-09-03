#pragma once

#include <string>
#include <filesystem>

struct LightExtension
{
	std::string LightType; // Point, Spot, Directional
	float Offset[3];
	float Rotation[3];
	float Color[3];
	float Intensity;
	float AttenuationRadius;
	float AspectRatio;
	float Fov;
};

struct StaticMeshInstanceStruct
{
	float Position[3];
	float Rotation[3];
	float Scale[3];

	std::shared_ptr<LightExtension> LightExt;
};

struct StaticMeshStruct
{
	std::string MeshName;

	std::filesystem::path AssetPath;
	std::filesystem::path OverrideNormalTexture;
	std::filesystem::path OverrideMetallicTexture;
	std::filesystem::path OverrideRoughnessTexture;
	std::filesystem::path OverrideBaseColorTexture;

	std::vector<std::shared_ptr<StaticMeshInstanceStruct>> Instances;
};

struct TileStruct
{
	std::string Name;
	float BboxMin[2];
	float BboxMax[2];

	std::vector<std::shared_ptr<StaticMeshStruct>> StaticMeshes;
};