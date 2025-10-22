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

class World2
{
private:
	std::vector<std::unique_ptr<Tile>> m_tiles;

	std::unique_ptr<Camera> m_activeCamera; // TODO: switch between different cameras

	std::unique_ptr<ConstantBuffer<UniformFrameConstants>> m_uniformFrameConstantBuffer = nullptr;

	std::unique_ptr<ConstantBuffer<LightConstantsDx>> m_lightConstantBuffer = nullptr;

	MaterialManager* m_materialManager = nullptr;

	TextureManager2* m_textureManager = nullptr;

public:
	World2(MaterialManager* materialManager, TextureManager2* texManager);

	bool Initialize(GraphicsContext* graphicsContext);

	~World2();
	
	bool LoadFromBinary(std::filesystem::path rootDirectory);

	void CreateCamera(UINT width, UINT height, FVector3 position, FRotator rotator);

public:
	/// <summary>
	/// Stream in the tiles around the position of the active camera
	/// </summary>
	bool StreamInAroundActiveCameraRange(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder, float radius);

	inline ConstantBuffer<UniformFrameConstants>* GetUniformFrameConstantBuffer() { return m_uniformFrameConstantBuffer.get(); }

	inline ConstantBuffer<LightConstantsDx>* GetLightConstantBuffer() { return m_lightConstantBuffer.get(); }

	void GetActiveStaticMeshes(std::vector<StaticMesh2*>& outMeshes);

	bool UpdateBuffersForFrame();


};