#include "FbxLoader.h"
#include "TextureLoader.h"

FbxLoader::FbxLoader(StaticMeshStruct* staticMeshStruct)
    : m_staticMeshStruct(staticMeshStruct)
{
}

FbxLoader::FbxLoader(std::filesystem::path path, int overrideMaterialIdx)
    :m_filePath(path), m_overrideMaterialIdx(overrideMaterialIdx) {
}


bool FbxLoader::Load(bool loadHeaders, uint32_t* meshOffset, std::ofstream* file)
{
    if (!std::filesystem::exists(m_filePath))
    {
        return false;
    }

	// Support only if there is an override material for now
    if (m_overrideMaterialIdx < 0)
    {
        return false;
    }

    // Initialize the FBX SDK manager
    m_fbxManager = FbxManager::Create();
    if (!m_fbxManager)
    {
        m_fbxManager->Destroy();
        m_fbxManager = nullptr;
        return false;
    }

    // Create an IO settings object
    FbxIOSettings* ios = FbxIOSettings::Create(m_fbxManager, IOSROOT);
    m_fbxManager->SetIOSettings(ios);


    // Create an importer
    FbxImporter* lImporter = FbxImporter::Create(m_fbxManager, "");
    if (!lImporter->Initialize(m_filePath.string().c_str(), -1, m_fbxManager->GetIOSettings()))
    {
        const char* str = lImporter->GetStatus().GetErrorString();

        m_fbxManager->Destroy();
        m_fbxManager = nullptr;
        return false;

    }

    // Create a scene and import the file contents into it
    FbxScene* lScene = FbxScene::Create(m_fbxManager, "LoadedScene");
    lImporter->Import(lScene);

    FbxGeometryConverter converter(m_fbxManager);
    converter.Triangulate(lScene, true);

    // Destroy the importer
    lImporter->Destroy();

    // Process the scene's root node
    FbxNode* lRootNode = lScene->GetRootNode();
    ProcessNode(lRootNode, m_staticMeshStruct, loadHeaders, meshOffset, file);
    m_loaded = true;
    
    return true; 
}

