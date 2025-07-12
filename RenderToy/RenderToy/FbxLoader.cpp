#include "FbxLoader.h"

FbxLoader::FbxLoader(std::string fileName)
    : m_fileName(fileName)
{
    
}

bool FbxLoader::Load(std::vector<std::shared_ptr<StaticMesh>>& outMeshes)
{
    if (m_loaded)
    {
        return true;
    }

    // Initialize the FBX SDK manager
    m_fbxManager = FbxManager::Create();
    if (!m_fbxManager)
    {
        m_fbxManager->Destroy();
        return false;
    }

    // Create an IO settings object
    FbxIOSettings* ios = FbxIOSettings::Create(m_fbxManager, IOSROOT);
    m_fbxManager->SetIOSettings(ios);


    // Create an importer
    FbxImporter* lImporter = FbxImporter::Create(m_fbxManager, "");
    if (!lImporter->Initialize(m_fileName.c_str(), -1, m_fbxManager->GetIOSettings()))
    {
        m_fbxManager->Destroy();
        return false;
    }

    // Create a scene and import the file contents into it
    FbxScene* lScene = FbxScene::Create(m_fbxManager, "LoadedScene");
    lImporter->Import(lScene);

    // Destroy the importer
    lImporter->Destroy();

    // Process the scene's root node
    FbxNode* lRootNode = lScene->GetRootNode();
    ProcessNode(lRootNode, outMeshes);
    m_loaded = true;

    return true;
}

void FbxLoader::ProcessNode(FbxNode* pNode, std::vector<std::shared_ptr<StaticMesh>>& outMeshes)
{
    // Process the node's attributes
    for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i)
    {
        FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
        if (pAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            std::shared_ptr<StaticMesh> staticMesh = std::shared_ptr<StaticMesh>(new StaticMesh());
            outMeshes.push_back(staticMesh);

            FbxMesh* pMesh = static_cast<FbxMesh*>(pAttribute);

            // Access control points (vertices)
            int controlPointCount = pMesh->GetControlPointsCount();
            FbxVector4* controlPoints = pMesh->GetControlPoints();
            for (int j = 0; j < controlPointCount; ++j)
            {
                float x = (float)controlPoints[j][0] / 100.0f;
                float y = (float)controlPoints[j][1] / 100.0f;
                float z = (float)controlPoints[j][2] / 100.0f;
                staticMesh->AddPoint(x, y, z);
            }

            // Access polygons (faces)
            int polygonCount = pMesh->GetPolygonCount();
            for (int j = 0; j < polygonCount; ++j)
            {
                int polygonSize = pMesh->GetPolygonSize(j);

                if (polygonSize >= 4)
                {
                    int lastPointIdx = pMesh->GetPolygonVertex(j, polygonSize - 1);

                    // Triangulate
                    for (int k = 0; k < polygonSize - 2; k++) 
                    {
                        int idx1 = pMesh->GetPolygonVertex(j, k);
                        int idx2 = pMesh->GetPolygonVertex(j, k + 1);
                        staticMesh->AddTriangle(idx1, idx2, lastPointIdx);
                    }
                }
                else if (polygonSize == 3)
                {
                    int idx1 = pMesh->GetPolygonVertex(j, 0);
                    int idx2 = pMesh->GetPolygonVertex(j, 1);
                    int idx3 = pMesh->GetPolygonVertex(j, 2);
                    staticMesh->AddTriangle(idx1, idx2, idx3);
                }
            }

            // FbxLayerElementNormal* normalLayer = pMesh->GetLayer(0)->GetNormals();
            // FbxLayerElementUV* uvLayer = pMesh->GetLayer(0)->GetUVs();
        }
    }

    // Recursively process child nodes
    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        ProcessNode(pNode->GetChild(i), outMeshes);
    }
}

FbxLoader::~FbxLoader()
{
    m_fbxManager->Destroy();
}