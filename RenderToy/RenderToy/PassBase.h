#pragma once

#include "Includes.h"
#include "PassDefines.h"
#include "CommandBuilder.h"
#include "D3DFence.h"
#include "World.h"
#include "GraphicsContext.h"
#include "ShaderManager.h"
#include "TextureManager2.h"
#include "GuidComparator.h"
#include "PassType.h"
#include "World2.h"


class PassBase
{
protected:
	GUID m_passGuid;

	PassType m_passType = PassType::NONE;

	bool m_initialized = false;

	std::unique_ptr<CommandBuilder> m_commandBuilder = nullptr;

	std::unique_ptr<D3DFence> m_fence = nullptr;

	// Passes that depend on this pass
	std::vector<PassBase*> m_dependentPasses;

	// Passes that this pass relies on.
	std::vector<PassBase*> m_dependencyPasses;

	// Resource states recorder
	std::map<ID3D12Resource*, D3D12_RESOURCE_STATES> m_resourceStates;

	bool m_skiped = false;

protected:
	bool ResourceBarrierTransition(ID3D12Resource* resource, ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter);

	PassBase* GetDependencyPassOfType(PassType passType);

public:
	virtual ~PassBase() = 0;

	// Interface
	virtual bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager);

	virtual bool PopulateCommands(World* world, GraphicsContext* graphicsContext);

	virtual bool PopulateCommands(World2* world, MaterialManager* materialManager, TextureManager2* textureManager, GraphicsContext* graphicsContext);

	virtual bool UpdateBuffers(World* world);

	virtual bool UpdateBuffers(World2* world);

	// A pass that can be used as final stage of render pass has to override this function, and return the corresponding buffer.
	virtual ID3D12Resource* GetFinalRenderPassOutputResource() const;

	virtual bool TransitFinalRenderPassOutputResource(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES stateAfter);

	// Add a dependency to this pass
	void AddDependency(PassBase* dependencyPass);

public:
	PassBase(GUID passGuid);

	inline const GUID GetPassGuid() const { return m_passGuid; }

	inline const PassType GetPassType() const { return m_passType; }

	inline CommandBuilder* GetCommandBuilder() const { return m_commandBuilder.get(); }

	inline std::vector<PassBase*>& GetDependencies() { return m_dependencyPasses; }

	inline std::vector<PassBase*>& GetDependents() { return m_dependentPasses; }

	inline ID3D12Fence* GetFence() { return m_fence.get()->GetFence(); }

	inline void SignalFence() { m_fence->Signal(); }

	inline const bool IsSkipped() const { return m_skiped; }

	// Signal the fence when complete
	inline void CommandQueueSignal(ID3D12CommandQueue* pCommandQueue) { m_fence->CommandQueueSignal(pCommandQueue); }

	// Signal the fence when complete and set an event
	inline void CommandQueueSignalAndSetEvent(ID3D12CommandQueue* pCommandQueue, HANDLE hEvent) { m_fence->CommandQueueSignalAndSetEvent(pCommandQueue, hEvent); }

	inline uint64_t GetCurrentFenceValue() { return m_fence->GetCurrentFenceValue(); }
};