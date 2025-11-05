#pragma once

#include "ResourceCompilerTile.h"
#include <memory>
#include <string>
#include <vector>

using namespace ResourceCompilerModule;

/// <summary>
///  Scene Graph Impl
/// </summary>
class SceneGraphImpl : public SceneGraph
{
private:
	std::vector<std::shared_ptr<PlayableCharacterObject>> m_playableSceneObjects;
	std::vector<std::shared_ptr<NonePlayableCharacterObject>> m_nonePlayableSceneObjects;

public:
	virtual bool GetPlayableCharacterObjects(std::vector<PlayableCharacterObject*>& outPlayableCharacters) override;

	virtual bool GetNonePlayableCharacterObjects(std::vector<NonePlayableCharacterObject*>& outNonePlayableCharacters) override;

	void AddPlayableCharacterObject(std::shared_ptr<PlayableCharacterObject> newObj);

	void AddNonePlayableCharacterObject(std::shared_ptr<NonePlayableCharacterObject> newObj);
};

/// <summary>
/// Scene Object Impl
/// </summary>
class SceneObjectImpl : public SceneObject
{
private:
	SceneObjectType m_type;

	std::string m_name;

	std::shared_ptr<SceneObjectComponent> m_rootComponent;

public:
	// Scene Object Interfaces
	inline virtual SceneObjectType GetType() override { return m_type; }

	inline virtual std::string GetName() override { return m_name; }

	inline virtual SceneObjectComponent* GetRootComponent() override { return m_rootComponent.get(); }

protected:
	SceneObjectImpl(SceneObjectType type, std::string name);

public:
	virtual ~SceneObjectImpl();

	inline void SetRootComponent(std::shared_ptr<SceneObjectComponent> rootComp) { m_rootComponent = rootComp; }
};

/// <summary>
/// Playable Character Object Impl
/// </summary>
class PlayableCharacterObjectImpl : public  PlayableCharacterObject, public SceneObjectImpl
{

public:
	PlayableCharacterObjectImpl(std::string name);

	// Scene Object Interfaces
	inline virtual SceneObjectType GetType() override { return SceneObjectImpl::GetType(); }

	inline virtual std::string GetName() override { return SceneObjectImpl::GetName(); }

	inline virtual SceneObjectComponent* GetRootComponent() override { return SceneObjectImpl::GetRootComponent(); }
};

/// <summary>
/// None Playable Character Object Impl
/// </summary>
class NonePlayableCharacterObjectImpl : public NonePlayableCharacterObject, public SceneObjectImpl
{
public:
	NonePlayableCharacterObjectImpl(std::string name);

	// Scene Object Interfaces
	inline virtual SceneObjectType GetType() override { return SceneObjectImpl::GetType(); }

	inline virtual std::string GetName() override { return SceneObjectImpl::GetName(); }

	inline virtual SceneObjectComponent* GetRootComponent() override { return SceneObjectImpl::GetRootComponent(); }
};

/// <summary>
/// Scene Object Component Impl
/// </summary>
class SceneObjectComponentImpl : public SceneObjectComponent
{
private:
	SceneObjectComponentType m_type;
	std::string m_name;
	std::shared_ptr<SceneObjectComponent> m_parent;
	std::vector<std::shared_ptr<SceneObjectComponent>> m_components;

public:
	inline virtual SceneObjectComponentType GetType() { return m_type; }

	inline virtual std::string GetName() { return m_name; }

	inline virtual SceneObjectComponent* GetParent() { return m_parent.get(); }

	virtual void GetComponents(std::vector<SceneObjectComponent*>& outComponents);

	void AddComponent(std::shared_ptr<SceneObjectComponent> comp) { m_components.push_back(comp); }

	void SetParent(std::shared_ptr<SceneObjectComponent> parent) { m_parent = parent; }

protected:
	SceneObjectComponentImpl(SceneObjectComponentType type, std::string name);

public:
	virtual ~SceneObjectComponentImpl();
};

/// <summary>
/// Static Mesh Component Impl
/// </summary>
class StaticMeshComponentImpl : public StaticMeshComponent, public SceneObjectComponentImpl
{
private:
	std::string m_fileName;

	std::vector<std::shared_ptr<StaticMeshInstance>> m_instances;

public:
	inline virtual std::string GetFileName() { return m_fileName; }
	virtual uint32_t GetInstanceCount() { return (uint32_t)m_instances.size(); }
	virtual void GetInstances(std::vector<StaticMeshInstance*>& outInstances);

