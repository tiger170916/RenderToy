#include "RenderToyApi.h"
#include "Renderer.h"

bool InitializeRenderer(HWND hwnd)
{
	Renderer* renderer = Renderer::Get();
	return renderer->Initialize(hwnd);
}

bool StartRendering()
{
	Renderer* renderer = Renderer::Get();
	return renderer->RenderStart();
}

bool FinalizeRenderer()
{
	return false;
}