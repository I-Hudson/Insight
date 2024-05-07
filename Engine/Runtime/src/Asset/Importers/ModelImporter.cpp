#include "Asset/Importers/ModelImporter.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"
#include "Asset/Assets/Texture.h"

#include "Resource/Mesh.h"

#include "Graphics/RenderContext.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Algorithm/Vector.h"

#include "FileSystem/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assimp/mesh.h>
#include <assimp/IOSystem.hpp>
#include <assimp/DefaultIOSystem.h>
#include <assimp/IOStream.hpp>

#include <meshoptimizer.h>

#include <future>

#define SINGLE_MESH 0

// Can't use this as memory usage skyrockets. 
//#define THREADED_LOADING

namespace Insight
{
	namespace Runtime
	{
#if EXP_MODEL_LOADING
		std::unordered_map<const aiMaterial*, Ref<MaterialAsset>> MaterialCache;
#endif

		//=============================================
		// CustomAssimpIOStrean
		//=============================================
		class CustomAssimpIOStrean : public Assimp::IOStream
		{
		public:
			std::string Path;

			CustomAssimpIOStrean(std::string path, const AssetInfo* assetInfo)
				: Path(path)
				, m_assetInfo(assetInfo)
			{
				m_fileData = AssetRegistry::Instance().LoadAssetData(m_assetInfo->GetFullFilePath());
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

		//=============================================
		// CustomAssimpIOSystem
		//=============================================
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
				return AssetRegistry::Instance().GetAssetInfo(pFile) != nullptr;
			}

			virtual char getOsSeparator() const override
			{
				return '\\';
			}

			virtual Assimp::IOStream* Open(const char* pFile,
				const char* pMode = "rb") override
			{
				const AssetInfo* assetInfo = AssetRegistry::Instance().GetAssetInfo(pFile);
				Assimp::IOStream* stream = new CustomAssimpIOStrean(pFile, assetInfo);
				Core::MemoryTracker::Instance().NameAllocation(stream, assetInfo->GetFullFilePath().c_str());
				return stream;
			}

			virtual void Close(Assimp::IOStream* pFile) override
			{
				delete pFile;
			}

		};

		//=============================================
		// MeshData
		//=============================================
		void MeshData::Optimise()
		{
			IS_PROFILE_FUNCTION();

			std::vector<Graphics::Vertex> optimisedVertices;
			std::vector<u32> optimisedIndices;

			for (size_t i = 0; i < LODs.size(); ++i)
			{
				const u64 vertexOffset = LODs[i].Vertex_offset;
				const u64 indexOffset = LODs[i].First_index;

				const u64 vertex_count = LODs[i].Vertex_count;
				const u64 index_count = LODs[i].Index_count;

				const u64 vertex_size = sizeof(Graphics::Vertex);

				/// The optimization order is important
				std::vector<u32> remapTable(index_count); /// allocate temporary memory for the remap table
				size_t total_vertices_optimized = meshopt_generateVertexRemap(remapTable.data(), NULL, index_count, Vertices.data() + vertexOffset, vertex_count, sizeof(Graphics::Vertex));
				
				optimisedIndices.reserve(optimisedIndices.size() + index_count);
				optimisedVertices.reserve(optimisedVertices.size() + total_vertices_optimized);

				std::vector<Graphics::Vertex> remapVertices(vertex_count);
				std::vector<u32> remapIndices(index_count);

				meshopt_remapIndexBuffer(remapIndices.data(), NULL, index_count, remapTable.data());
				meshopt_remapVertexBuffer(remapVertices.data(), Vertices.data() + vertexOffset, vertex_count, sizeof(Graphics::Vertex), remapTable.data());

				/// Vertex cache optimization - reordering triangles to maximize cache locality
				IS_LOG_INFO("Optimizing vertex cache...");
				meshopt_optimizeVertexCache(remapIndices.data(), Indices.data() + indexOffset, index_count, vertex_count);

				/// Overdraw optimizations - reorders triangles to minimize overdraw from all directions
				IS_LOG_INFO("Optimizing overdraw...");
				meshopt_optimizeOverdraw(optimisedIndices.data() + optimisedIndices.size(), remapIndices.data(), index_count, &Vertices.data()->Position.x, vertex_count, vertex_size, 1.05f);

				/// Vertex fetch optimization - reorders triangles to maximize memory access locality
				IS_LOG_INFO("Optimizing vertex fetch...");
				meshopt_optimizeVertexFetch(optimisedVertices.data() + optimisedVertices.size(), Indices.data() + indexOffset, index_count, remapVertices.data(), vertex_count, vertex_size);
			}
		}

