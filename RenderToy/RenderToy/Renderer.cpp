#include "Renderer.h"
#include "FbxLoader.h"

Renderer::~Renderer()
{

}

Renderer* Renderer::Get()
{
	static std::unique_ptr<Renderer> singleton = std::unique_ptr<Renderer>(new Renderer());

	return singleton.get();
}

bool Renderer::Initialize(HWND hwnd)
{
	if (m_initialized)
	{
		return true;
	}

	// Initialize graphics device
	m_graphicsContext = std::unique_ptr<GraphicsContext>(new GraphicsContext());

	if (!m_graphicsContext->Initialize(hwnd))
	{
		return false;
	}


	FbxLoader* fbxLoader = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\medieval-house-2\\source\\House2\\House2.fbx");
	fbxLoader->Load();

	FbxLoader* fbxLoader2 = new FbxLoader("C:\\Users\\erlie\\Desktop\\Assets\\medieval-house-2\\source\\House2\\House2.fbx");
	fbxLoader2->Load();

	m_initialized = true;
	return true;
}