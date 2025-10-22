#pragma once
#include "Includes.h"
#include "Camera.h"
#include "StaticMesh.h"
#include "GlobalStructs.h"
#include "LightStructs.h"
#include "ConstantBuffer.h"
#include "DescriptorHeapManager.h"
#include "Tile.h"

class World
{
private:
	std::unique_ptr<Camera> m_activeCamera; // TODO: switch between different cameras

	std::vector<std::shared_ptr<StaticMesh>> m_staticMeshes;

	std::unique_ptr<ConstantBuffer<UniformFrameConstants>> m_uniformFrameConstantBuffer = nullptr;

	std::unique_ptr<ConstantBuffer<LightConstantsDx>> m_lightConstants = nullptr;

	std::vector<std::unique_ptr<Tile>> m_tiles;

	bool m_initialized = false;

public:
	World();

	bool Initialize(GraphicsContext* graphicsContext);

	/// <summary>
	/// Initialize a world from binary files (each file represents a tile, which is the smallest unit of scene streaming)
	/// </summary>
	bool InitializeFromBinary(GraphicsContext* graphicsContext, std::filesystem::path binaryDir);

	~World();

	void SpawnStaticMesh(std::shared_ptr<StaticMesh> staticMesh);

	void SpawnStaticMeshes(std::vector<std::shared_ptr<StaticMesh>>& staticMeshes);

	void SetActiveCamera(UINT width, UINT height, FVector3 position, FRotator rotator);

	const std::vector<std::shared_ptr<StaticMesh>>& GetAllStaticMeshes() const { return m_staticMeshes; }

	inline Camera* GetActiveCamera() const { return m_activeCamera.get(); }

	inline ConstantBuffer<UniformFrameConstants>* GetUniformFrameConstantBuffer() const { return m_uniformFrameConstantBuffer.get(); }

	inline ConstantBuffer<LightConstantsDx>* GetLightConstantBuffer() const { return m_lightConstants.get(); }

	bool UpdateBuffers();

	bool FrameBegin(float delta);

	bool GetAdjacentTilesAroundActiveCamera(std::vector<Tile*>& outAdjacentTiles, std::vector<Tile*> outOtherTiles);

private:
	bool LoadTileBinaryFile(std::filesystem::path tileFilePath);
};