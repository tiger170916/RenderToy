#pragma once
#include "Includes.h"
#include "World.h"
#include "CommandBuilder.h"
#include "CommandQueue.h"
#include "EarlyZPass.h"
#include "GeometryPass.h"
#include "LightingPass.h"
#include "GraphicsContext.h"
#include "ShaderManager.h"

/// <summary>
/// Render pipeline
/// </summary>
class RenderPipeline
{
private:
	std::unique_ptr<PipelineResourceStates> m_pipelineResourceStates = nullptr;

	std::unique_ptr<EarlyZPass> m_earlyZPass = nullptr;

	std::unique_ptr<GeometryPass> m_geometryPass = nullptr;

	std::unique_ptr<CommandQueue> m_commandQueue = nullptr;

	std::unique_ptr<CommandBuilder> m_commandBuilder = nullptr;	// May use parallel pipeline when workflows getting complicated

	bool m_initialized = false;

public:
	RenderPipeline();

	bool Initialize(GraphicsContext* graphicsContext, ShaderManager* shaderManager);

	~RenderPipeline();

	void FrameBegin();

	void Frame(GraphicsContext* graphicsContext, World* worldToRender);

	void FrameEnd();
};