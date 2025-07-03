#pragma once
#include "Includes.h"
#include "GraphicsContext.h"

/// <summary>
/// Renderer singleton class
/// </summary>
class Renderer
{
private:
	std::unique_ptr<GraphicsContext> m_graphicsContext = nullptr;

	bool m_initialized = false;

public:
	~Renderer();

	/// <summary>
	/// Get singleton
	/// </summary>
	static Renderer* Get();

	bool Initialize(HWND hwnd);

private:
	Renderer() {};
};