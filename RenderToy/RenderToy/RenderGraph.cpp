#include "RenderGraph.h"
#include "PassFactory.h"
#include "Utils.h"
#include "3rdParty/json.hpp"
#include "GuidComparator.h"
#include <iostream>
#include <cstring> 

using json = nlohmann::json;

RenderGraph::RenderGraph(std::filesystem::path filePath)
	: m_filePath(filePath)
{

}

RenderGraph::~RenderGraph()
{

}

bool RenderGraph::Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager)
{
	if (!graphicsContext || !shaderManager)
	{
		return false;
	}

	// Read json file
	std::vector<RenderGraph::PipelineStruct> pipelines;
	std::map<GUID, std::set<GUID, GuidComparator>, GuidComparator> extraDependencies;
	std::string finalRenderOutputPass;
	std::string finalPass;
	if (!ParseFile(pipelines, extraDependencies, finalRenderOutputPass, finalPass))
	{
		return false;
	}

	std::map<GUID, PassBase*, GuidComparator> passes;

	PassBase* prevPass = nullptr;
	PassBase* firstPass = nullptr;
	// Build pipelines and fill in pass dependencies
	for (int i = 0; i < pipelines.size(); i++)
	{
		PipelineStruct& pipelineStruct = pipelines[i];
		Pipeline* pipeline = new Pipeline(pipelineStruct.Name, pipelineStruct.Guid);
		m_pipelines.push_back(std::unique_ptr<Pipeline>(pipeline));

		for (int j = 0; j < pipelineStruct.Passes.size(); j++)
		{
			PassStruct& passStruct = pipelineStruct.Passes[j];
			// Add passes to pipeline
			PassBase* pPass;
			if (!PassFactory::CreatePass(passStruct.Type, passStruct.Guid, &pPass))
			{
				return false;
			}

			if (!firstPass)
			{
				firstPass = pPass;
			}

			pipeline->AddPass(pPass);

			if (prevPass != nullptr)
			{
				// Add dependeny of the previous node of the pipeline
				pipeline->AddPassDependency(pPass, prevPass);
			}

			prevPass = pPass;
			m_allPasses.push_back(pPass);

			if (finalRenderOutputPass.compare(passStruct.Name) == 0)
			{
				m_finalRenderOutputPass = pPass;
			}

			if (finalPass.compare(passStruct.Name) == 0)
			{
				m_finalPass = pPass;
			}

			passes[passStruct.Guid] = pPass;
		}
	}

	for (auto& pair : extraDependencies)
	{
		if (!passes.contains(pair.first))
		{
			continue;
		}

		PassBase* pass = passes[pair.first];
		if (!pass)
		{
			continue;
		}

		for (auto& guid : pair.second)
		{
			if (!passes.contains(guid))
			{
				continue;
			}

			PassBase* dependencyPass = passes[guid];
			if (!dependencyPass)
			{
				continue;
			}

			pass->AddDependency(dependencyPass);
		}
	}

	if ((!m_finalRenderOutputPass && !finalRenderOutputPass.empty())
		|| !m_finalPass)
	{
		return false;
	}

	FlattenRenderGraph(firstPass);

	for (int i = 0; i < m_pipelines.size(); i++)
	{
		if (!m_pipelines[i]->Initialize(graphicsContext))
		{
			return false;
		}

		std::vector<PassBase*> passes = m_pipelines[i]->GetPasses();
		for (int j = 0; j < passes.size(); j++)
		{
			if (!passes[j]->Initialize(graphicsContext, shaderManager))
			{
				return false;
			}
		}
	}

	if (!Validate())
	{
		return false;
	}

	m_startingFence = std::unique_ptr<D3DFence>(new D3DFence());
	if (!m_startingFence->Initialize(graphicsContext->GetDevice()))
	{
		return false;
	}

	m_renderOutputWorkDoneEvent= CreateEventA(NULL, false, false, NULL);

	if (!m_renderOutputWorkDoneEvent)
	{
		return false;
	}

	m_initialized = true;

	return true;
}

void RenderGraph::FlattenRenderGraph(PassBase* randomPass)
{
	if (randomPass == nullptr)
	{
		return;
	}

	std::map<PassBase*, int> levels;
	FlattenRenderGraphRecursive(levels, randomPass, INT_MIN, 0);

	int maxLevel = INT_MIN;
	int minLevel = INT_MAX;
	for (auto& pair : levels)
	{
		maxLevel = max(maxLevel, pair.second);
		minLevel = min(minLevel, pair.second);
	}

	for (int level = maxLevel; level >= minLevel; level--)
	{
		for (auto& pair : levels)
		{
			if (pair.second == level)
			{
				PassLevelStruct passLevelStruct = { pair.first, level };
				m_flattenedPassGraph.push_back(passLevelStruct);
			}
		}
	}
}

