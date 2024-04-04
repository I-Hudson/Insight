#include "Asset/Importers/ModelImporter.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"
#include "Asset/Assets/TExture.h"

#include "Graphics/Vertex.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assimp/mesh.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultIOSystem.h>
#include <assimp/IOStream.hpp>

#include <meshoptimizer.h>

#include <future>

//#define THREADED_LOADING

namespace Insight
{
    namespace Runtime
    {
		class CustomAssimpIOStrean : public Assimp::IOStream
		{
		public:
			std::string Path;

			CustomAssimpIOStrean(std::string path, const AssetInfo* assetInfo)
				: Path(path)
				, m_assetInfo(assetInfo)
			{
				m_fileData = AssetRegistry::Instance().LoadAsset(m_assetInfo->GetFullFilePath());
				ASSERT(!m_fileData.empty());
			}

			virtual size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override
			{
				const size_t cnt = std::min(pCount, (m_fileData.size() - m_cursor) / pSize);
				const size_t ofs = pSize * cnt;

				Platform::MemCopy(pvBuffer, m_fileData.data() + m_cursor, ofs);
				m_cursor += ofs;
				return cnt;
			}

			virtual size_t Write(const void* pvBuffer, size_t pSize, size_t pCount)  override
			{
				FAIL_ASSERT();
				return 0;
			}

			virtual aiReturn Seek(size_t pOffset, aiOrigin pOrigin)  override
			{
				if (aiOrigin_SET == pOrigin)
				{
					if (pOffset > m_fileData.size())
					{
						return AI_FAILURE;
					}
					m_cursor = pOffset;
				}
				else if (aiOrigin_END == pOrigin)
				{
					if (pOffset > m_fileData.size())
					{
						return AI_FAILURE;
					}
					m_cursor = m_fileData.size() - pOffset;
				}
				else
				{
					if (pOffset + m_cursor > m_fileData.size())
					{
						return AI_FAILURE;
					}
					m_cursor += pOffset;
				}
				return aiReturn_SUCCESS;
			}

			virtual size_t Tell() const override
			{
				return m_cursor;
			}

			virtual size_t FileSize() const override
			{
				return m_fileData.size();
			}

			virtual void Flush() override
			{
			}

		private:
			const AssetInfo* m_assetInfo = nullptr;
			std::vector<Byte> m_fileData;
			u64 m_cursor = 0;
		};

		class CustomAssimpIOSystem : public Assimp::DefaultIOSystem
		{
			/*
				This must use the C++ new and delete and not Insight's own New and Delete
				as some loaders like gtlf uses shared_ptrs. Because of this if we use our own New
				then we never call our own Delete so Insight doesn't know that a pointer has been deleted.
			*/
		public:
			virtual bool Exists(const char* pFile) const override
			{
				return AssetRegistry::Instance().GetAsset(pFile) != nullptr;
			}

			virtual char getOsSeparator() const override
			{
				return '\\';
			}

			virtual Assimp::IOStream* Open(const char* pFile,
				const char* pMode = "rb") override
			{
				const AssetInfo* assetInfo = AssetRegistry::Instance().GetAsset(pFile);
				Assimp::IOStream* stream = new CustomAssimpIOStrean(pFile, assetInfo);
				Core::MemoryTracker::Instance().NameAllocation(stream, assetInfo->GetFullFilePath().c_str());
				return stream;
			}

			virtual void Close(Assimp::IOStream* pFile) override
			{
				delete pFile;
			}

		};

        ModelImporter::ModelImporter()
            : IAssetImporter(
                {
                    ".obj",
                    ".fbx",
                    ".gltf",
                })
        {
        }

