#pragma once

#include "Includes.h"
#include "GraphicsContext.h"
#include "CommandBuilder.h"

/// <summary>
/// IStreamable interface
/// </summary>
class IStreamable
{
protected:
	bool m_streamedIn = false;
public:
	/// <summary>
	/// Stream in the content from binary file
	/// </summary>
	virtual bool StreamInBinary(GraphicsContext* graphicsContext, CommandBuilder* cmdBuilder) = 0;

	/// <summary>
	/// Stream out the content
	/// </summary>
	virtual bool StreamOut() = 0;

	/// <summary>
	/// Clean up the intermediate format after streaming in is done.
	/// </summary>
	virtual bool CleanUpAfterStreamIn() = 0;

	inline bool IsStreamedIn() const { return m_streamedIn; }
};