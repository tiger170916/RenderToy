#pragma once

#include "Includes.h"
#include "PassType.h"
#include "Pipeline.h"
#include "GraphicsContext.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include "TextureManager2.h"
#include "World2.h"

class RenderGraph
{
	// Internal structs
private:
	struct PassStruct
	{
		std::string Name;
		GUID Guid;
		PassType Type;
	};

	struct PipelineStruct
	{
		std::string Name;
		GUID Guid;
		std::vector<PassStruct> Passes;
	};

	struct PassLevelStruct
	{
		PassBase* Pass;
		int Level;
	};

private:
	std::filesystem::path m_filePath;

	std::vector<std::unique_ptr<Pipeline>> m_pipelines;

	std::vector<PassLevelStruct> m_flattenedPassGraph;

	std::vector<PassBase*> m_allPasses;

	// Fence indicating that all pipelines are ready to execute commands
	std::unique_ptr<D3DFence> m_startingFence;

	PassBase* m_finalRenderOutputPass = nullptr;

	PassBase* m_finalPass = nullptr;

	HANDLE m_renderOutputWorkDoneEvent = NULL;

	bool m_initialized = false;

public:
	RenderGraph(std::filesystem::path filePath);

	~RenderGraph();

	bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager);

	bool PopulateCommandLists(World* world, GraphicsContext* graphicsContext);

	bool PopulateCommandLists(World2* world, MaterialManager* materialManager, TextureManager2* textureManager, GraphicsContext* graphicsContext);

	bool ExecuteCommands();

	bool UpdateBuffers(World* world);

	ID3D12Resource* GetFinalRenderOutputResource();

	bool TransitFinalRenderOutputResource(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState);

	void WaitForRenderFinalOutputDone();

	bool UpdateConstants(World2* world);

private:
	bool Validate();

	bool ParseFile(std::vector<PipelineStruct>& outPipelines, std::map<GUID, std::set<GUID, GuidComparator>, GuidComparator>& outExtraDependencies, std::string& outFinalRenderOutputPass, std::string& outFinalPass);

	void FlattenRenderGraph(PassBase* randomPass);

	void FlattenRenderGraphRecursive(std::map<PassBase*, int>& levels, PassBase* node, int lastLevel, int level);

	Pipeline* GetParentPipeline(PassBase* pass);
};