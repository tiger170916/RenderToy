#include "FbxLoader.h"
#include "Material.h"

FbxLoader::FbxLoader(std::string fileName, float scale)
    : m_fileName(fileName), m_scale(scale)
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

    FbxGeometryConverter converter(m_fbxManager);
    converter.Triangulate(lScene, true);

    // Destroy the importer
    lImporter->Destroy();

    // Process the scene's root node
    FbxNode* lRootNode = lScene->GetRootNode();
    ProcessNode(lRootNode, outMeshes);
    m_loaded = true;

    return true;
}

void FbxLoader::ProcessMesh(FbxNode* pNode, FbxMesh* pMesh, std::vector<std::shared_ptr<StaticMesh>>& outMeshes)
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

    // Create new static mesh
    StaticMesh* staticMesh = new StaticMesh();
    outMeshes.push_back(std::shared_ptr<StaticMesh>(staticMesh));

    // Access normals
    FbxLayerElementNormal* pNormals = pMesh->GetLayer(0)->GetNormals();
    bool normalDirectIndex = pNormals ? pNormals->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect : false;

    // Access material indices
    FbxLayerElementArrayTemplate<int>* pMaterialIndices = nullptr;
    FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;

    bool isSingleMaterial = false;
    int singleMaterialId = -1;  // -1 indicates no material 

    if (!pMesh->GetElementMaterial())
    {
        isSingleMaterial = true;
    }
    else
    {
        pMaterialIndices = &pMesh->GetElementMaterial()->GetIndexArray();
        materialMappingMode = pMesh->GetElementMaterial()->GetMappingMode();
    }

    int numUvSets = 0;
    const FbxGeometryElementUV* fbxUvs[8];
    // Access uv sets
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

    // Check if the mesh is referring to a single material
    if (!pMaterialIndices)
    {
        isSingleMaterial = true;
    }
    else
    {
        if (materialMappingMode == FbxGeometryElement::eAllSame)
        {
            isSingleMaterial = true;
            singleMaterialId = pMaterialIndices->GetAt(0);
        }
        else if (materialMappingMode != FbxGeometryElement::eByPolygon)
        {
            isSingleMaterial = true;
        }
    }

    // Add mesh parts
    for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
    {
        int materialIndex = isSingleMaterial ? singleMaterialId : pMaterialIndices->GetAt(triangleIndex);

        int vertexIdx1 = triangleIndex * 3;

        int pointIdx1 = pMesh->GetPolygonVertex(triangleIndex, 0);
        int pointIdx2 = pMesh->GetPolygonVertex(triangleIndex, 1);
        int pointIdx3 = pMesh->GetPolygonVertex(triangleIndex, 2);

        FVector3 normal1 = FVector3::Zero();
        FVector3 normal2 = FVector3::Zero();
        FVector3 normal3 = FVector3::Zero();

        // The mesh contains normal data
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

            normal1 = FVector3((float)n1[0], (float)n1[1], (float)n1[2]);
            normal2 = FVector3((float)n2[0], (float)n2[1], (float)n2[2]);
            normal3 = FVector3((float)n3[0], (float)n3[1], (float)n3[2]);
        }

        StaticMesh::MeshVertex vertex1
        {
            .Position = FVector3((float)controlPoints[pointIdx1][0] / m_scale, (float)controlPoints[pointIdx1][1] / m_scale, (float)controlPoints[pointIdx1][2] / m_scale),
            .Normal = normal1,
        };
        StaticMesh::MeshVertex vertex2
        {
            .Position = FVector3((float)controlPoints[pointIdx2][0] / m_scale, (float)controlPoints[pointIdx2][1] / m_scale, (float)controlPoints[pointIdx2][2] / m_scale),
            .Normal = normal2,
        };
        StaticMesh::MeshVertex vertex3
        {
            .Position = FVector3((float)controlPoints[pointIdx3][0] / m_scale, (float)controlPoints[pointIdx3][1] / m_scale, (float)controlPoints[pointIdx3][2] / m_scale),
            .Normal = normal3,
        };

        for (int uvSet = 0; uvSet < numUvSets; uvSet++)
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

            vertex1.UvSets[uvSet] = FVector2((float)uv1[0], (float)uv1[1]);
            vertex2.UvSets[uvSet] = FVector2((float)uv2[0], (float)uv2[1]);
            vertex3.UvSets[uvSet] = FVector2((float)uv3[0], (float)uv3[1]);
        }

        staticMesh->AddTriangle(materialIndex, vertex1, vertex2, vertex3);
    } 

    staticMesh->SetNumUvs(numUvSets);

    // Get all materials
    //int materialCount = pNode->GetSrcObjectCount<FbxSurfaceMaterial>();
    //for (int materialIdx = 0; materialIdx < materialCount; materialIdx++)
    //{
        FbxSurfaceMaterial* material = pNode->GetSrcObject<FbxSurfaceMaterial>(0);
        if (material)
        {
            FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
            int layers = prop.GetSrcObjectCount<FbxLayeredTexture>();
            if (layers == 0)
            {
                int textureCount = prop.GetSrcObjectCount<FbxTexture>();
                for (int j = 0; j < textureCount; j++)
                {
                }
            }
        }

        Material* newMaterial = new Material();
        
        std::filesystem::path fbxPath = m_fileName;
        std::filesystem::path baseColor = fbxPath.replace_filename("BaseColor.png");
        std::filesystem::path metallic = fbxPath.replace_filename("Metallic.png");
        std::filesystem::path roughness = fbxPath.replace_filename("Roughness.png");
        std::filesystem::path normal = fbxPath.replace_filename("Normal.png");
        std::filesystem::path ambientOcclusion = fbxPath.replace_filename("AmbientOcclusion.png");
        newMaterial->SetBaseColorTexturePath(baseColor);
        newMaterial->SetMetallicTexturePath(metallic);
        newMaterial->SetRoughnessTexturePath(roughness);
        newMaterial->SetNormalTexturePath(normal);
        newMaterial->SetAmbientOcclusionTexturePath(ambientOcclusion);

        staticMesh->AddMaterial(newMaterial);
   //}
}

void FbxLoader::ProcessNode(FbxNode* pNode, std::vector<std::shared_ptr<StaticMesh>>& outMeshes)
{
    // Process the node's attributes
    for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i)
    {
        FbxNodeAttribute* pAttribute = pNode->GetNodeAttributeByIndex(i);
        if (pAttribute->GetAttributeType() == FbxNodeAttribute::eMesh)
        {
            FbxMesh* pMesh = static_cast<FbxMesh*>(pAttribute);
            ProcessMesh(pNode, pMesh, outMeshes);
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