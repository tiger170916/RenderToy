#include "Renderer.h"

Renderer::~Renderer()
{

}

Renderer* Renderer::Get()
{
	static std::unique_ptr<Renderer> singleton = std::unique_ptr<Renderer>(new Renderer());

	return singleton.get();
}

bool Renderer::Initialize()
{
	if (m_initialized)
	{
		return true;
	}

	// Initialize graphics device
	m_graphicsContext = std::unique_ptr<GraphicsContext>(new GraphicsContext());

	if (!m_graphicsContext->Initialize())
	{
		return false;
	}

	m_initialized = true;
	return true;
}