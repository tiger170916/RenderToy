#include "ResourceCompilerTileImpl.h"

// Scene Graph Impl
bool SceneGraphImpl::GetPlayableCharacterObjects(std::vector<PlayableCharacterObject*>& outPlayableCharacters)
{
	outPlayableCharacters.clear();

	if (m_playableSceneObjects.empty())
	{
		return true;
	}

	for (size_t i = 0; i < m_playableSceneObjects.size(); i++)
	{
		outPlayableCharacters.push_back(m_playableSceneObjects[i].get());
	}

	return true;
}

bool SceneGraphImpl::GetNonePlayableCharacterObjects(std::vector<NonePlayableCharacterObject*>& outNonePlayableCharacters)
{
	outNonePlayableCharacters.clear();

	if (m_playableSceneObjects.empty())
	{
		return true;
	}

	for (size_t i = 0; i < m_nonePlayableSceneObjects.size(); i++)
	{
		outNonePlayableCharacters.push_back(m_nonePlayableSceneObjects[i].get());
	}

	return true;
}

void SceneGraphImpl::AddPlayableCharacterObject(std::shared_ptr<PlayableCharacterObject> newObj)
{
	if (!newObj)
	{
		return;
	}

	m_playableSceneObjects.push_back(newObj);
}

void SceneGraphImpl::AddNonePlayableCharacterObject(std::shared_ptr<NonePlayableCharacterObject> newObj)
{
	if (!newObj)
	{
		return;
	}

	m_nonePlayableSceneObjects.push_back(newObj);
}

// Scene Object Impl
SceneObjectImpl::SceneObjectImpl(SceneObjectType type, std::string name)
	: m_type(type), m_name(name)
{
}

SceneObjectImpl::~SceneObjectImpl()
{
}

// Playable Character Object Impl
PlayableCharacterObjectImpl::PlayableCharacterObjectImpl(std::string name)
	: SceneObjectImpl(SceneObjectType::SCENE_OBJECT_PLAYABLE, name)
{
}

// None playable character object impl
NonePlayableCharacterObjectImpl::NonePlayableCharacterObjectImpl(std::string name)
	: SceneObjectImpl(SceneObjectType::SCENE_OBJECT_NONE_PLAYABLE, name)
{

}

// Scene Object Component impl
void SceneObjectComponentImpl::GetComponents(std::vector<SceneObjectComponent*>& outComponents)
{
	outComponents.clear();
	for (size_t i = 0; i < m_components.size(); i++)
	{
		outComponents.push_back(m_components[i].get());
	}
}

SceneObjectComponentImpl::SceneObjectComponentImpl(SceneObjectComponentType type, std::string name)
	: m_type(type), m_name(name)
{
}

SceneObjectComponentImpl::~SceneObjectComponentImpl() {}

// Static Mesh Component impl
StaticMeshComponentImpl::StaticMeshComponentImpl(std::string name, std::string fileName)
	: SceneObjectComponentImpl(SceneObjectComponentType::SCENE_OBJECT_COMPONENT_STATIC_MESH, name), m_fileName(fileName)
{
}

void StaticMeshComponentImpl::GetInstances(std::vector<StaticMeshInstance*>& outInstances)
{
	outInstances.clear();
	for (size_t i = 0; i < m_instances.size(); i++)
	{
		outInstances.push_back(m_instances[i].get());
	}
}

// Camera Arm Component Impl
CameraArmComponentImpl::CameraArmComponentImpl(std::string name, float armLen)
	: SceneObjectComponentImpl(SceneObjectComponentType::SCENE_OBJECT_COMPONENT_CAMERA_ARM, name), m_armLength(armLen)
{
}

// Camera Component Impl
CameraComponentImpl::CameraComponentImpl(std::string name)
	: SceneObjectComponentImpl(SceneObjectComponentType::SCENE_OBJECT_COMPONENT_CAMERA, name)
{
}

// Light Extension Impl
LightExtensionImpl::LightExtensionImpl(
	const uint32_t& type,
	const float& offsetX, const float& offsetY, const float& offsetZ,
	const float& pitch, const float& yaw, const float& roll,
	const float& r, const float& g, const float& b,
	const float& intensity,
	const float& attenuationRadius,
	const float& aspectRatio,
	const float& fov)
	:	m_type(type),
		m_offsetX(offsetX), m_offsetY(offsetY), m_offsetZ(offsetZ),
		m_pitch(pitch), m_yaw(yaw), m_roll(roll),
		m_red(r), m_green(g), m_blue(b),
		m_intensity(intensity),
		m_attenuationRadius(attenuationRadius),
		m_aspectRatio(aspectRatio),
		m_fov(fov)
{
}

void LightExtensionImpl::GetOffset(float& outX, float& outY, float& outZ)
{
	outX = m_offsetX;
	outY = m_offsetY;
	outZ = m_offsetZ;
}

void LightExtensionImpl::GetRotation(float& outPitch, float& outYaw, float& outRoll)
{
	outPitch = m_pitch;
	outYaw = m_yaw;
	outRoll = m_roll;
}

void LightExtensionImpl::GetColor(float& outR, float& outG, float& outB)
{
	outR = m_red;
	outG = m_green;
	outB = m_blue;
}

// Static Mesh Instance Impl
void StaticMeshInstanceImpl::GetPosition(float& outX, float& outY, float& outZ)
{
	outX = m_posX;
	outY = m_posY;
	outZ = m_posZ;
}

void StaticMeshInstanceImpl::GetRotation(float& outPitch, float& outYaw, float& outRoll)
{
	outPitch = m_pitch;
	outYaw = m_yaw;
	outRoll = m_roll;
}

void StaticMeshInstanceImpl::GetScale(float& outX, float& outY, float& outZ)
{
	outX = m_scaleX;
	outY = m_scaleY;
	outZ = m_scaleZ;
}

StaticMeshInstanceImpl::StaticMeshInstanceImpl(
	const float& posX, const float& posY, const float& posZ,
	const float& pitch, const float& yaw, const float& roll,
	const float& scaleX, const float& scaleY, const float& scaleZ)
	: m_posX(posX), m_posY(posY), m_posZ(posZ),
	m_pitch(pitch), m_yaw(yaw), m_roll(roll),
	m_scaleX(scaleX), m_scaleY(scaleY), m_scaleZ(scaleZ)
{
}



// Tile Data Impl
TileDataImpl::TileDataImpl(std::string name, const float& minX, const float& minY, const float& maxX, const float& maxY)
	: m_tileName(name), m_bboxMinX(minX), m_bboxMinY(minY), m_bboxMaxX(maxX), m_bboxMaxY(maxY)
{
}

void TileDataImpl::GetBoundingBox(float& outMinX, float& outMinY, float& outMaxX, float& outMaxY)
{
	outMinX = m_bboxMinX;
	outMinY = m_bboxMinY;
	outMaxX = m_bboxMaxX;
	outMaxY = m_bboxMaxY;
}