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
	std::string File;
	std::string Name;

	std::vector<std::shared_ptr<StaticMeshInstanceStruct>> Instances;
};

struct TileStruct
{
	std::string Name;
	float BboxMin[2];
	float BboxMax[2];

	std::vector<std::shared_ptr<StaticMeshStruct>> StaticMeshes;
};

// Meshes
struct StaticMeshDefine
{
	std::string Name;
	std::string MeshAssetPath;

	bool HasOverrideMaterial;
	std::string OverrideMaterialName;
};

struct TextureDefine
{
	std::string Name;
	std::string File;
};