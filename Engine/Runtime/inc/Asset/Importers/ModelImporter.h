#pragma once

#include "Asset/Importers/IAssetImporter.h"

#include "Asset/Assets/Material.h"
#include "Resource/AnimationClip.h"

#include "Graphics/Vertex.h"

#include "Maths/Matrix4.h"
#include "Maths/Quaternion.h"

#include <assimp/matrix4x4.h>
#include <assimp/quaternion.h>
#include <vector>

#define EXP_MODEL_LOADING 1

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiAnimation;
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
        class Skeleton;
        struct SkeletonBone;

        struct MeshData
        {
            void Optimise();
            void GenerateLODs();

            struct LOD
            {
                LOD() { }
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
            std::vector<Graphics::VertexBoneInfluence> VerticesBoneInfluence;
            std::vector<u32> Indices;
            std::vector<LOD> LODs;

            Graphics::RHI_Buffer* RHI_VertexBuffer = nullptr;
            Graphics::RHI_Buffer* RHI_IndexBuffer = nullptr;

            Graphics::RHI_Buffer* RHI_MonolithVertexBuffer = nullptr;
            Graphics::RHI_Buffer* RHI_MonolithIndexBuffer = nullptr;

            Maths::Matrix4 TransformOffset;
            std::string Name;
        };

        struct MeshNode
        {
            ~MeshNode()
            {
                Delete(MeshData);
                Delete(Mesh);
            }

            const aiScene* AssimpScene = nullptr;
            const aiNode* AssimpNode = nullptr;
            const aiMesh* AssimpMesh = nullptr;
            const aiMaterial* AssimpMaterial = nullptr;

            const MeshNode* Parent = nullptr;
            MeshData* MeshData = nullptr;

            ModelAsset* Model = nullptr;

            // Store all the materials found when loading the model in a cache to be reused if needed by multiple meshes.
            std::unordered_map<const aiMaterial*, Ref<MaterialAsset>>* MaterialCache;
            mutable std::mutex MaterialCacheLock;

            std::vector<MeshNode*> Children;
            Mesh* Mesh;
            std::string_view Directory;
            std::string_view FileName;
            const AssetInfo* AssetInfo = nullptr;
        };


        class ModelImporter : public IAssetImporter
        {
        public:
            ModelImporter();

            virtual Ref<Asset> CreateAsset(const AssetInfo* assetInfo) const  override;
            virtual void Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const override;

        private:
#if ENABLED_UFBX
            void ProcessNodeUfbx(const ufbx_scene* fbxScene, const ufbx_node* fbxNode, ModelAsset* modelAsset) const;
            void ProcessMeshUfbx(const ufbx_scene* fbxScene, const ufbx_node* fbxNode, const ufbx_mesh* fbxMesh, ModelAsset* modelAsset) const;
            void ParseMeshDataUfbx(const ufbx_scene* fbxScene, const ufbx_node* fbxNode, const ufbx_mesh* fbxMesh, MeshData& meshData, ModelAsset* modelAsset) const;
            Ref<MaterialAsset> ProcessMaterialUfbx(const ufbx_scene* fbxScene, const ufbx_node* fbxNode, const ufbx_material* materialsData, const u32 materialsCount, ModelAsset* modelAsset) const;

            void ExtractBoneWeights(const ufbx_scene* fbxScene, const ufbx_node* fbxNode, const ufbx_mesh* fbxMesh, const u32 index, MeshData* meshData, ModelAsset* modelAsset) const;

            void ProcessAnimations(const ufbx_scene* fbxScene, ModelAsset* modelAsset) const;

            Maths::Matrix4 UfbxToInsightMatrix4(const ufbx_matrix& matrix) const;
#elif EXP_MODEL_LOADING
            void ProcessNode(const aiScene* aiScene, const aiNode* aiNode, ModelAsset* modelAsset) const;
            void ProcessMesh(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, ModelAsset* modelAsset) const;
            void ParseMeshData(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, MeshData& meshData, ModelAsset* modelAsset) const;
            Ref<MaterialAsset> ProcessMaterial(const aiScene* aiScene, const aiNode* aiNode, const aiMaterial* aiMaterial, ModelAsset* modelAsset) const;
            Ref<TextureAsset> LoadTexture(const aiScene* assimpScene, const aiMaterial* assimpMaterial, const aiTextureType PBRType, const aiTextureType legacyType, ModelAsset* modelAsset) const;

            void ExtractSkeleton(const aiScene* aiScene, const aiNode* aiNode, Maths::Matrix4 parentTransform, ModelAsset* modelAsset) const;
            const aiNode* FindRootBone(const aiScene* aiScene, const aiNode* node, ModelAsset* modelAsset) const;
            void BuildBoneHierarchy(const aiScene* aiScene, const aiNode* bone, Maths::Matrix4 transform, SkeletonBone* parentBone, ModelAsset* modelAsset)  const;

            void ExtractBoneWeights(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, MeshData* meshData, ModelAsset* modelAsset) const;

            void ProcessAnimations(const aiScene* aiScene, ModelAsset* modelAsset) const;
#if ANIMATION_NODE_TRANSFORMS
            void ReadHierarchyData(AnimationNode& node, const aiNode* assimpNode) const;
            void ReadMissingBones(const aiAnimation* animation, ModelAsset* modelAsset) const;
#endif
#else
            MeshNode* GetMeshHierarchy(const aiScene* aiScene, const aiNode* aiNode, const MeshNode* parentMeshNode, std::vector<MeshNode*>& meshNodes, MeshData* monolithMeshData = nullptr) const;
            void PreallocateVeretxAndIndexBuffers(MeshNode* meshNode) const;
            void ProcessNode(MeshNode* meshNode) const;
            void ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, MeshData* meshData, MeshNode* meshNode) const;
            Ref<MaterialAsset> ProcessMaterial(MeshNode* meshNode) const;
#endif
            void SetVertexBoneData(Graphics::Vertex& vertex, Graphics::VertexBoneInfluence& vertexBoneInfluence, const u32 boneId, const float boneWeight) const;
            void ProcessMesh(MeshData& meshData, ModelAsset* modelAsset) const;

            /// @brief Returns the texture path from the model directory.
            /// @param aiMaterial 
            /// @param textureType 
            /// @param textureTypelegcy 
            /// @return 
            std::string GetTexturePath(const aiMaterial* aiMaterial, const std::string_view directory, const aiTextureType textureTypePBR, const aiTextureType textureTypeLegacy) const;

            Maths::Vector3 AssimpToInsightVector3(const aiVector3D& vector) const;
            Maths::Quaternion AssimpToInsightQuaternion(const aiQuaternion& quaternion) const;
            Maths::Matrix4 AssimpToInsightMatrix4(const aiMatrix4x4& transform) const;
        };
    }
}