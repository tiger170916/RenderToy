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

	float m_scale;

public:
	FbxLoader(std::string fileName, float scale = 1.0f);

	bool Load(std::vector<std::shared_ptr<StaticMesh>> & outMeshes);

	~FbxLoader();

private:
	void ProcessNode(FbxNode* pNode, std::vector<std::shared_ptr<StaticMesh>>& outMeshes);

	void ProcessMesh(FbxNode* pNode, FbxMesh* pMesh, std::vector<std::shared_ptr<StaticMesh>>& outMeshes);
};