#pragma once

#include <string>
#include <filesystem>


struct LightExtensionInternal
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

struct StaticMeshInstanceInternal
{
	float Position[3];
	float Rotation[3];
	float Scale[3];

	std::shared_ptr<LightExtensionInternal> LightExt;
};

struct NonePlayableCharacterInternal
{
	std::string Name;
	std::string RootComponent;
};

struct PlayableCharacterInternal
{
	std::string Name;
	std::string RootComponent;
};

struct SceneObjectComponentInternal
{
	std::string Name;
	std::string Parent;
	std::vector<std::string> Components;
};

struct CameraArmComponentInternal : public SceneObjectComponentInternal
{
	float Length;
};

struct CameraComponentInternal : public SceneObjectComponentInternal
{
};

struct StaticMeshComponentInternal : public SceneObjectComponentInternal
{
	std::string File;

	std::vector<std::shared_ptr<StaticMeshInstanceInternal>> Instances;
};

struct TileStruct
{
	std::string Name;
	float BboxMin[2];
	float BboxMax[2];

	std::vector<std::shared_ptr<NonePlayableCharacterInternal>> NonePlayableObjects;
	std::vector<std::shared_ptr<PlayableCharacterInternal>> PlayableObjects;
	std::vector<std::shared_ptr<StaticMeshComponentInternal>> StaticMeshComponents;
	std::vector<std::shared_ptr<CameraArmComponentInternal>> CameraArmComponents;
	std::vector<std::shared_ptr<CameraComponentInternal>> CameraComponents;
};

// Mesh
struct StaticMeshDefineInternal
{
	std::string Name;
	std::string MeshAssetPath;

	bool HasOverrideMaterial;
	std::string OverrideMaterialName;
};

// Texture
struct TextureDefineInternal
{
	std::string Name;
	std::string File;
};