		void MeshData::GenerateLODs()
		{
			ASSERT(LODs.size() == 1);
			// Set the minimum amount of indices for any lod.
			const u32 lowestIndexCount = 1024;
			bool foundLowestIndexCountMesh = false;

			if (LODs[0].Index_count <= lowestIndexCount)
			{
				// LOD 0 is already at the minimum indices count so don't generate any more LODs.
				return;
			}

			const u64 LOD0_IndicesStart = LODs[0].First_index;
			const u64 LOD0_IndicesCount = LODs[0].Index_count;
			const u64 LOD0_VertexStart = LODs[0].Vertex_offset;
			const u64 LOD0_VertexCount = LODs[0].Vertex_count;

			for (u32 lodIdx = 1; lodIdx < Mesh::s_MAX_LOD_COUNT; ++lodIdx)
			{
				if (foundLowestIndexCountMesh)
				{
					break;
				}

				std::vector<u32> indcies_to_lod;

				const float error_rate = 1.0f;
				const float lodSplitPercentage = 1.0f / Mesh::s_MAX_LOD_COUNT;
				const float lodSplit = 1.0f - (lodSplitPercentage * lodIdx);
				const u32 target_index_count = static_cast<u32>(static_cast<float>(LODs.at(0).Index_count) * lodSplit);

				std::vector<u32>::iterator indices_begin = Indices.begin() + LOD0_IndicesStart;
				std::vector<u32> result_lod;
				result_lod.resize(LOD0_IndicesCount);

				// Try and simplify the mesh, try and preserve mesh topology.
				u64 resultIndexCount = meshopt_simplify(
					result_lod.data()
					, &*indices_begin
					, static_cast<u64>(LOD0_IndicesCount)
					, &(Vertices.data() + LOD0_VertexStart)->Position.x
					, LOD0_VertexCount
					, sizeof(Graphics::Vertex)
					, target_index_count
					, error_rate);

				const u64 PreviousLOD_IndicesCount = LODs[lodIdx - 1].Index_count;
				if (resultIndexCount == PreviousLOD_IndicesCount)
				{
					// Our simplified mesh has the same number of indices as our previous lod. 
					// So just break as we haven't made a simpler mesh.
					break;
				}
				else if (resultIndexCount <= lowestIndexCount)
				{
					foundLowestIndexCountMesh = true;
				}

				if (false && resultIndexCount > target_index_count)
				{
					// Try and simplify the mesh, doesn't preserve mesh topology.
					resultIndexCount = meshopt_simplifySloppy(
						  result_lod.data()
						, &*indices_begin
						, static_cast<u64>(LOD0_IndicesCount)
						, &(Vertices.data() + LOD0_VertexStart)->Position.x
						, LOD0_VertexCount
						, sizeof(Graphics::Vertex)
						, target_index_count
						, error_rate);
				}
				result_lod.resize(resultIndexCount);

				const u32 First_index = static_cast<u32>(Indices.size());
				const u32 Index_count = static_cast<u32>(resultIndexCount);
				LODs.push_back(MeshData::LOD(lodIdx, 0, LOD0_VertexCount, First_index, Index_count));

				std::move(result_lod.begin(), result_lod.end(), std::back_inserter(Indices));
			}
		}

		//=============================================
		// ModelImporter
		//=============================================
		ModelImporter::ModelImporter()
			: IAssetImporter(
				{
					".obj",
					".fbx",
					".gltf",
				})
		{
		}

		Ref<Asset> ModelImporter::CreateAsset(const AssetInfo* assetInfo) const
		{
			return Ref<ModelAsset>(::New<ModelAsset>(assetInfo));
		}