bool RenderGraph::PopulateCommandLists(World2* world, MaterialManager* materialManager, TextureManager2* textureManager, GraphicsContext* graphicsContext)
{
	if (!world || !graphicsContext)
	{
		return false;
	}

	for (int i = 0; i < m_flattenedPassGraph.size(); i++)
	{
		if (!m_flattenedPassGraph[i].Pass->PopulateCommands(world, materialManager, textureManager, graphicsContext))
		{
			return false;
		}
	}

	return true;
}

bool RenderGraph::PopulateCommandLists(World* world, GraphicsContext* graphicsContext)
{
	if (!world || !graphicsContext)
	{
		return false;
	}

	for (int i = 0; i < m_flattenedPassGraph.size(); i++)
	{
		if (!m_flattenedPassGraph[i].Pass->PopulateCommands(world, graphicsContext))
		{
			return false;
		}
	}

	return true;
}

ID3D12Resource* RenderGraph::GetFinalRenderOutputResource()
{
	if (m_finalRenderOutputPass)
	{
		return m_finalRenderOutputPass->GetFinalRenderPassOutputResource();
	}

	return nullptr;
}

bool RenderGraph::TransitFinalRenderOutputResource(ID3D12GraphicsCommandList* commandList, D3D12_RESOURCE_STATES afterState)
{
	if (m_finalRenderOutputPass)
	{
		return m_finalRenderOutputPass->TransitFinalRenderPassOutputResource(commandList, afterState);
	}

	return false;
}

Pipeline* RenderGraph::GetParentPipeline(PassBase* pass)
{
	for (auto& pipeline : m_pipelines)
	{
		if (pipeline->FindPass(pass))
		{
			return pipeline.get();
		}
	}

	return nullptr;
}

bool RenderGraphUpdateBuffers(World* world)
{
	if (!world)
	{
		return false;
	}
	
	return true;
}

bool RenderGraph::ExecuteCommands()
{
	uint64_t waitForStaringSignal = m_startingFence->GetCurrentFenceValue() + 1;
	for (auto& pipeline : m_pipelines)
	{
		CommandQueue* commandQueue = pipeline->GetCommandQueue();
		commandQueue->WaitForFence(m_startingFence->GetFence(), waitForStaringSignal);
	}

	for (int i = 0; i < m_flattenedPassGraph.size(); i++)
	{
		PassBase* pass = m_flattenedPassGraph[i].Pass;

		if (pass->IsSkipped())
		{
			continue;
		}

		Pipeline* parentPipeline = GetParentPipeline(pass);
		
		CommandQueue* commandQueue = parentPipeline->GetCommandQueue();
		CommandBuilder* commandBuilder = pass->GetCommandBuilder();

		// Wait for its dependencies to finish
		std::vector<PassBase*>& dependencies = pass->GetDependencies();
		for (int j = 0; j < dependencies.size(); j++)
		{
			PassBase* dependencyPass = dependencies[j];
			commandQueue->WaitForFence(dependencyPass->GetFence(), dependencyPass->GetCurrentFenceValue());
		}

		// Execute commands 
		commandQueue->DispatchCommands(commandBuilder);

		// Signal
		if (pass == m_finalPass)
		{
			// Set the render work done event when the current pass is the final render stage.
			pass->CommandQueueSignalAndSetEvent(commandQueue->GetCommandQueue(), m_renderOutputWorkDoneEvent);
		}
		else
		{
			pass->CommandQueueSignal(commandQueue->GetCommandQueue());
		}
	}

	// Can signal to start processing
	m_startingFence->Signal();

	return true;
}

void RenderGraph::FlattenRenderGraphRecursive(std::map<PassBase*, int>& levels, PassBase* node, int lastLevel, int level)
{
	if (levels.contains(node))
	{
		if (levels[node] == level ||							// current pass to the node has the same level as recorded
			(lastLevel < level && level <= levels[node]) ||		// going forward and current pass's level is less than recorded
			(lastLevel > level && level >= levels[node]))		// going backward and current pass's level is greater than recorded
		{
			return;
		}
	}

	levels[node] = level;

	for (auto& pass : node->GetDependencies())
	{
		FlattenRenderGraphRecursive(levels, pass, level, level + 1);
	}

	for (auto& pass : node->GetDependents())
	{
		FlattenRenderGraphRecursive(levels, pass, level, level - 1);
	}
}


bool RenderGraph::Validate()
{
	if (m_flattenedPassGraph.size() != m_allPasses.size())
	{
		return false;
	}

	if (m_finalRenderOutputPass && !m_finalRenderOutputPass->GetFinalRenderPassOutputResource())
	{
		return false;
	}

	if (!m_finalPass)
	{
		return false;
	}

	// TODO: Dependency validations
	return true;
}

void RenderGraph::WaitForRenderFinalOutputDone()
{
	if (m_finalPass)
	{
		WaitForSingleObject(m_renderOutputWorkDoneEvent, INFINITE);
	}
}

bool RenderGraph::UpdateBuffers(World* world)
{
	if (!world)
	{
		return false;
	}

	bool succ = world->UpdateBuffers();

	for (auto& pass : m_allPasses)
	{
		succ &= pass->UpdateBuffers(world);
	}

	return succ;
}

