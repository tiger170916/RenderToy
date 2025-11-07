#pragma once
#include "Windows.h"
#include <cstdint>
#include <string>
#include <vector>

namespace ResourceCompilerModule
{
	enum class SceneObjectType
	{
		SCENE_OBJECT_PLAYABLE = 1,
		SCENE_OBJECT_NONE_PLAYABLE = 2,
	};

	enum class SceneObjectComponentType
	{
		SCENE_OBJECT_COMPONENT_STATIC_MESH = 1,
		SCENE_OBJECT_COMPONENT_CAMERA_ARM = 2,
		SCENE_OBJECT_COMPONENT_CAMERA = 3,
	};

	class SceneObjectComponent
	{
	public:
		virtual SceneObjectComponentType GetType() = 0;

		virtual std::string GetName() = 0;

		virtual SceneObjectComponent* GetParent() = 0;

		virtual void GetComponents(std::vector<SceneObjectComponent*>& outComponents) = 0;
	};

	class LightExtension
	{
	public:
		virtual void GetOffset(float& outX, float& outY, float& outZ) = 0;

		virtual void GetRotation(float& outPitch, float& outYaw, float& outRoll) = 0;

		virtual void GetColor(float& outR, float& outG, float& outB) = 0;

		virtual float GetIntensity() = 0;

		virtual uint32_t GetLightType() = 0;

		virtual float GetAttenuationRadius() = 0;

		virtual float GetAspectRatio() = 0;

		virtual float GetFov() = 0;
	};

	class StaticMeshInstance
	{
	public:
		virtual void GetPosition(float& outX, float& outY, float& outZ) = 0;

		virtual void GetRotation(float& outPitch, float& outYaw, float& outRoll) = 0;

		virtual void GetScale(float& outX, float& outY, float& outZ) = 0;

		virtual bool HasLightExtension() = 0;

		virtual LightExtension* GetLightExtension() = 0;
	};

	class StaticMeshComponent : public SceneObjectComponent
	{
	public:
		virtual std::string GetFileName() = 0;
		virtual uint32_t GetInstanceCount() = 0;
		virtual void GetInstances(std::vector<StaticMeshInstance*>& outInstances) = 0;
	};

	class CameraArmComponent : public SceneObjectComponent
	{
	public:
		virtual float GetArmLength() = 0;
	};

	class CameraComponent : public SceneObjectComponent
	{
	public:
		// No specific interface for now
	};

	class SceneObject
	{
	public:
		virtual SceneObjectType GetType() = 0;

		virtual std::string GetName() = 0;

		virtual SceneObjectComponent* GetRootComponent() = 0;
	};

	class PlayableCharacterObject : public  SceneObject
	{
	public:
		// No specific interface for now
	};

	class NonePlayableCharacterObject : public SceneObject
	{
	public:
		// No specific interface for now
	};

	class SceneGraph
	{
	public:
		/// <summary>
		/// Get a list of all playable character objects. It's up to the caller to release output array
		/// </summary>
		virtual bool GetPlayableCharacterObjects(std::vector<PlayableCharacterObject*>& outPlayableCharacters) = 0;

		/// <summary>
		/// Get a list of all non playable character objects. It's up the the caller to release output array.
		/// </summary>
		virtual bool GetNonePlayableCharacterObjects(std::vector<NonePlayableCharacterObject*>& outNonePlayableCharacters) = 0;
	};

	class TileData
	{
	public:
		virtual std::string GetTileName() = 0;

		virtual void GetBoundingBox(float& outMinX, float& outMinY, float& outMaxX, float& outMaxY) = 0;

		virtual SceneGraph* GetSceneGraph() = 0;
	};
}