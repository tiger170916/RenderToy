#pragma once

#include "Includes.h"

enum PipelinePassOutputType
{
	NONE = 0,
	EARLY_Z_PASS_DEPTH_BUFFER = 1,
};

struct PipelineOutput
{
	UINT64 SrvId;
};

struct PipelineOutputsStruct
{
	std::map<PipelinePassOutputType, PipelineOutput> Outputs;
};