        Ref<Asset> ModelImporter::Import(const AssetInfo* assetInfo, const std::string_view path) const
        {
			IS_PROFILE_FUNCTION();

			Assimp::Importer importer;
			// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

			CustomAssimpIOSystem ioSystem;
			importer.SetIOHandler(&ioSystem);

			const uint32_t importerFlags =
				// Switch to engine conventions
				// Validate and clean up
				aiProcess_ValidateDataStructure			/// Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct
				| aiProcess_Triangulate					/// Triangulates all faces of all meshes
				| aiProcess_SortByPType					/// Splits meshes with more than one primitive type in homogeneous sub-meshes.

				| aiProcess_MakeLeftHanded				/// DirectX style.
				//aiProcess_FlipUVs						/// DirectX style.
				| aiProcess_FlipWindingOrder			/// DirectX style.

				| aiProcess_CalcTangentSpace			/// Calculates the tangents and bitangents for the imported meshes.
				| aiProcess_GenSmoothNormals			/// Ignored if the mesh already has normal.
				| aiProcess_GenUVCoords					/// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.


				//| aiProcess_RemoveRedundantMaterials	/// Searches for redundant/unreferenced materials and removes them
				//| aiProcess_JoinIdenticalVertices		/// Triangulates all faces of all meshes
				//| aiProcess_FindDegenerates			/// Convert degenerate primitives to proper lines or points.
				//| aiProcess_FindInvalidData			/// This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them
				//| aiProcess_FindInstances				/// This step searches for duplicate meshes and replaces them with references to the first mesh
				;

			const aiScene* scene = importer.ReadFile(path.data(), importerFlags);
			importer.SetIOHandler(nullptr);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("[ModelImporter::Import] Assimp model load: {0}", importer.GetErrorString());
				return Ref<Asset>();
			}


			Ref<ModelAsset> modelAsset = New<ModelAsset>(assetInfo);
			modelAsset.Ptr()->m_assetState = AssetState::Loaded;

			std::vector<MeshNode*> meshNodes;
			{
				IS_PROFILE_SCOPE("Get complete mesh hierarchy");
				GetMeshHierarchy(scene, scene->mRootNode, nullptr, meshNodes);
				for (size_t i = 0; i < meshNodes.size(); ++i)
				{
					meshNodes[i]->Directory = assetInfo->FilePath;
				}
			}

			{

#ifdef THREADED_LOADING
				IS_PROFILE_SCOPE("Process all mesh nodes - threaded");
				std::vector<std::future<void>> processNodesFutures;
				for (size_t i = 0; i < meshNodes.size(); ++i)
				{
					processNodesFutures.push_back(std::async(&ModelImporter::ProcessNode, *this, meshNodes[i]));
				}

				{
					IS_PROFILE_SCOPE("Wait for all 'ProcessNode' to complete");
					for (size_t processNodeFutureIdx = 0; processNodeFutureIdx < processNodesFutures.size(); ++processNodeFutureIdx)
					{
						processNodesFutures[processNodeFutureIdx].get();
					}
				}
#else
				IS_PROFILE_SCOPE("Process all mesh nodes");
				for (size_t i = 0; i < meshNodes.size(); ++i)
				{
					ModelImporter::ProcessNode(meshNodes[i]);
				}
#endif
			}

			for (size_t i = 0; i < meshNodes.size(); ++i)
			{
				MeshNode*& meshNode = meshNodes[i];
				Delete(meshNode->Mesh);
				Delete(meshNode);
			}

            return modelAsset;
        }

		MeshNode* ModelImporter::GetMeshHierarchy(const aiScene* aiScene, const aiNode* aiNode, const MeshNode* parentMeshNode, std::vector<MeshNode*>& meshNodes) const
		{
			IS_PROFILE_FUNCTION();

			MeshNode* newMeshNode = New<MeshNode>();
			newMeshNode->AssimpNode = aiNode;
			newMeshNode->AssimpScene = aiScene;
			newMeshNode->Parent = parentMeshNode;
			newMeshNode->Mesh = New<Mesh>();
			meshNodes.push_back(newMeshNode);

			for (size_t childIdx = 0; childIdx < aiNode->mNumChildren; ++childIdx)
			{
				newMeshNode->Children.push_back(GetMeshHierarchy(aiScene, aiNode->mChildren[childIdx], newMeshNode, meshNodes));
			}
			return newMeshNode;
		}