void FbxLoader::ProcessMesh(FbxNode* pNode, FbxMesh* pMesh, StaticMeshStruct* staticMeshStruct, bool loadHeaders, uint32_t* meshOffset, std::ofstream* file)
{
    if (!pNode || !pMesh || !pMesh->IsTriangleMesh()) // expecting a triangle mesh
    {
        return;
    }

    int triangleCount = pMesh->GetPolygonCount();
    // Access points
    FbxVector4* controlPoints = pMesh->GetControlPoints();
    if (!controlPoints)
    {
        return;
    }

    if (loadHeaders)
    {
        if (m_overrideMaterialIdx >= 0)
        {
            MeshPartHeader meshPartHeader = {};
            meshPartHeader.NumVertices = triangleCount * 3;
            meshPartHeader.MeshDataSize = triangleCount * 3 * sizeof(MeshVertexDx);
            meshPartHeader.MeshDataOffset = 0;
			meshPartHeader.MaterialIdx = m_overrideMaterialIdx;
            m_meshPartHeaders.push_back(meshPartHeader);
        }

        return;
    }

    // Access normals
    FbxLayerElementNormal* pNormals = pMesh->GetLayer(0)->GetNormals();
    FbxLayerElementBinormal* pBinormals = pMesh->GetLayer(0)->GetBinormals();
    bool normalDirectIndex = pNormals ? pNormals->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect : false;

    // Access uv sets
    int numUvSets = 0;
    const FbxGeometryElementUV* fbxUvs[8];
    for (int layerIdx = 0; layerIdx < pMesh->GetLayerCount(); layerIdx++)
    {
        FbxLayer* layer = pMesh->GetLayer(layerIdx);
        if (!layer)
        {
            continue;
        }

        //iterate all uv channel indexed by element_texture_type
        for (int textureIndex = FbxLayerElement::EType::eTextureDiffuse; textureIndex < FbxLayerElement::EType::eTypeCount; textureIndex++)
        {
            FbxLayerElement* uvElement = layer->GetUVs(FbxLayerElement::EType(textureIndex));
            if (uvElement)
            {
                const char* uvSetName = uvElement->GetName();
                const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(uvSetName);
                if (lUVElement)
                {
                    fbxUvs[numUvSets] = lUVElement;
                    numUvSets++;
                }
            }
        }
    }

    std::vector<MeshVertexDx> verticesDx;

    for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
    {
        MeshVertexDx v1Dx{};
        MeshVertexDx v2Dx{};
        MeshVertexDx v3Dx{};

        int vertexIdx1 = triangleIndex * 3;

        int pointIdx1 = pMesh->GetPolygonVertex(triangleIndex, 0);
        int pointIdx2 = pMesh->GetPolygonVertex(triangleIndex, 1);
        int pointIdx3 = pMesh->GetPolygonVertex(triangleIndex, 2);

        FbxVector4 v1 = controlPoints[pointIdx1];
        FbxVector4 v2 = controlPoints[pointIdx2];
        FbxVector4 v3 = controlPoints[pointIdx3];

        v1Dx.position[0] = (float)v1[0];
        v1Dx.position[1] = (float)v1[1];
        v1Dx.position[2] = (float)v1[2];

        v2Dx.position[0] = (float)v2[0];
        v2Dx.position[1] = (float)v2[1];
        v2Dx.position[2] = (float)v2[2];

        v3Dx.position[0] = (float)v3[0];
        v3Dx.position[1] = (float)v3[1];
        v3Dx.position[2] = (float)v3[2];

        for (int uvSet = 0; uvSet < 1; uvSet++)
        {
            const FbxGeometryElementUV* elementUv = fbxUvs[uvSet];
            const bool lUseIndex = elementUv->GetReferenceMode() != FbxGeometryElement::eDirect;

            int uvIdx1 = 0, uvIdx2 = 0, uvIdx3 = 0;
            if (elementUv->GetMappingMode() == FbxGeometryElement::eByControlPoint)
            {
                uvIdx1 = lUseIndex ? elementUv->GetIndexArray().GetAt(pointIdx1) : pointIdx1;
                uvIdx2 = lUseIndex ? elementUv->GetIndexArray().GetAt(pointIdx2) : pointIdx2;
                uvIdx3 = lUseIndex ? elementUv->GetIndexArray().GetAt(pointIdx3) : pointIdx3;
            }
            else if (elementUv->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
            {
                uvIdx1 = lUseIndex ? elementUv->GetIndexArray().GetAt(vertexIdx1) : vertexIdx1;
                uvIdx2 = lUseIndex ? elementUv->GetIndexArray().GetAt(vertexIdx1 + 1) : vertexIdx1 + 1;
                uvIdx3 = lUseIndex ? elementUv->GetIndexArray().GetAt(vertexIdx1 + 2) : vertexIdx1 + 2;
            }
            else
            {
                continue;
            }

            FbxVector2 uv1 = elementUv->GetDirectArray().GetAt(uvIdx1);
            FbxVector2 uv2 = elementUv->GetDirectArray().GetAt(uvIdx2);
            FbxVector2 uv3 = elementUv->GetDirectArray().GetAt(uvIdx3);

            v1Dx.UV0[0] = (float)uv1[0];
            v1Dx.UV0[1] = (float)uv1[1];
            v2Dx.UV0[0] = (float)uv2[0];
            v2Dx.UV0[1] = (float)uv2[1];
            v3Dx.UV0[0] = (float)uv3[0];
            v3Dx.UV0[1] = (float)uv3[1];
        }

        if (pNormals)
        {
            int nId1 = 0, nId2 = 0, nId3 = 0;
            if (pNormals->GetMappingMode() == FbxLayerElement::eByControlPoint)
            {
                nId1 = normalDirectIndex ? pointIdx1 : pNormals->GetIndexArray().GetAt(pointIdx1);
                nId2 = normalDirectIndex ? pointIdx2 : pNormals->GetIndexArray().GetAt(pointIdx2);
                nId3 = normalDirectIndex ? pointIdx3 : pNormals->GetIndexArray().GetAt(pointIdx3);
            }
            else if (pNormals->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
            {
                nId1 = normalDirectIndex ? pointIdx1 : pNormals->GetIndexArray().GetAt(vertexIdx1);
                nId2 = normalDirectIndex ? pointIdx2 : pNormals->GetIndexArray().GetAt(vertexIdx1 + 1);
                nId3 = normalDirectIndex ? pointIdx3 : pNormals->GetIndexArray().GetAt(vertexIdx1 + 2);
            }

            FbxVector4 n1 = pNormals->GetDirectArray().GetAt(nId1);
            FbxVector4 n2 = pNormals->GetDirectArray().GetAt(nId2);
            FbxVector4 n3 = pNormals->GetDirectArray().GetAt(nId3);

			v1Dx.Normal[0] = (float)n1[0];
            v1Dx.Normal[1] = (float)n1[1];
            v1Dx.Normal[2] = (float)n1[2];

            v2Dx.Normal[0] = (float)n2[0];
            v2Dx.Normal[1] = (float)n2[1];
            v2Dx.Normal[2] = (float)n2[2];

            v3Dx.Normal[0] = (float)n3[0];
            v3Dx.Normal[1] = (float)n3[1];
            v3Dx.Normal[2] = (float)n3[2];
           
        }

        verticesDx.push_back(v1Dx);
        verticesDx.push_back(v2Dx);
        verticesDx.push_back(v3Dx);
    }

    uint32_t meshSize = (uint32_t)verticesDx.size() * sizeof(MeshVertexDx);
    file->seekp(*meshOffset);
    file->write((char*)verticesDx.data(), meshSize);

    MeshPartHeader& meshPartHeader = m_meshPartHeaders[0];
    m_meshPartHeaders[0].MeshDataOffset = *meshOffset;
    (*meshOffset) += meshSize;

}

void FbxLoader::ProcessNode(FbxNode* pNode, StaticMeshStruct* staticMeshStruct, bool loadHeaders, uint32_t* meshOffset, std::ofstream* file)
{
    // Process the node's attributes
    for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i)
    {
        FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
        if (pAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* pMesh = static_cast<FbxMesh*>(pAttribute);
            ProcessMesh(pNode, pMesh, staticMeshStruct, loadHeaders, meshOffset, file);
        }
    }

    // Recursively process child nodes
    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        ProcessNode(pNode->GetChild(i), staticMeshStruct, loadHeaders, meshOffset, file);
    }
}

FbxLoader::~FbxLoader()
{
    m_fbxManager->Destroy();
}