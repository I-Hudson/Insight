#pragma once

#include "Asset/Importers/IAssetImporter.h"

#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType : int;

namespace Insight
{
    namespace Runtime
    {
        class Mesh;

        struct MeshNode
        {
            const aiScene* AssimpScene = nullptr;
            const aiNode* AssimpNode = nullptr;
            const aiMaterial* AssimpMaterial = nullptr;

            const MeshNode* Parent = nullptr;
            std::vector<MeshNode*> Children;
            Mesh* Mesh;
            std::string_view Directory;
        };


        class ModelImporter : public IAssetImporter
        {
        public:
            ModelImporter();

            virtual Ref<Asset> Import(const AssetInfo* assetInfo, const std::string_view path) const override;

        private:
            MeshNode* GetMeshHierarchy(const aiScene* aiScene, const aiNode* aiNode, const MeshNode* parentMeshNode, std::vector<MeshNode*>& meshNodes) const;
            void ProcessNode(MeshNode* meshNode) const;
            void ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, Mesh* mesh) const;
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