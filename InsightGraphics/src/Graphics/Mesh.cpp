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
		void Submesh::Draw() const
		{
			const int indexCount = m_indexView.GetSize() / sizeof(int);
			Renderer::DrawIndexed(indexCount, 1, 0, 0, 0);
		}

		int vertexOffset = 0;
		int indexOffset = 0;

		Mesh::Mesh()
		{

		}

		bool Mesh::LoadFromFile(std::string filePath)
		{
			ZoneScoped;

			vertexOffset = 0;
			indexOffset = 0;

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

			CreateGPUBuffers(scene, filePath);
			ProcessNode(scene->mRootNode, scene, "");

			return true;
		}

		void Mesh::Draw() const
		{
			Renderer::BindVertexBuffer(*m_vertexBuffer);
			Renderer::BindIndexBuffer(*m_indexBuffer);
			for (const Submesh& submesh : m_submeshes)
			{
				submesh.Draw();
			}
		}

		void Mesh::CreateGPUBuffers(const aiScene* scene, std::string_view filePath)
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

			std::string_view shortFilePath = filePath.substr(filePath.find_last_of('/') + 1);
			std::wstring wShortFileName;
			std::transform(shortFilePath.begin(), shortFilePath.end(), std::back_inserter(wShortFileName), [](const char c)
				{
					return (wchar_t)c;
				});

			m_vertexBuffer = UPtr(Renderer::CreateVertexBuffer(vertexCount * sizeof(Vertex)));
			m_vertexBuffer->SetName(wShortFileName + L"_Mesh_Vertex_Buffer");
			m_indexBuffer = UPtr(Renderer::CreateIndexBuffer(indexCount * sizeof(int)));
			m_indexBuffer->SetName(wShortFileName + L"_Mesh_Index_Buffer");
		}

		void Mesh::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory)
		{
			ZoneScoped;

			if (aiNode->mNumMeshes > 0)
			{
				for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];

					std::vector<Vertex> vertices;
					vertices.reserve(aiMesh->mNumVertices);
					std::vector<int> indices;
					ProcessMesh(aiMesh, aiScene, vertices, indices);

					const int vertexSizeBytes = (int)vertices.size() * (int)sizeof(Vertex);
					const int indexSizeBytes = (int)indices.size() * (int)sizeof(int);

					const int vertexOffsetSizeBytes = vertexOffset * (int)sizeof(Vertex);
					const int indexOffsetSizeBytes = indexOffset * (int)sizeof(int);

					Submesh subMesh(this);
					subMesh.SetVertexView(RHI_BufferView(*m_vertexBuffer, vertexOffsetSizeBytes, vertexSizeBytes));
					subMesh.SetIndexView(RHI_BufferView(*m_indexBuffer, indexOffsetSizeBytes, indexSizeBytes));
					m_submeshes.push_back(subMesh);

					// Upload data
					m_vertexBuffer->Upload(vertices.data(), vertexSizeBytes, vertexOffsetSizeBytes);
					m_indexBuffer->Upload(indices.data(), indexSizeBytes, indexOffsetSizeBytes);

					vertexOffset += (int)vertices.size();
					indexOffset += (int)indices.size();
				}

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

		void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<int>& indices)
		{
			// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex;
				glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
				// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;
				// normals
				if (mesh->HasNormals())
				{
					vector.x = mesh->mNormals[i].x;
					vector.y = mesh->mNormals[i].y;
					vector.z = mesh->mNormals[i].z;
					vertex.Normal = vector;
				}
				// texture coordinates
				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.UV = vec;
					// tangent
					if (mesh->mTangents)
					{
						vector.x = mesh->mTangents[i].x;
						vector.y = mesh->mTangents[i].y;
						vector.z = mesh->mTangents[i].z;
					}
					//vertex.Tangent = vector;
					// bitangent
					if (mesh->mBitangents)
					{
						vector.x = mesh->mBitangents[i].x;
						vector.y = mesh->mBitangents[i].y;
						vector.z = mesh->mBitangents[i].z;
					}
					//vertex.Bitangent = vector;
				}
				else
				{
					vertex.UV = glm::vec2(0.0f, 0.0f);
				}

				vertices.push_back(vertex);
			}

			// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			// process materials
			aiMaterial* material = aiScene->mMaterials[mesh->mMaterialIndex];
			// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
			// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			// Same applies to other texture as the following list summarizes:
			// diffuse: texture_diffuseN
			// specular: texture_specularN
			// normal: texture_normalN

			// 1. diffuse maps
			//vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			//textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			// 2. specular maps
			//vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			//textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			// 3. normal maps
			//std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			//textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			// 4. height maps
			//std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			//textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
	}
}