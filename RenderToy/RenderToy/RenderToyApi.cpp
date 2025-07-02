#include "RenderToyApi.h"
#include "Renderer.h"

bool InitializeRenderer()
{
	Renderer* renderer = Renderer::Get();
	return renderer->Initialize();
}

bool FinalizeRenderer()
{
	return false;
}