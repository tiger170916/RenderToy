#pragma once
#include "fbxsdk.h"
#include "Includes.h"

class FbxLoader
{
private:
	bool m_loaded = false;

	std::string m_fileName;

	FbxManager* m_fbxManager = nullptr;

public:
	FbxLoader(std::string fileName);

	bool Load();

	~FbxLoader();

private:
	void ProcessNode(FbxNode* pNode);
};