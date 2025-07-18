#pragma once

#include "Includes.h"
#include "CommandQueue.h"
#include "PassBase.h"

class Pipeline
{
private:
	GUID m_guid;

	std::string m_name;

	std::vector<PassBase*> m_passes;

	std::unique_ptr<CommandQueue> m_commandQueue = nullptr;

public:
	Pipeline(std::string name, GUID guid);

	bool Initialize(GraphicsContext* graphicsContext);

	bool AddPass(PassBase* pPass);

	bool AddPassDependency(PassBase* passAddDependencyTo, PassBase* dependency);

	std::vector<PassBase*>& GetPasses() { return m_passes; }

	bool FindPass(PassBase* pass);

	CommandQueue* GetCommandQueue() { return m_commandQueue.get(); }

	~Pipeline();
};