		void ModelImporter::Import(Ref<Asset>& asset, const AssetInfo* assetInfo, const std::string_view path) const
		{
			IS_PROFILE_FUNCTION();

			MaterialCache.clear();

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
				| aiProcess_FlipUVs						/// DirectX style.
				| aiProcess_FlipWindingOrder			/// DirectX style.

				| aiProcess_CalcTangentSpace			/// Calculates the tangents and bitangents for the imported meshes.
				| aiProcess_GenSmoothNormals			/// Ignored if the mesh already has normal.
				//| aiProcess_GenUVCoords					/// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.
				

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
				IS_LOG_CORE_ERROR("[ModelImporter::Import] Assimp model load: {0}", importer.GetErrorString());
				return;
			}


			Ref<ModelAsset> modelAsset = asset.As<ModelAsset>();
			modelAsset.Ptr()->m_assetState = AssetState::Loading;
			modelAsset->SetName(scene->mName.C_Str());

#if EXP_MODEL_LOADING
			ProcessNode(scene, scene->mRootNode, modelAsset);
#else
			std::unordered_map<const aiMaterial*, Ref<MaterialAsset>> materialCache;

			std::vector<MeshNode*> meshNodes;
			{
				IS_PROFILE_SCOPE("Get complete mesh hierarchy");
				GetMeshHierarchy(scene, scene->mRootNode, nullptr, meshNodes);
				for (size_t i = 0; i < meshNodes.size(); ++i)
				{
					meshNodes[i]->Directory = assetInfo->FilePath;
					meshNodes[i]->FileName = assetInfo->FileName;
					meshNodes[i]->MaterialCache = &materialCache;
					meshNodes[i]->AssetInfo = assetInfo;
				}
			}

			{
#ifdef THREADED_LOADING
				FAIL_ASSERT_MSG("[ModelImporter::Import] Please undefine 'THREADED_LOADING'. With this enabled currently the memory usage is maxed out");
				{
					IS_PROFILE_SCOPE("Preallocate all vertex and index buffers for all meshes");
					for (size_t i = 0; i < meshNodes.size(); ++i)
					{
						//PreallocateVeretxAndIndexBuffers(meshNodes[i]);
					}
				}

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
				{
					IS_PROFILE_SCOPE("Process all mesh nodes");
					for (size_t i = 0; i < meshNodes.size(); ++i)
					{
						ProcessNode(meshNodes[i]);
					}
				}
#endif
			}

			for (size_t i = 0; i < meshNodes.size(); ++i)
			{
				if (meshNodes[i]->Mesh)
				{
					modelAsset->m_meshes.push_back(meshNodes[i]->Mesh);
					if (meshNodes[i]->Mesh->GetMaterialAsset() != nullptr)
					{
						Algorithm::VectorAddUnique(modelAsset->m_materials, meshNodes[i]->Mesh->GetMaterialAsset());
					}
					meshNodes[i]->Mesh->m_assetInfo = modelAsset->GetAssetInfo();
					meshNodes[i]->Mesh = nullptr;
				}
			}

			for (size_t i = 0; i < meshNodes.size(); ++i)
			{
				MeshNode*& meshNode = meshNodes[i];
				Delete(meshNode->MeshData);
				//Delete(meshNode->Mesh);
				Delete(meshNode);
			}
			meshNodes.clear();
#endif
			MaterialCache.clear();
			modelAsset.Ptr()->m_assetState = AssetState::Loaded;
		}

#if EXP_MODEL_LOADING
		void ModelImporter::ProcessNode(const aiScene* aiScene, const aiNode* aiNode, Ref<ModelAsset> modelAsset) const
		{
			for (size_t meshIdx = 0; meshIdx < aiNode->mNumMeshes; ++meshIdx)
			{
				ProcessMesh(aiScene, aiNode, aiScene->mMeshes[aiNode->mMeshes[meshIdx]], modelAsset);
			}

			for (size_t childIdx = 0; childIdx < aiNode->mNumChildren; ++childIdx)
			{
				ProcessNode(aiScene, aiNode->mChildren[childIdx], modelAsset);
			}
		}

