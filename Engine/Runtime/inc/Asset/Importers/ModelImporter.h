#pragma once

#include "Asset/Importers/IAssetImporter.h"

#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Insight
{
    namespace Runtime
    {
        class Mesh;

        struct MeshNode
        {
            const aiNode* AssimpNode;
            const aiScene* AssimpScene;
            const MeshNode* Parent;
            std::vector<MeshNode*> Children;
            Mesh* Mesh;
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
        };
    }
}