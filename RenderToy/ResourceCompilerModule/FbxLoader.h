#pragma once
#include "fbxsdk.h"
#include "Structs.h"
#include "ResourceStructs.h"
#include <filesystem>
#include <vector>
#include <fstream>

using namespace ResourceCompilerModule;

class FbxLoader
{
private:
	bool m_loaded = false;

	FbxManager* m_fbxManager = nullptr;

	StaticMeshStruct* m_staticMeshStruct = nullptr;

	std::vector<MeshPartHeader> m_meshPartHeaders;

public:
	FbxLoader(StaticMeshStruct* staticMeshStruct);

	bool Load(bool loadHeaders, uint32_t* meshOffset, std::ofstream* file);

	inline const std::vector<MeshPartHeader>& GetMeshPartHeaders() { return m_meshPartHeaders; }

	~FbxLoader();

private:
	void ProcessNode(FbxNode* pNode, StaticMeshStruct* staticMeshStruct, bool loadHeaders, uint32_t* meshOffset, std::ofstream* file);

	void ProcessMesh(FbxNode* pNode, FbxMesh* pMesh, StaticMeshStruct* staticMeshStruct, bool loadHeaders, uint32_t* meshOffset, std::ofstream* file);
};
