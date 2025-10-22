#pragma once
#include "Includes.h"
#include "World2.h"
#include "CommandBuilder.h"
#include "CommandQueue.h"
#include "TextureManager2.h"
#include "D3DFence.h"

/// <summary>
/// Streaming engine class: used for streaming in/out a worldr.
/// </summary>
class StreamingEngine
{
private:
	std::unique_ptr<CommandQueue> m_commandQueue;

	std::unique_ptr<CommandBuilder> m_commandBuilder;

	std::unique_ptr<D3DFence> m_fence;

	GraphicsContext* m_graphicsContext = nullptr;

	TextureManager2* m_textureManager = nullptr;

	bool m_initialized = false;

	bool m_streaming = false;

	HANDLE m_streamingThreadHandle = 0;

	World2* m_streamingWorld = nullptr;

	HANDLE m_streamingDoneEvent = NULL;

public:
	bool Initialize(GraphicsContext* graphicsContext, TextureManager2* textureManager);

	bool StartStreaming(World2* world);

	void StopStreaming();

private:
	static DWORD WINAPI StreamingThreadRoutine(LPVOID lpParam);

};