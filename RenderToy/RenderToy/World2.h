#pragma once

#include "Includes.h"
#include "Tile.h"
#include "Camera.h"
#include "GraphicsContext.h"
#include "TextureManager2.h"
#include "MaterialManager.h"
#include "ConstantBuffer.h"
#include "GlobalStructs.h"
#include "LightStructs.h"
#include "GraphicsContext.h"
#include "PlayableCharacterObject.h"

class World2
{
private:
	std::vector<std::unique_ptr<Tile>> m_tiles;

	int m_activeCameraIdx = -1;

	std::unique_ptr<ConstantBuffer<UniformFrameConstants>> m_uniformFrameConstantBuffer = nullptr;

	std::unique_ptr<ConstantBuffer<LightConstantsDx>> m_lightConstantBuffer = nullptr;

	MaterialManager* m_materialManager = nullptr;

	TextureManager2* m_textureManager = nullptr;

	GraphicsContext* m_graphicsContext = nullptr;

	std::vector<std::unique_ptr<PlayableCharacterObject>> m_standaloneCameras;

	bool m_cameraListDirty = true;

	// all cameras in the world
	std::vector<Camera*> m_allCameras;

	bool m_playableObjectsDirty = true;

	// all playable objects in the world.
	std::vector<PlayableCharacterObject*> m_playableObjects;

public:
	World2(GraphicsContext* graphicsContext, MaterialManager* materialManager, TextureManager2* texManager);

	bool Initialize(GraphicsContext* graphicsContext);

	~World2();
	
	bool LoadFromBinary(std::filesystem::path rootDirectory);

	void CreateStandaloneCamera(UINT width, UINT height, FVector3 position, FRotator rotator);

public:
	/// <summary>
	/// Stream in the tiles around the position of the active camera
	/// </summary>
	bool StreamInAroundActiveCameraRange(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder, float radius);

	inline ConstantBuffer<UniformFrameConstants>* GetUniformFrameConstantBuffer() { return m_uniformFrameConstantBuffer.get(); }

	inline ConstantBuffer<LightConstantsDx>* GetLightConstantBuffer() { return m_lightConstantBuffer.get(); }

	bool UpdateBuffersForFrame();

	Camera* GetActiveCamera();

	void SwitchCamera();

	void GetActiveTiles(std::vector<Tile*>& outTiles);

private:
	// Iterate all global standalone cameras and component cameras and cache all available camerasd
	void GetAllCamerasInternal();
};