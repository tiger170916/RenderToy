#include "FbxLoader.h"
#include "TextureLoader.h"

FbxLoader::FbxLoader(StaticMeshStruct* staticMeshStruct)
    : m_staticMeshStruct(staticMeshStruct)
{
}

bool FbxLoader::Load(bool loadHeaders, uint32_t* meshOffset, std::ofstream* file)
{
    if (!std::filesystem::exists(m_staticMeshStruct->AssetPath))
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
    if (!lImporter->Initialize(m_staticMeshStruct->AssetPath.string().c_str(), -1, m_fbxManager->GetIOSettings()))
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


    // Access normals
    FbxLayerElementNormal* pNormals = pMesh->GetLayer(0)->GetNormals();
    FbxLayerElementBinormal* pBinormals = pMesh->GetLayer(0)->GetBinormals();
    bool normalDirectIndex = pNormals ? pNormals->GetReferenceMode() == FbxLayerElement::EReferenceMode::eDirect : false;

    // Access material indices
    FbxLayerElementArrayTemplate<int>* pMaterialIndices = nullptr;
    FbxGeometryElement::EMappingMode materialMappingMode = FbxGeometryElement::eNone;

    // Check if it has material defined in the fbx
    bool hasMaterial = pMesh->GetElementMaterial(0) != nullptr;
    bool useOverrideMaterial = false;
    if (!staticMeshStruct->OverrideBaseColorTexture.empty())
    {
        useOverrideMaterial = true;
    }

    if (useOverrideMaterial)
    {
        if (loadHeaders)
        {
            MeshPartHeader meshPartHeader = {};
            meshPartHeader.NumVertices = triangleCount * 3;
            meshPartHeader.MeshDataSize = triangleCount * 3 * sizeof(MeshVertexDx);
            meshPartHeader.MeshDataOffset = 0;
            m_meshPartHeaders.push_back(meshPartHeader);
        }
        else
        {
            std::vector<MeshVertexDx> verticesDx;

            for (int triangleIndex = 0; triangleIndex < triangleCount; ++triangleIndex)
            {
                MeshVertexDx v1Dx {};
                MeshVertexDx v2Dx {};
                MeshVertexDx v3Dx {};

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

            // Load textures
            TextureLoader texLoader;

            meshPartHeader.BaseColorTextureOffset = *meshOffset;

            if (std::filesystem::exists(staticMeshStruct->OverrideBaseColorTexture))
            {
                texLoader.LoadTexture(
                    staticMeshStruct->OverrideBaseColorTexture,
                    file, meshOffset,
                    meshPartHeader.BaseColorTextureWidth,
                    meshPartHeader.BaseColorTextureHeight,
                    meshPartHeader.BaseColorTextureDataSize,
                    meshPartHeader.BaseColorNumChannels);

                std::filesystem::path baseColorFileName = staticMeshStruct->OverrideBaseColorTexture.stem();
                std::string baseColorFileNameStr = baseColorFileName.string();
                memcpy(meshPartHeader.BaseColorTextureName, baseColorFileNameStr.c_str(), baseColorFileNameStr.size());
            }

            meshPartHeader.MetallicTextureOffset = *meshOffset;
            if (std::filesystem::exists(staticMeshStruct->OverrideMetallicTexture))
            {
                texLoader.LoadTexture(
                    staticMeshStruct->OverrideMetallicTexture,
                    file, meshOffset,
                    meshPartHeader.MetallicTextureWidth,
                    meshPartHeader.MetallicTextureHeight,
                    meshPartHeader.MetallicTextureDataSize,
                    meshPartHeader.MetallicNumChannels);

                std::filesystem::path metallicFileName = staticMeshStruct->OverrideMetallicTexture.stem();
                std::string metallicFileNameStr = metallicFileName.string();
                memcpy(meshPartHeader.MetallicTextureName, metallicFileNameStr.c_str(), metallicFileNameStr.size());
            }


            if (std::filesystem::exists(staticMeshStruct->OverrideRoughnessTexture))
            {
                meshPartHeader.RoughnessTextureOffset = *meshOffset;
                texLoader.LoadTexture(
                    staticMeshStruct->OverrideRoughnessTexture,
                    file, meshOffset,
                    meshPartHeader.RoughnessTextureWidth,
                    meshPartHeader.RoughnessTextureHeight,
                    meshPartHeader.RoughnessTextureDataSize,
                    meshPartHeader.RoughnessNumChannels);

                std::filesystem::path roughnessFileName = staticMeshStruct->OverrideRoughnessTexture.stem();
                std::string roughnessFileNameStr = roughnessFileName.string();
                memcpy(meshPartHeader.RoughnessTextureName, roughnessFileNameStr.c_str(), roughnessFileNameStr.size());
            }

            meshPartHeader.NormalTextureOffset = *meshOffset;
            if (std::filesystem::exists(staticMeshStruct->OverrideNormalTexture))
            {
                texLoader.LoadTexture(
                    staticMeshStruct->OverrideNormalTexture,
                    file, meshOffset,
                    meshPartHeader.NormalTextureWidth,
                    meshPartHeader.NormalTextureHeight,
                    meshPartHeader.NormalTextureDataSize,
                    meshPartHeader.NormalNumChannels);

                std::filesystem::path normalFileName = staticMeshStruct->OverrideNormalTexture.stem();
                std::string normalFileNameStr = normalFileName.string();
                memcpy(meshPartHeader.NormalTextureName, normalFileNameStr.c_str(), normalFileNameStr.size());
            }
        }
    }

    
    /*
    bool isSingleMaterial = false; 
    int singleMaterialId = -1;  // -1 indicates no material 
    
    const auto materialIndices = pMesh->Getpolygonmaterial
    if (!pMesh->GetElementMaterial(13))
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

        //FVector3 position1 = FVector3((float)controlPoints[pointIdx1][0] / m_scale, (float)controlPoints[pointIdx1][1] / m_scale, (float)controlPoints[pointIdx1][2] / m_scale);
        //FVector3 position2 = FVector3((float)controlPoints[pointIdx2][0] / m_scale, (float)controlPoints[pointIdx2][1] / m_scale, (float)controlPoints[pointIdx2][2] / m_scale);
        //FVector3 position3 = FVector3((float)controlPoints[pointIdx3][0] / m_scale, (float)controlPoints[pointIdx3][1] / m_scale, (float)controlPoints[pointIdx3][2] / m_scale);

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
    //}*/
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