		void ModelImporter::ProcessMesh(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, Ref<ModelAsset> modelAsset) const
		{
			Mesh* mesh = ::New<Mesh>();
			modelAsset->m_meshes.push_back(mesh);

			const char* nodeName = aiNode->mName.C_Str();
			const char* meshName = aiMesh->mName.C_Str();
			if (strcmp(nodeName, "sponza_ornament") == 0
				|| strcmp(meshName, "sponza_ornament") == 0)
			{
				DEBUG_BREAK;
			}

			mesh->m_mesh_name = aiMesh->mName.C_Str();
			mesh->m_transform_offset = AssimpToGLMMat4(aiNode->mTransformation);

			MeshData meshData = { };
			ParseMeshData(aiMesh, meshData);

			if (!meshData.Vertices.empty() && !meshData.Indices.empty())
			{
				//meshData.GenerateLODs();
				//meshData.Optimise();

				ASSERT(mesh);

				if (!meshData.RHI_VertexBuffer)
				{
					meshData.RHI_VertexBuffer = Renderer::CreateVertexBuffer(meshData.Vertices.size() * sizeof(Graphics::Vertex), sizeof(Graphics::Vertex));
					meshData.RHI_VertexBuffer->Upload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}

				if (!meshData.RHI_IndexBuffer)
				{
					meshData.RHI_IndexBuffer = Renderer::CreateIndexBuffer(meshData.Indices.size() * sizeof(u32));
					meshData.RHI_IndexBuffer->Upload(meshData.Indices.data(), meshData.RHI_IndexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}

				mesh->m_lods.resize(meshData.LODs.size());
				for (size_t lodIdx = 0; lodIdx < meshData.LODs.size(); ++lodIdx)
				{
					MeshData::LOD meshDataLod = meshData.LODs[lodIdx];
					MeshLOD& meshLod = mesh->m_lods[lodIdx];
					meshLod.LOD_index = static_cast<u32>(lodIdx);

					meshLod.Vertex_offset = static_cast<u32>(meshDataLod.Vertex_offset);
					meshLod.Vertex_count = static_cast<u32>(meshDataLod.Vertex_count);
					meshLod.First_index = static_cast<u32>(meshDataLod.First_index);
					meshLod.Index_count = static_cast<u32>(meshDataLod.Index_count);

					meshLod.Vertex_buffer = meshData.RHI_VertexBuffer;
					meshLod.Index_buffer = meshData.RHI_IndexBuffer;

					const std::string vertexBufferName = std::string(aiNode->mName.C_Str()) + "_" + aiMesh->mName.C_Str() + "_Veretx";
					const std::string indexBufferName = std::string(aiNode->mName.C_Str()) + "_" + aiMesh->mName.C_Str() + "_Index";
					meshLod.Vertex_buffer->SetName(vertexBufferName);
					meshLod.Index_buffer->SetName(indexBufferName);
				}
			}

			if (aiScene->HasMaterials())
			{
				const aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
				Ref<MaterialAsset> material = ProcessMaterial(aiScene, aiNode, aiMaterial, modelAsset);

				modelAsset->m_materials.push_back(material);
				mesh->SetMaterial(material);
			}
		}

		void ModelImporter::ParseMeshData(const aiMesh* aiMesh, MeshData& meshData) const
		{
			IS_PROFILE_FUNCTION();

			//meshData.Vertices.reserve(meshData.Vertices.size() + static_cast<u64>(aiMesh->mNumVertices));
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
				//vertex.Position += glm::vec3(meshData.TransformOffset[3].xyz);

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
				meshData.Vertices.push_back(vertex);
			}

			//meshData.Indices.reserve(meshData.Indices.size() + (static_cast<u64>(aiMesh->mNumFaces) * 3));
			/// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
			{
				aiFace face = aiMesh->mFaces[i];
				/// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					meshData.Indices.push_back(face.mIndices[j]);
				}
			}