bool RenderGraph::ParseFile(std::vector<RenderGraph::PipelineStruct>& outPipelines, std::map<GUID, std::set<GUID, GuidComparator>, GuidComparator>& outExtraDependencies, std::string& outFinalRenderOutputPass, std::string& outFinalPass)
{
	outPipelines.clear();
	outFinalRenderOutputPass.clear();

	if (!std::filesystem::exists(m_filePath))
	{
		return false;
	}

	std::ifstream file(m_filePath);
	bool succ = true;
	try
	{
		json data = json::parse(file);
		unsigned char buf[256] = { 0 };

		if (data.contains("RenderGraph") && data["RenderGraph"].is_object())
		{
			auto renderGraph = data["RenderGraph"];
			if (renderGraph.contains("Pipelines") && renderGraph["Pipelines"].is_array())
			{
				auto pipelines = renderGraph["Pipelines"];
				if (pipelines.size() > 0)
				{
					for (int i = 0; i < pipelines.size(); i++) 
					{
						auto pipeline = pipelines[i];
						if (!(pipeline.contains("Name") && pipeline["Name"].is_string()) ||
							!(pipeline.contains("Guid") && pipeline["Guid"].is_string()) ||
							!(pipeline.contains("Passes") && pipeline["Passes"].is_array()))
						{
							succ = false;
							break;
						}

						GUID pipelineGuid;
						memset(buf, 0, 256);
						std::string pipelineGuidStr = pipeline["Guid"];
						memcpy(buf, pipelineGuidStr.c_str(), pipelineGuidStr.size());
						if (UuidFromStringA(buf, &pipelineGuid) != RPC_S_OK)
						{
							succ = false;
							break;
						}

						PipelineStruct newPipelineStruct = {pipeline["Name"], pipelineGuid, std::vector<PassStruct>()};

						for (int j = 0; j < pipeline["Passes"].size(); j++)
						{
							auto pass = pipeline["Passes"][j];
							if (!pass.is_object() ||
								!pass.contains("Name") || !pass["Name"].is_string() ||
								!pass.contains("Guid") || !pass["Guid"].is_string() ||
								!pass.contains("Type") || !pass["Type"].is_string())
							{
								succ = false;
								break;
							}

							PassType passType = Utils::GetPassTypeFromString(pass["Type"]);
							if (passType == PassType::NONE)
							{
								succ = false;
								break;
							}

							GUID passGuid;
							memset(buf, 0, 256);
							std::string passGuidStr = pass["Guid"];
							memcpy(buf, passGuidStr.c_str(), passGuidStr.size());
							if (UuidFromStringA(buf, &passGuid) != RPC_S_OK)
							{
								succ = false;
								break;
							}

							PassStruct newPassStruct = { pass["Name"], passGuid, passType };
							newPipelineStruct.Passes.push_back(newPassStruct);
						}

						outPipelines.push_back(newPipelineStruct);
					}
				}
			}

			if (renderGraph.contains("Dependencies") && renderGraph["Dependencies"].is_array())
			{
				auto dependencies = renderGraph["Dependencies"];
				if (dependencies.size() > 0)
				{
					for (int i = 0; i < dependencies.size(); i++)
					{
						auto passDependency = dependencies[i];
						if (passDependency.is_object())
						{
							if (passDependency.contains("Guid") && passDependency["Guid"].is_string() &&
								passDependency.contains("Dependencies") && passDependency["Dependencies"].is_array() && passDependency["Dependencies"].size() > 0)
							{
								GUID passGuid;
								memset(buf, 0, 256);
								std::string passGuidStr = passDependency["Guid"];
								memcpy(buf, passGuidStr.c_str(), passGuidStr.size());
								if (UuidFromStringA(buf, &passGuid) != RPC_S_OK)
								{
									succ = false;
									break;
								}

								if (outExtraDependencies.contains(passGuid))
								{
									continue;
								}

								outExtraDependencies[passGuid] = std::set<GUID, GuidComparator>();

								auto dependencyGuidList = passDependency["Dependencies"];
								for (int j = 0; j < dependencyGuidList.size(); j++)
								{
									GUID dependencyGuid;
									memset(buf, 0, 256);
									std::string dependencyGuidStr = dependencyGuidList[j];
									memcpy(buf, dependencyGuidStr.c_str(), dependencyGuidStr.size());
									if (UuidFromStringA(buf, &dependencyGuid) != RPC_S_OK)
									{
										succ = false;
										break;
									}

									outExtraDependencies[passGuid].insert(dependencyGuid);
								}
							}
						}
					}
				}
			}

			if (renderGraph.contains("FinalRenderOutputPass") && renderGraph["FinalRenderOutputPass"].is_string())
			{
				outFinalRenderOutputPass = renderGraph["FinalRenderOutputPass"];
			}

			if (renderGraph.contains("FinalPass") && renderGraph["FinalPass"].is_string())
			{
				outFinalPass = renderGraph["FinalPass"];
			}
		}
	}
	catch(std::exception)
	{
		succ = false;
	}

	file.close();

	if (!succ)
	{
		outPipelines.clear();
	}

	return succ;
}