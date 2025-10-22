#include "StreamingEngine.h"


bool StreamingEngine::Initialize(GraphicsContext* graphicsContext, TextureManager2* textureManager)
{
	if (m_initialized)
	{
		return true;
	}

	if (!graphicsContext)
	{
		return false;
	}

	UINT adapterMask = graphicsContext->GetAdapterNodeMask();
	ID3D12Device* pDevice = graphicsContext->GetDevice();

	m_commandQueue = std::unique_ptr<CommandQueue>(
		new CommandQueue(
			D3D12_COMMAND_QUEUE_FLAG_NONE,
			D3D12_COMMAND_LIST_TYPE_COPY,
			D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			adapterMask));

	m_fence = std::make_unique<D3DFence>();
	if (!m_fence->Initialize(pDevice))
	{
		return false;
	}

	if (!m_commandQueue->Initialize(pDevice))
	{
		return false;
	}
	
	m_commandBuilder = std::unique_ptr<CommandBuilder>(new CommandBuilder(D3D12_COMMAND_LIST_TYPE_COPY));
	if (!m_commandBuilder->Initialize(pDevice))
	{
		return false;
	}
	m_commandBuilder->GetCommandList()->SetName(L"Streaming Engine Copy CommandList");

	m_streamingDoneEvent = CreateEventA(NULL, true, true, NULL);

	m_graphicsContext = graphicsContext;

	m_textureManager = textureManager;

	m_initialized = true;

	return true;
}

bool StreamingEngine::StartStreaming(World2* world)
{
	if (m_streaming || m_streamingThreadHandle != NULL)
	{
		return false;
	}

	if (world == nullptr)
	{
		return false;
	}

	m_streaming = true;
	m_streamingWorld = world;

	m_streamingThreadHandle = CreateThread(NULL, 0, StreamingThreadRoutine, this, 0, NULL);
	if (m_streamingThreadHandle == NULL)
	{
		m_streaming = false;
		m_streamingWorld = nullptr;
		return false;
	}

	return true;
}

void StreamingEngine::StopStreaming()
{
	m_streaming = false;

	WaitForSingleObject(m_streamingThreadHandle, 100000);
	CloseHandle(m_streamingThreadHandle);

	m_streamingThreadHandle = NULL;
}

DWORD StreamingEngine::StreamingThreadRoutine(LPVOID lpParam)
{
	StreamingEngine* engine = (StreamingEngine*)lpParam;
	if (!engine)
	{
		return 1;
	}
	
	while (engine->m_streaming)
	{
		// Stream in world
		engine->m_streamingWorld->StreamInAroundActiveCameraRange(engine->m_graphicsContext, engine->m_commandBuilder.get(), 50.0f);

		// Stream in textures

		engine->m_textureManager->StreamInBinary(engine->m_graphicsContext, engine->m_commandBuilder.get());

		engine->m_commandBuilder->Close();
		if (engine->m_commandQueue->DispatchCommands(engine->m_commandBuilder.get()))
		{
			ResetEvent(engine->m_streamingDoneEvent);
			engine->m_fence->CommandQueueSignalAndSetEvent(engine->m_commandQueue->GetCommandQueue(), engine->m_streamingDoneEvent);
			WaitForSingleObject(engine->m_streamingDoneEvent, INFINITE);

			engine->m_textureManager->SetStreamedInDone();
		}
	}

	return 0;
}