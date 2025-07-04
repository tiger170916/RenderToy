#pragma once
#include "fbxsdk.h"
#include "Includes.h"
#include "StaticMesh.h"

class FbxLoader
{
private:
	bool m_loaded = false;

	std::string m_fileName;

	FbxManager* m_fbxManager = nullptr;

public:
	FbxLoader(std::string fileName);

	bool Load(std::vector<std::shared_ptr<StaticMesh>> & outMeshes);

	~FbxLoader();

private:
	void ProcessNode(FbxNode* pNode, std::vector<std::shared_ptr<StaticMesh>>& outMeshes);
};