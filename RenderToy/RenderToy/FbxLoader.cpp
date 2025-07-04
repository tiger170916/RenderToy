#include "FbxLoader.h"

FbxLoader::FbxLoader(std::string fileName)
    : m_fileName(fileName)
{
    
}

bool FbxLoader::Load()
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
    ProcessNode(lRootNode);
    m_loaded = true;

    return true;
}

void FbxLoader::ProcessNode(FbxNode* pNode)
{
    // Process the node's attributes
    for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i)
    {
        FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
        if (pAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* pMesh = static_cast<FbxMesh*>(pAttribute);

            // Access control points (vertices)
            int controlPointCount = pMesh->GetControlPointsCount();
            FbxVector4* controlPoints = pMesh->GetControlPoints();
            for (int j = 0; j < controlPointCount; ++j)
            {
                // Access vertex data (x, y, z)
                controlPoints[j][0], controlPoints[j][1], controlPoints[j][2];
            }

            // Access polygons (faces)
            int polygonCount = pMesh->GetPolygonCount();
            for (int j = 0; j < polygonCount; ++j)
            {
                int polygonSize = pMesh->GetPolygonSize(j);

                if (polygonSize >= 4)
                {
                    int lastPointIdx = pMesh->GetPolygonVertex(j, polygonSize - 1);
                    for (int k = 0; k < polygonSize - 2; k++) 
                    {
                        int idx1 = pMesh->GetPolygonVertex(j, k);
                        int idx2 = pMesh->GetPolygonVertex(j, k + 1);
                        // and lastPointIdx is the 3rd vertex of triangle
                    }
                }
                else
                {
                    for (int k = 0; k < polygonSize; ++k)
                    {
                        int controlPointIndex = pMesh->GetPolygonVertex(j, k);
                    }
                }
            }

            // FbxLayerElementNormal* normalLayer = pMesh->GetLayer(0)->GetNormals();
            // FbxLayerElementUV* uvLayer = pMesh->GetLayer(0)->GetUVs();
        }
    }

    // Recursively process child nodes
    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        ProcessNode(pNode->GetChild(i));
    }
}

FbxLoader::~FbxLoader()
{
    m_fbxManager->Destroy();
}