		void ModelImporter::ProcessNode(MeshNode* meshNode) const
		{
			IS_PROFILE_FUNCTION();
			const aiNode* aiNode = meshNode->AssimpNode;
			const aiScene* aiScene = meshNode->AssimpScene;

			if (aiNode->mNumMeshes > 0)
			{
				for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					IS_PROFILE_SCOPE("Mesh evaluated");
					const aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
					ProcessMesh(aiScene, aiMesh, meshNode->Mesh);
					if (aiScene->HasMaterials() && aiMesh->mMaterialIndex < aiScene->mNumMaterials)
					{
						meshNode->AssimpMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
						ProcessMaterial(meshNode);
					}
				}
			}
		}

		void ModelImporter::ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, Mesh* mesh) const
		{
			IS_PROFILE_FUNCTION();

			std::vector<Graphics::Vertex> vertices;
			vertices.reserve(aiMesh->mNumVertices);

			/// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
			{
				IS_PROFILE_SCOPE("Add Vertex");

				Graphics::Vertex vertex = { };
				glm::vec4 vector = { }; /// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
				/// so we transfer the data to this placeholder glm::vec3 first.
				/// positions
				vector.x = aiMesh->mVertices[i].x;
				vector.y = aiMesh->mVertices[i].y;
				vector.z = aiMesh->mVertices[i].z;
				vector.w = 1.0f;
				vertex.Position = vector;

				vector = { };
				/// Normals
				if (aiMesh->HasNormals())
				{
					vector.x = aiMesh->mNormals[i].x;
					vector.y = aiMesh->mNormals[i].y;
					vector.z = aiMesh->mNormals[i].z;
					vector.w = 1.0f;
					vector = glm::normalize(vector);
				}
				vertex.Normal = vector;

				vector = { };
				if (aiMesh->mColors[0])
				{
					vector.x = aiMesh->mColors[0]->r;
					vector.y = aiMesh->mColors[0]->g;
					vector.z = aiMesh->mColors[0]->b;
					vector.w = aiMesh->mColors[0]->a;
				}
				else
				{
					vector.x = (rand() % 100 + 1) * 0.01f;
					vector.y = (rand() % 100 + 1) * 0.01f;
					vector.z = (rand() % 100 + 1) * 0.01f;
					vector.w = 1.0f;
				}
				vertex.Colour = vector;

				vector = { };
				/// texture coordinates
				if (aiMesh->mTextureCoords[0]) /// does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = aiMesh->mTextureCoords[0][i].x;
					vec.y = aiMesh->mTextureCoords[0][i].y;
					vertex.UV = glm::vec4(vec, 0, 0);
				}
				else
				{
					FAIL_ASSERT();
					///vertex.UV = glm::vec2(0.0f, 0.0f);
				}

				Graphics::VertexOptomised vertexOptomised(vertex);
				vertices.push_back(vertex);
			}

			std::vector<u32> indices;
			indices.reserve(aiMesh->mNumFaces * 3);

			/// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
			{
				aiFace face = aiMesh->mFaces[i];
				/// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}
		}

		void ModelImporter::ProcessMaterial(MeshNode* meshNode) const
		{
			IS_PROFILE_FUNCTION();

			const aiMaterial* aiMaterial = meshNode->AssimpMaterial;
			ASSERT_MSG(aiMaterial, "[ModelImporter::ProcessMaterial] AssimpMaterial from MeshNode was nullptr. This shouldn't happen.");
			const std::string materialname = aiMaterial->GetName().C_Str();

			const std::string diffuseTexturePath = GetTexturePath(aiMaterial, meshNode->Directory, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE);
			Ref<TextureAsset> diffuseTexture = AssetRegistry::Instance().LoadAsset2(diffuseTexturePath).As<TextureAsset>();

			const std::string normalTexturePath = GetTexturePath(aiMaterial, meshNode->Directory, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS);
			Ref<TextureAsset> normalTexture = AssetRegistry::Instance().LoadAsset2(normalTexturePath).As<TextureAsset>();

			aiColor4D colour(1.0f);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &colour);

			aiColor4D opacity(1.0f);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_OPACITY, &opacity);
		}

		std::string ModelImporter::GetTexturePath(const aiMaterial* aiMaterial, const std::string_view directory, const aiTextureType textureTypePBR, const aiTextureType textureTypeLegacy) const
		{
			aiTextureType textureType = aiTextureType_NONE;
			if (aiMaterial->GetTextureCount(textureTypePBR) > 0)
			{
				textureType = textureTypePBR;
			}
			else if (aiMaterial->GetTextureCount(textureTypeLegacy) > 0)
			{
				textureType = textureTypeLegacy;
			}

			if (textureType == aiTextureType_NONE)
			{
				IS_CORE_ERROR("[ModelImporter::GetTexturePath] Unable to find PRB/Legacy textures for material '{}'.", aiMaterial->GetName().C_Str());
				return std::string();
			}

			aiString texturePath;
			aiMaterial->GetTexture(textureType, 0, &texturePath);
			if (texturePath.length == 0)
			{
				IS_CORE_ERROR("[ModelImporter::GetTexturePath] Texture path for texture type '{}' was empty.", aiTextureTypeToString(textureType));
				return std::string();
			}

			return std::string(directory) + "/" + texturePath.C_Str();
		}
    }
}