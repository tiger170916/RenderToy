#include "RenderToyApi.h"
#include "Renderer.h"

bool InitializeRenderer(HWND hwnd)
{
	Renderer* renderer = Renderer::Get();
	return renderer->Initialize(hwnd);
}

bool FinalizeRenderer()
{
	return false;
}