	StaticMeshComponentImpl(std::string name, std::string fileName);

	inline void AddInstance(std::shared_ptr<StaticMeshInstance> instance) { m_instances.push_back(instance); }
};

/// <summary>
/// Camera Arm Component Impl
/// </summary>
class CameraArmComponentImpl : public CameraArmComponent, public SceneObjectComponentImpl
{
private:
	float m_armLength;
public:
	virtual float GetArmLength() override { return m_armLength; }

	CameraArmComponentImpl(std::string name, float armLen);
};

/// <summary>
/// Camera Component Impl
/// </summary>
class CameraComponentImpl : public CameraComponent, public SceneObjectComponentImpl
{
public:
	CameraComponentImpl(std::string name);
};

/// <summary>
/// Light Extension Impl
/// </summary>
class LightExtensionImpl : public LightExtension
{
private:
	uint32_t m_type = 0;

	float m_offsetX = 0.0f;
	float m_offsetY = 0.0f;
	float m_offsetZ = 0.0f;

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	float m_roll = 0.0f;

	float m_red = 0.0f;
	float m_green = 0.0f;
	float m_blue = 0.0f;

	float m_intensity = 0.0f;
	float m_attenuationRadius = 0.0f;
	float m_aspectRatio = 0.0f;
	float m_fov = 0.0f;

public:	
	virtual void GetOffset(float& outX, float& outY, float& outZ) override;

	virtual void GetRotation(float& outPitch, float& outYaw, float& outRoll) override;

	virtual void GetColor(float& outR, float& outG, float& outB) override;

	inline virtual float GetIntensity() override { return m_intensity; }

	inline virtual uint32_t GetLightType() override { return m_type; }

	inline virtual float GetAttenuationRadius() override { return m_attenuationRadius; }

	inline virtual float GetAspectRatio() override { return m_aspectRatio; }

	inline virtual float GetFov() override { return m_fov; }

	LightExtensionImpl(
		const uint32_t& type,
		const float& offsetX, const float& offsetY, const float& offsetZ,
		const float& pitch, const float& yaw, const float& roll,
		const float& r, const float& g, const float& b,
		const float& intensity,
		const float& attenuationRadius,
		const float& aspectRatio,
		const float& fov);
};

/// <summary>
/// Static Mesh Instance Impl
/// </summary>
class StaticMeshInstanceImpl : public StaticMeshInstance
{
private:
	float m_posX = 0.0f;
	float m_posY = 0.0f;
	float m_posZ = 0.0f;

	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	float m_roll = 0.0f;

	float m_scaleX = 0.0f;
	float m_scaleY = 0.0f;
	float m_scaleZ = 0.0f;

	std::shared_ptr<LightExtension> m_lightExt = nullptr;

public:
	virtual void GetPosition(float& outX, float& outY, float& outZ) override;

	virtual void GetRotation(float& outPitch, float& outYaw, float& outRoll) override;

	virtual void GetScale(float& outX, float& outY, float& outZ) override;

	inline virtual bool HasLightExtension() override { return m_lightExt != nullptr; }

	inline virtual LightExtension* GetLightExtension() override { return m_lightExt.get(); }

	StaticMeshInstanceImpl(
		const float& posX, const float& posY, const float& posZ,
		const float& pitch, const float& yaw, const float& roll,
		const float& scaleX, const float& scaleY, const float& scaleZ);

	inline void AttachLightExtension(std::shared_ptr<LightExtension> lightExt) { m_lightExt = lightExt; }
};

class TileDataImpl : public TileData
{
private:
	/// <summary>
	/// Bounding Box
	/// </summary>
	float m_bboxMinX;
	float m_bboxMinY;
	float m_bboxMaxX;
	float m_bboxMaxY;

	/// <summary>
	/// Name
	/// </summary>
	std::string m_tileName;

	/// <summary>
	/// Scene graph of this tile
	/// </summary>
	std::shared_ptr<SceneGraph> m_sceneGraph;

public:
	inline virtual std::string GetTileName() override { return m_tileName; }

	virtual void GetBoundingBox(float& outMinX, float& outMinY, float& outMaxX, float& outMaxY) override;

	inline virtual SceneGraph* GetSceneGraph() override { return m_sceneGraph.get(); }

	TileDataImpl(std::string name, const float& minX, const float& minY, const float& maxX, const float& maxY);

	inline void SetSceneGraph(std::shared_ptr<SceneGraph> sceneGraph) { m_sceneGraph = sceneGraph; }
};