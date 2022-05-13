#include "Graphics/Mesh.h"
#include "Graphics/RenderContext.h"

#include "Tracy.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

namespace Insight
{
	namespace Graphics
	{
		Mesh::Mesh()
		{

		}

		bool Mesh::LoadFromFile(std::string filePath)
		{
			ZoneScoped;

			Assimp::Importer importer;
			const aiScene* scene = importer.ReadFile(filePath,
				aiProcess_Triangulate
				| aiProcess_FlipUVs
				| aiProcess_GenNormals
				| aiProcess_OptimizeMeshes
				//| aiProcess_OptimizeGraph
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
				return false;
			}

			CreateGPUBuffers(scene);
		}

		void Mesh::CreateGPUBuffers(const aiScene* scene)
		{
			int vertexCount = 0;
			int indexCount = 0;
			std::function<void(aiNode* node, const aiScene* scene)> getVertexAndIndexCount;
			getVertexAndIndexCount = [&getVertexAndIndexCount, &vertexCount, &indexCount](aiNode* node, const aiScene* scene)
			{
				// process all the node's meshes (if any)
				for (unsigned int i = 0; i < node->mNumMeshes; i++)
				{
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					vertexCount += mesh->mNumVertices;
					for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
					{
						aiFace face = mesh->mFaces[faceIndex];
						indexCount += face.mNumIndices;
					}
				}
				// then do the same for each of its children
				for (unsigned int i = 0; i < node->mNumChildren; i++)
				{
					getVertexAndIndexCount(node->mChildren[i], scene);
				}
			};
			getVertexAndIndexCount(scene->mRootNode, scene);

			m_vertexBuffer = UPtr(Renderer::CreateVertexBuffer(vertexCount * sizeof(Vertex)));
			m_indexBuffer = UPtr(Renderer::CreateIndexBuffer(indexCount * sizeof(int)));
		}

		void Mesh::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory)
		{
			ZoneScoped;

			if (aiNode->mNumMeshes > 0)
			{
				//Mesh* mesh = ::New<Mesh, MemoryCategory::Core>(&model, static_cast<u32>(model.m_meshes.size()));
				//Animation::Skeleton* skeleton = ::New<Animation::Skeleton, MemoryCategory::Core>();;
				//// process all the node's meshes (if any)
				//for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				//{
				//	aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
				//	mesh->m_subMeshes.push_back(ProcessMesh(*mesh, aiMesh, aiNode, aiScene, directory));

				//	ExtractSkeleton(*skeleton, mesh->m_vertices, aiMesh, aiScene, mesh);
				//}
				//model.m_meshes.push_back(mesh);
				//model.m_skeletons.push_back(skeleton);

				//if (skeleton->GetBoneCount() > 0)
				//{
				//	model.m_meshToSkeleton.emplace((u32)model.m_meshes.size() - 1, (u32)model.m_skeletons.size() - 1);
				//	model.m_skeletonToMesh.emplace((u32)model.m_skeletons.size() - 1, (u32)model.m_meshes.size() - 1);
				//}
			}

			// then do the same for each of its children
			for (u32 i = 0; i < aiNode->mNumChildren; i++)
			{
				ProcessNode(aiNode->mChildren[i], aiScene, directory);
			}
		}

	}
}