			meshData.LODs.push_back(
				MeshData::LOD(
					0,
					0,
					static_cast<u32>(meshData.Vertices.size()),
					0,
					static_cast<u32>(meshData.Indices.size())));
		}

		Ref<MaterialAsset> ModelImporter::ProcessMaterial(const aiScene* aiScene, const aiNode* aiNode, const aiMaterial* aiMaterial, Ref<ModelAsset> modelAsset) const
		{
			IS_PROFILE_FUNCTION();

			if (auto iter = MaterialCache.find(aiMaterial);
				iter != MaterialCache.end())
			{
				return iter->second;
			}

			ASSERT_MSG(aiMaterial, "[ModelImporter::ProcessMaterial] AssimpMaterial from MeshNode was nullptr. This shouldn't happen.");
			const std::string materialname = aiMaterial->GetName().C_Str();

			const std::string_view Directory = modelAsset->GetAssetInfo()->FilePath;
			const std::string diffuseTexturePath = GetTexturePath(aiMaterial, Directory, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE);
			Ref<TextureAsset> diffuseTexture = AssetRegistry::Instance().LoadAsset(diffuseTexturePath).As<TextureAsset>();

			const std::string normalTexturePath = GetTexturePath(aiMaterial, Directory, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS);
			Ref<TextureAsset> normalTexture = AssetRegistry::Instance().LoadAsset(normalTexturePath).As<TextureAsset>();

			aiColor4D colour(1.0f);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &colour);

			float opacity(1.0f);
			aiGetMaterialFloat(aiMaterial, AI_MATKEY_OPACITY, &opacity);

			Ref<MaterialAsset> material = ::New<MaterialAsset>(modelAsset->GetAssetInfo());
			material->SetName(materialname);

			material->SetTexture(TextureAssetTypes::Diffuse, diffuseTexture);
			material->SetTexture(TextureAssetTypes::Normal, normalTexture);

			material->SetProperty(MaterialAssetProperty::Colour_R, colour.r);
			material->SetProperty(MaterialAssetProperty::Colour_G, colour.g);
			material->SetProperty(MaterialAssetProperty::Colour_B, colour.b);
			material->SetProperty(MaterialAssetProperty::Colour_A, colour.a);

			material->SetProperty(MaterialAssetProperty::Opacity, opacity);

			MaterialCache[aiMaterial] = material;
			return material;
		}
