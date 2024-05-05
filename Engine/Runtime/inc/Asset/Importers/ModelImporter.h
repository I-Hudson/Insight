#pragma once

#include "Asset/Importers/IAssetImporter.h"

#include "Graphics/Vertex.h"

#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType : int;

namespace Insight
{
    namespace Graphics
    {
        class RHI_Buffer;
    }

    namespace Runtime
    {
        class Mesh;

        struct MeshData
        {
            void Optimise();
            void GenerateLODs();

            struct LOD
            {
                LOD(const u32 index, const u32 vertexOffset, const u32 vertexCount, const u32 firstIndx, const u32 indexCount)
                    : LOD_index(index), Vertex_offset(vertexOffset), Vertex_count(vertexCount), First_index(firstIndx), Index_count(indexCount)
                { }

                u32 LOD_index = 0;
                u32 Vertex_offset = 0;
                u32 Vertex_count = 0;
                u32 First_index = 0;
                u32 Index_count = 0;
            };

            std::vector<Graphics::Vertex> Vertices;
            std::vector<u32> Indices;
            std::vector<LOD> LODs;

            Graphics::RHI_Buffer* RHI_VertexBuffer = nullptr;
            Graphics::RHI_Buffer* RHI_IndexBuffer = nullptr;

            Graphics::RHI_Buffer* RHI_MonolithVertexBuffer = nullptr;
            Graphics::RHI_Buffer* RHI_MonolithIndexBuffer = nullptr;
        };

        struct MeshNode
        {
            const aiScene* AssimpScene = nullptr;
            const aiNode* AssimpNode = nullptr;
            const aiMaterial* AssimpMaterial = nullptr;

            const MeshNode* Parent = nullptr;
            MeshData* MeshData = nullptr;

            std::vector<MeshNode*> Children;
            Mesh* Mesh;
            std::string_view Directory;
            std::string_view FileName;
        };


        class ModelImporter : public IAssetImporter
        {
        public:
            ModelImporter();

            virtual Ref<Asset> Import(const AssetInfo* assetInfo, const std::string_view path) const override;

        private:
            MeshNode* GetMeshHierarchy(const aiScene* aiScene, const aiNode* aiNode, const MeshNode* parentMeshNode, std::vector<MeshNode*>& meshNodes, MeshData* monolithMeshData = nullptr) const;
            void PreallocateVeretxAndIndexBuffers(MeshNode* meshNode) const;
            void ProcessNode(MeshNode* meshNode) const;
            void ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, MeshData* meshData) const;
            void ProcessMaterial(MeshNode* meshNode) const;

            /// @brief Returns the texture path from the model directory.
            /// @param aiMaterial 
            /// @param textureType 
            /// @param textureTypelegcy 
            /// @return 
            std::string GetTexturePath(const aiMaterial* aiMaterial, const std::string_view directory, const aiTextureType textureTypePBR, const aiTextureType textureTypeLegacy) const;
        };
    }
}