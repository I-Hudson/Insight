#include "Graphics/Mesh.h"
#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"
#include "Core/Logger.h"

#include "Core/Profiler.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

namespace Insight
{
	namespace Graphics
	{
		Submesh::~Submesh()
		{
			Destroy();
		}

		void Submesh::Draw() const
		{
			Renderer::BindVertexBuffer(m_vertexInfo.Buffer);
			Renderer::BindIndexBuffer(*m_indexBuffer);

			const int indexCount = (int)(m_indexBuffer->GetSize() / sizeof(int));
			Renderer::DrawIndexed(indexCount, 1, 0, m_vertexInfo.VertexOffset, 0);
		}

		void Submesh::SetVertexInfo(SubmeshVertexInfo info)
		{
			m_vertexInfo = info;
		}

		void Submesh::SetIndexBuffer(RHI_Buffer* buffer)
		{
			m_indexBuffer = UPtr<RHI_Buffer>(buffer);
		}

		void Submesh::Destroy()
		{
			//if (m_vertexBuffer)
			//{
			//	Renderer::FreeVertexBuffer(m_vertexBuffer.Get());
			//	m_vertexBuffer.Release();
			//}
			m_vertexInfo = {};
			
			if (m_indexBuffer)
			{
				Renderer::FreeIndexBuffer(m_indexBuffer.Get());
				m_indexBuffer.Release();
			}
		}

		int vertexOffset = 0;
		int indexOffset = 0;

		Mesh::Mesh()
		{

		}

		bool Mesh::LoadFromFile(std::string filePath)
		{
			IS_PROFILE_FUNCTION();

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

			std::vector<Vertex> vertices;

			CreateGPUBuffers(scene, filePath, vertices);
			ProcessNode(scene->mRootNode, scene, "", vertices);

			const u64 expectedVertexByteSize = m_vertexBuffer->GetSize();
			const u64 vertexByteSize = vertices.size() * sizeof(Vertex);

			ASSERT(expectedVertexByteSize == vertexByteSize);

			m_vertexBuffer->Upload(vertices.data(), vertexByteSize);

			return true;
		}

		void Mesh::Destroy()
		{
			for (Submesh* submesh : m_submeshes)
			{
				submesh->Destroy();
				DeleteTracked(submesh);
			}

			if (m_vertexBuffer)
			{
				m_vertexBuffer->Release();
				m_vertexBuffer.Reset();
			}

			m_submeshes.clear();
		}

		void Mesh::Draw() const
		{
			IS_PROFILE_FUNCTION();
			for (Submesh* submesh : m_submeshes)
			{
				submesh->Draw();
			}
		}

		void Mesh::CreateGPUBuffers(const aiScene* scene, std::string_view filePath, const std::vector<Vertex>& vertices)
		{
			IS_PROFILE_FUNCTION();

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

			const u64 vertexByteSize = vertexCount * sizeof(Vertex);

			m_vertexBuffer = UPtr(RHI_Buffer::New());
			m_vertexBuffer->Create(GraphicsManager::Instance().GetRenderContext(), BufferType::Vertex, vertexByteSize, sizeof(Vertex));

			std::string_view shortFilePath = filePath.substr(filePath.find_last_of('/') + 1);
			std::wstring wShortFileName;
			std::transform(shortFilePath.begin(), shortFilePath.end(), std::back_inserter(wShortFileName), [](const char c)
				{
					return (wchar_t)c;
				});
		}

		void Mesh::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, std::vector<Vertex>& vertices)
		{
			IS_PROFILE_FUNCTION();

			if (aiNode->mNumMeshes > 0)
			{
				SubmeshVertexInfo submeshVertexInfo = { };
				submeshVertexInfo.VertexOffset = static_cast<int>(vertices.size());
				submeshVertexInfo.Buffer = m_vertexBuffer.Get();

				for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];

					std::vector<int> indices;
					ProcessMesh(aiMesh, aiScene, vertices, indices);

					const int vertexSizeBytes = (int)vertices.size() * (int)sizeof(Vertex);
					const int indexSizeBytes = (int)indices.size() * (int)sizeof(int);

					RHI_Buffer* iBuffer = Renderer::CreateIndexBuffer(indexSizeBytes);
					{
						//ZoneScopedN("Index_Upload");
						iBuffer->Upload(indices.data(), indexSizeBytes, 0);
					}

					submeshVertexInfo.VertexCount = static_cast<int>(vertices.size()) - submeshVertexInfo.VertexOffset;

					Submesh* subMesh = NewArgsTracked(Submesh, this);
					subMesh->SetVertexInfo(submeshVertexInfo);
					subMesh->SetIndexBuffer(iBuffer);
					m_submeshes.push_back(std::move(subMesh));
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
				ProcessNode(aiNode->mChildren[i], aiScene, directory, vertices);
			}
		}

		void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<int>& indices)
		{
			IS_PROFILE_FUNCTION();

			glm::vec4 vertexColour;
			vertexColour.x = (rand() % 100 + 1) * 0.01f;
			vertexColour.y = (rand() % 100 + 1) * 0.01f;
			vertexColour.z = (rand() % 100 + 1) * 0.01f;
			vertexColour.w = 1.0f;
 
			// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				Vertex vertex = { };
				glm::vec4 vector = { }; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
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
				vector = { };
				if (mesh->mColors[0])
				{
					vector.x = mesh->mColors[0]->r;
					vector.y = mesh->mColors[0]->g;
					vector.z = mesh->mColors[0]->b;
					vector.w = mesh->mColors[0]->a;
					vertex.Normal = vector;
				}
				else
				{
					vector.x = (rand() % 100 + 1) * 0.01f;
					vector.y = (rand() % 100 + 1) * 0.01f;
					vector.z = (rand() % 100 + 1) * 0.01f;
					vector.w = 1.0f;
					vertex.Colour = vertexColour;
				}
				vector = { };
				// texture coordinates
				if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					//vertex.UV = vec;
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
					//vertex.UV = glm::vec2(0.0f, 0.0f);
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