#endif

		MeshNode* ModelImporter::GetMeshHierarchy(const aiScene* aiScene, const aiNode* aiNode, const MeshNode* parentMeshNode, std::vector<MeshNode*>& meshNodes, MeshData* monolithMeshData) const
		{
			IS_PROFILE_FUNCTION();

			MeshNode* newMeshNode = newMeshNode = New<MeshNode>();
			newMeshNode->AssimpNode = aiNode;
			newMeshNode->AssimpScene = aiScene;
			newMeshNode->Parent = parentMeshNode;
			if (aiNode->mNumMeshes > 0)
			{
				newMeshNode->AssimpMesh = aiScene->mMeshes[aiNode->mMeshes[0]];
				newMeshNode->MeshData = monolithMeshData != nullptr ? monolithMeshData : New<MeshData>();
				newMeshNode->Mesh = New<Mesh>();
			}
			meshNodes.push_back(newMeshNode);

			// Some nodes contain multiple meshes. As a MeshNode for us is a single mesh, we must create
			// additional MeshNodes if an assimp node has mulitple meshes so we are aligned on the interpretation
			// of the scene.
			// Hierarchy wise this should be fine as the meshes being created here are on the same "level".
			for (size_t meshIdx = 1; meshIdx < aiNode->mNumMeshes; ++meshIdx)
			{
				MeshNode* meshNode = New<MeshNode>();
				meshNode->AssimpNode = aiNode;
				meshNode->AssimpMesh = aiScene->mMeshes[aiNode->mMeshes[meshIdx]];
				meshNode->AssimpScene = aiScene;
				meshNode->Parent = parentMeshNode;
				meshNode->MeshData = monolithMeshData != nullptr ? monolithMeshData : New<MeshData>();
				meshNode->Mesh = New<Mesh>();
				meshNodes.push_back(meshNode);
			}

			for (size_t childIdx = 0; childIdx < aiNode->mNumChildren; ++childIdx)
			{
				newMeshNode->Children.push_back(GetMeshHierarchy(aiScene, aiNode->mChildren[childIdx], newMeshNode, meshNodes));
			}
			return newMeshNode;
		}

		void ModelImporter::PreallocateVeretxAndIndexBuffers(MeshNode* meshNode) const
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
					meshNode->MeshData->Vertices.reserve(meshNode->MeshData->Vertices.size() + static_cast<u64>(aiMesh->mNumVertices));
					u64 meshIndicesCount = 0;

					// Do the loop as if Assimp couldn't make all faces a triangle then will catch that edge case by not assuming that all faces area triangle.
					for (size_t faceIdx = 0; faceIdx < aiMesh->mNumFaces; ++faceIdx)
					{
						const aiFace& face = aiMesh->mFaces[faceIdx];
						for (size_t indicesIdx = 0; indicesIdx < face.mNumIndices; ++indicesIdx)
						{
							++meshIndicesCount;
						}
					}
					meshNode->MeshData->Indices.reserve(meshNode->MeshData->Indices.size() + static_cast<u64>(meshIndicesCount));
				}
			}
		}

		void ModelImporter::ProcessNode(MeshNode* meshNode) const
		{
			IS_PROFILE_FUNCTION();
			const aiNode* aiNode = meshNode->AssimpNode;
			const aiScene* aiScene = meshNode->AssimpScene;

			if (meshNode->AssimpMesh != nullptr)
			{
				//for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					IS_PROFILE_SCOPE("Mesh evaluated");
					const aiMesh* aiMesh = meshNode->AssimpMesh;// aiScene->mMeshes[aiNode->mMeshes[i]];
					meshNode->MeshData->TransformOffset = AssimpToGLMMat4(aiNode->mTransformation);
					ProcessMesh(aiScene, aiMesh, meshNode->MeshData);
					meshNode->Mesh->m_mesh_name = aiMesh->mName.C_Str();
					meshNode->Mesh->m_transform_offset = AssimpToGLMMat4(aiNode->mTransformation);

					if (aiScene->HasMaterials() && aiMesh->mMaterialIndex < aiScene->mNumMaterials)
					{
						meshNode->AssimpMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
						Ref<MaterialAsset> material = ProcessMaterial(meshNode);
						meshNode->Mesh->SetMaterial(material);
					}
				}
			}

			MeshData* meshData = meshNode->MeshData;
			if (meshData
				&& !meshData->Vertices.empty()
				&& !meshData->Indices.empty())
			{
				meshData->GenerateLODs();
				//meshData->Optimise();

				Mesh* mesh = meshNode->Mesh;
				ASSERT(mesh);

				if (!meshData->RHI_VertexBuffer)
				{
					meshData->RHI_VertexBuffer = Renderer::CreateVertexBuffer(meshData->Vertices.size() * sizeof(Graphics::Vertex), sizeof(Graphics::Vertex));
					meshData->RHI_VertexBuffer->Upload(meshData->Vertices.data(), meshData->RHI_VertexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}

				if (!meshData->RHI_IndexBuffer)
				{
					meshData->RHI_IndexBuffer = Renderer::CreateIndexBuffer(meshData->Indices.size() * sizeof(u32));
					meshData->RHI_IndexBuffer->Upload(meshData->Indices.data(), meshData->RHI_IndexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}

				mesh->m_lods.resize(meshData->LODs.size());
				for (size_t lodIdx = 0; lodIdx < meshData->LODs.size(); ++lodIdx)
				{
					MeshData::LOD meshDataLod = meshData->LODs[lodIdx];
					MeshLOD& meshLod = mesh->m_lods[lodIdx];
					meshLod.LOD_index = static_cast<u32>(lodIdx);

					meshLod.Vertex_offset = static_cast<u32>(meshDataLod.Vertex_offset);
					meshLod.Vertex_count = static_cast<u32>(meshDataLod.Vertex_count);
					meshLod.First_index = static_cast<u32>(meshDataLod.First_index);
					meshLod.Index_count = static_cast<u32>(meshDataLod.Index_count);

					meshLod.Vertex_buffer = meshData->RHI_VertexBuffer;
					meshLod.Index_buffer = meshData->RHI_IndexBuffer;

					const std::string vertexBufferName = std::string(meshNode->FileName) + "_" + aiNode->mName.C_Str() + "_Veretx";
					const std::string indexBufferName = std::string(meshNode->FileName) + "_" + aiNode->mName.C_Str() + "_Index";
					meshLod.Vertex_buffer->SetName(vertexBufferName);
					meshLod.Index_buffer->SetName(indexBufferName);
				}

			}
		}

		void ModelImporter::ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, MeshData* meshData) const
		{
			IS_PROFILE_FUNCTION();

			meshData->Vertices.reserve(meshData->Vertices.size() + static_cast<u64>(aiMesh->mNumVertices));
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
				//vertex.Position += glm::vec3(meshData->TransformOffset[3].xyz);

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
				meshData->Vertices.push_back(vertex);
			}

			meshData->Indices.reserve(meshData->Indices.size() + (static_cast<u64>(aiMesh->mNumFaces) * 3));
			/// Now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
			{
				aiFace face = aiMesh->mFaces[i];
				/// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; ++j)
				{
					meshData->Indices.push_back(face.mIndices[j]);
				}
			}

			meshData->LODs.push_back(
				MeshData::LOD(
					0, 
					0, 
					static_cast<u32>(meshData->Vertices.size()),
					0,
					static_cast<u32>(meshData->Indices.size())));
		}

		Ref<MaterialAsset> ModelImporter::ProcessMaterial(MeshNode* meshNode) const
		{
			IS_PROFILE_FUNCTION();

			const aiMaterial* aiMaterial = meshNode->AssimpMaterial;

			{
				std::lock_guard lock(meshNode->MaterialCacheLock);
				if (auto iter = (*meshNode->MaterialCache).find(aiMaterial);
					iter != (*meshNode->MaterialCache).end())
				{
					return iter->second;
				}
			}

			ASSERT_MSG(aiMaterial, "[ModelImporter::ProcessMaterial] AssimpMaterial from MeshNode was nullptr. This shouldn't happen.");
			const std::string materialname = aiMaterial->GetName().C_Str();

			const std::string diffuseTexturePath = GetTexturePath(aiMaterial, meshNode->Directory, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE);
			Ref<TextureAsset> diffuseTexture = AssetRegistry::Instance().LoadAsset(diffuseTexturePath).As<TextureAsset>();

			const std::string normalTexturePath = GetTexturePath(aiMaterial, meshNode->Directory, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS);
			Ref<TextureAsset> normalTexture = AssetRegistry::Instance().LoadAsset(normalTexturePath).As<TextureAsset>();

			aiColor4D colour(1.0f);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &colour);

			float opacity(1.0f);
			aiGetMaterialFloat(aiMaterial, AI_MATKEY_OPACITY, &opacity);

			Ref<MaterialAsset> material = ::New<MaterialAsset>(meshNode->AssetInfo);
			material->SetName(materialname);

			material->SetTexture(TextureAssetTypes::Diffuse, diffuseTexture);
			material->SetTexture(TextureAssetTypes::Normal, normalTexture);

			material->SetProperty(MaterialAssetProperty::Colour_R, colour.r);
			material->SetProperty(MaterialAssetProperty::Colour_G, colour.g);
			material->SetProperty(MaterialAssetProperty::Colour_B, colour.b);
			material->SetProperty(MaterialAssetProperty::Colour_A, colour.a);

			material->SetProperty(MaterialAssetProperty::Opacity, opacity);

			std::lock_guard lock(meshNode->MaterialCacheLock);
			(*meshNode->MaterialCache)[aiMaterial] = material;
			return material;
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
				IS_LOG_CORE_ERROR("[ModelImporter::GetTexturePath] Unable to find PRB/Legacy textures for material '{}'.", aiMaterial->GetName().C_Str());
				return std::string();
			}

			aiString texturePath;
			aiMaterial->GetTexture(textureType, 0, &texturePath);
			if (texturePath.length == 0)
			{
				IS_LOG_CORE_ERROR("[ModelImporter::GetTexturePath] Texture path for texture type '{}' was empty.", aiTextureTypeToString(textureType));
				return std::string();
			}

			return std::string(directory) + "/" + texturePath.C_Str();
		}

		glm::mat4 ModelImporter::AssimpToGLMMat4(const aiMatrix4x4& transform) const
		{
			return glm::mat4
			(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);
		}

		Maths::Matrix4 ModelImporter::AssimpToInsightMatrix4(const aiMatrix4x4& transform) const
		{
			return Maths::Matrix4
			(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);
		}
	}
}