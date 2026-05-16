#include "Asset/Importers/ModelImporter.h"
#include "Asset/Importers/TextureImporter.h"
#include "Asset/AssetRegistry.h"
#include "Asset/Assets/Model.h"
#include "Asset/Assets/Texture.h"

#include "Resource/Mesh.h"

#include "Graphics/RenderContext.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Platforms/Platform.h"
#include "Algorithm/Vector.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assimp/mesh.h>
//#include <assimp/DefaultIOSystem.h>
#include <assimp/IOStream.hpp>
#include <assimp/ProgressHandler.hpp>

#include <meshoptimizer.h>
#include <unordered_set>
#include <iterator>

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

		class ConsoleAssimpProgressHandler : public Assimp::ProgressHandler
		{
		public:
			ConsoleAssimpProgressHandler() = delete;
			ConsoleAssimpProgressHandler(std::string fileName)
				: FileName(std::move(fileName))
			{ }

			virtual bool Update(float percentage = -1.f) override
			{
				IS_LOG_CORE_INFO("Loading Model '{}', progrss: '{}'", FileName, percentage * 100.0f);
				return true;
			}

			std::string FileName;
		};

		//=============================================
		// CustomAssimpIOSystem
		//=============================================
		//class CustomAssimpIOSystem : public Assimp::DefaultIOSystem
		//{
		//	/*
		//		This must use the C++ new and delete and not Insight's own New and Delete
		//		as some loaders like gtlf uses shared_ptrs. Because of this if we use our own New
		//		then we never call our own Delete so Insight doesn't know that a pointer has been deleted.
		//	*/
		//public:
		//	virtual bool Exists(const char* pFile) const override
			//{
			//	return AssetRegistry::Instance().GetAssetInfo(pFile) != nullptr;
			//}
		//
		//	virtual char getOsSeparator() const override
			//{
			//	return '\\';
			//}
		//
		//	virtual Assimp::IOStream* Open(const char* pFile,
		//		const char* pMode = "rb") override
			//{
			//	const AssetInfo* assetInfo = AssetRegistry::Instance().GetAssetInfo(pFile);
			//	Assimp::IOStream* stream = new CustomAssimpIOStrean(pFile, assetInfo);
			//	Core::MemoryTracker::Instance().NameAllocation(stream, assetInfo->GetFullFilePath().c_str());
			//	return stream;
			//}
		//
		//	virtual void Close(Assimp::IOStream* pFile) override
			//{
			//	delete pFile;
			//}
		//
		//};

		//=============================================
		// MeshNode
		//=============================================
		MeshNode::~MeshNode()
		{
			Delete(MeshData);
			Delete(Mesh);
		}

		//=============================================
		// MeshData
		//=============================================
		void MeshData::Optimise()
		{
			IS_PROFILE_FUNCTION();

			ASSERT(LODs.size() == 1);

			const u32 vertexCount = LODs[0].Vertex_count;
			const u32 indexCount = LODs[0].Index_count;
#if VERTEX_SPLIT_STREAMS
			const u32 vertexSize = Vertices.GetStride(Graphics::Vertices::Stream::Position);
#else
			const u32 vertexSize = Vertices.GetStride(Graphics::Vertices::Stream::Interleaved);
#endif

			std::vector<u32> remap(indexCount);
			const u64 optimisedVertexCount = meshopt_generateVertexRemap(remap.data(), 
				Indices.data(),
				indexCount, 
				Vertices.GetData(Graphics::Vertices::Stream::Position),
				vertexCount,
				vertexSize);

			std::vector<u32> optimisedIndices;
			optimisedIndices.resize(indexCount);

			Graphics::Vertices optimisedVertices;
			optimisedVertices.Resize(optimisedVertexCount);

			// Optimisation 1: Remove all duplicate vertices
			meshopt_remapIndexBuffer(optimisedIndices.data(), Indices.data(), indexCount, remap.data());
			meshopt_remapVertexBuffer(optimisedVertices.GetData(Graphics::Vertices::Stream::Position), Vertices.GetData(Graphics::Vertices::Stream::Position), vertexCount, vertexSize, remap.data());

			// Vertex cache optimization - reordering triangles to maximize cache locality
			IS_LOG_INFO("Optimizing vertex cache...");
			meshopt_optimizeVertexCache(optimisedIndices.data(), optimisedIndices.data(), indexCount, optimisedVertexCount);

			// Overdraw optimizations - reorders triangles to minimize overdraw from all directions
			IS_LOG_INFO("Optimizing overdraw...");
			meshopt_optimizeOverdraw(optimisedIndices.data(), optimisedIndices.data(), indexCount, (float*)Vertices.GetData(Graphics::Vertices::Stream::Position), optimisedVertexCount, vertexSize, 1.05f);

			// Vertex fetch optimization - reorders triangles to maximize memory access locality
			IS_LOG_INFO("Optimizing vertex fetch...");
			meshopt_optimizeVertexFetch(optimisedVertices.GetData(Graphics::Vertices::Stream::Position)
				, optimisedIndices.data()
				, indexCount
				, optimisedVertices.GetData(Graphics::Vertices::Stream::Position)
				, optimisedVertexCount, vertexSize);
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

#if VERTEX_SPLIT_STREAMS
				const u32 vertexSize = Vertices.GetStride(Graphics::Vertices::Stream::Position);
#else
				const u32 vertexSize = Vertices.GetStride(Graphics::Vertices::Stream::Interleaved);
#endif

				// Try and simplify the mesh, try and preserve mesh topology.
				u64 resultIndexCount = meshopt_simplify(
					result_lod.data()
					, &*indices_begin
					, static_cast<u64>(LOD0_IndicesCount)
#if VERTEX_SPLIT_STREAMS
					, (float*)Vertices.GetData(Graphics::Vertices::Stream::Position) + (LOD0_VertexStart * vertexSize)
#else
					, (float*)Vertices.GetData(Graphics::Vertices::Stream::Interleaved) + (LOD0_VertexStart * vertexSize)
#endif
					, LOD0_VertexCount
					, vertexSize
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
#if VERTEX_SPLIT_STREAMS
						, (float*)Vertices.GetData(Graphics::Vertices::Stream::Position) + (LOD0_VertexStart * vertexSize)
#else
						, (float*)Vertices.GetData(Graphics::Vertices::Stream::Interleaved) + (LOD0_VertexStart * vertexSize)
#endif
						, LOD0_VertexCount
						, vertexSize
						, target_index_count
						, error_rate);
				}
				result_lod.resize(resultIndexCount);

				const u32 First_index = static_cast<u32>(Indices.size());
				const u32 Index_count = static_cast<u32>(resultIndexCount);
				LODs.push_back(MeshData::LOD(lodIdx, 0, static_cast<u32>(LOD0_VertexCount), First_index, Index_count));

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
					//".fbx",
					".gltf",
					".dae",
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
			IS_PROFILE_SCOPE_TEXT("Model name: '%s'", assetInfo->FileName.c_str());

			MaterialCache.clear();

			Ref<ModelAsset> modelAsset = asset.As<ModelAsset>();
			modelAsset.Ptr()->m_assetState = AssetState::Loading;
			//modelAsset->SetName(scene->mName.C_Str());

#if EXP_MODEL_LOADING
			Assimp::Importer importer;
			// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
			//importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
			
			if (Mesh::kMeshIndexType == Graphics::IndexType::Uint16)
			{
				importer.SetPropertyInteger(AI_CONFIG_PP_SLM_VERTEX_LIMIT, _UI16_MAX); //65535
				importer.SetPropertyInteger(AI_CONFIG_PP_SLM_TRIANGLE_LIMIT, _UI16_MAX);
			}

			ConsoleAssimpProgressHandler progressHandler(assetInfo->FileName);
			importer.SetProgressHandler(&progressHandler);

			//CustomAssimpIOSystem ioSystem;
			//importer.SetIOHandler(&ioSystem);

			const uint32_t importerFlags =
				0
				// Switch to engine conventions
				// Validate and clean up
				| aiProcess_ValidateDataStructure			/// Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct
				| aiProcess_PopulateArmatureData
				| aiProcess_Triangulate						/// Triangulates all faces of all meshes
				//| aiProcess_SortByPType					/// Splits meshes with more than one primitive type in homogeneous sub-meshes.
				| aiProcess_LimitBoneWeights

				| aiProcess_MakeLeftHanded					/// DirectX style.
				| aiProcess_FlipUVs							/// DirectX style.
				| aiProcess_FlipWindingOrder				/// DirectX style.

				| aiProcess_CalcTangentSpace				/// Calculates the tangents and bitangents for the imported meshes.
				| aiProcess_GenSmoothNormals				/// Ignored if the mesh already has normal.
				| aiProcess_GenUVCoords						/// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels.
				
				| aiProcess_GenBoundingBoxes				//

				| aiProcess_SplitLargeMeshes

				//| aiProcess_RemoveRedundantMaterials		/// Searches for redundant/unreferenced materials and removes them
				| aiProcess_JoinIdenticalVertices			/// Triangulates all faces of all meshes
				//| aiProcess_PopulateArmatureData
				//| aiProcess_FindDegenerates				/// Convert degenerate primitives to proper lines or points.
				//| aiProcess_FindInvalidData				/// This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them
				//| aiProcess_FindInstances					/// This step searches for duplicate meshes and replaces them with references to the first mesh
				;

			importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
			importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);

			const aiScene* scene = nullptr;	
			{
				IS_PROFILE_SCOPE("Assimp Import");
				scene = importer.ReadFile(path.data(), importerFlags);
			}
			importer.SetIOHandler(nullptr);
			importer.SetProgressHandler(nullptr);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_LOG_CORE_ERROR("[ModelImporter::Import] Assimp model '{}' load: {}", path.data(), importer.GetErrorString());
				return;
			}
			ExtractSkeleton(scene, scene->mRootNode, Maths::Matrix4::Identity, modelAsset.Ptr());

			const aiNode* rootBone = FindRootBone(scene, scene->mRootNode, modelAsset.Ptr());
			if (rootBone)
			{
				BuildBoneHierarchy(scene, rootBone, Maths::Matrix4::Identity, nullptr, modelAsset.Ptr());
			}

			if (Ref<Skeleton> skeleton = modelAsset->GetSkeleton(0))
			{
				//skeleton->m_globalInverseTransforms = AssimpToInsightMatrix4(scene->mRootNode->mTransformation).Inversed();
			}

			ModelNode modelNode;
			PreProcessVertexAndIndexBuffer(scene, modelNode);

			ProcessNode(scene, scene->mRootNode, modelAsset.Ptr(), modelNode);
			ProcessAnimations(scene, modelAsset.Ptr());
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

		Reflect::Type ModelImporter::GetAssetType() const
		{
			return ModelAsset::GetStaticTypeInfo().GetType();
		}

#if EXP_MODEL_LOADING
		void ModelImporter::PreProcessVertexAndIndexBuffer(const aiScene* aiScene, ModelNode& modelNode) const
		{
			ASSERT(aiScene);

			for (unsigned int i = 0; i < aiScene->mNumMeshes; ++i)
			{
				const aiMesh* mesh = aiScene->mMeshes[i];
				modelNode.VertexSize += mesh->mNumVertices;
				modelNode.IndexSize += mesh->mNumFaces * 3;
			}
#if VERTEX_SPLIT_STREAMS
			Graphics::RHI_Buffer_Overrides vertexOverrides;
			vertexOverrides.AllowUnorderedAccess = true;

			Graphics::Vertices v;
			const auto CreateVertexBuffer = [](Graphics::RHI_Buffer*& buffer, int stride, u64 vertexSize, Graphics::RHI_Buffer_Overrides& overrides)
			{
				if (buffer == nullptr)
				{
					buffer = Renderer::CreateVertexBuffer(stride * vertexSize, stride, overrides);
				}
			};

			CreateVertexBuffer(modelNode.RHI_VertexBuffers.Position, v.GetStride(Graphics::Vertices::Stream::Position), modelNode.VertexSize, vertexOverrides);
			CreateVertexBuffer(modelNode.RHI_VertexBuffers.Normal, v.GetStride(Graphics::Vertices::Stream::Normal), modelNode.VertexSize, vertexOverrides);
			CreateVertexBuffer(modelNode.RHI_VertexBuffers.Colour, v.GetStride(Graphics::Vertices::Stream::Colour), modelNode.VertexSize, vertexOverrides);
			CreateVertexBuffer(modelNode.RHI_VertexBuffers.UV, v.GetStride(Graphics::Vertices::Stream::UV), modelNode.VertexSize, vertexOverrides);
			CreateVertexBuffer(modelNode.RHI_VertexBuffers.BoneIds, v.GetStride(Graphics::Vertices::Stream::BoneId), modelNode.VertexSize, vertexOverrides);
			CreateVertexBuffer(modelNode.RHI_VertexBuffers.BoneWeights, v.GetStride(Graphics::Vertices::Stream::BoneWeight), modelNode.VertexSize, vertexOverrides);
#else
			CreateVertexBuffer(modelNode.RHI_VertexBuffer, v.GetStride(Graphics::Vertices::Stream::All), modelNode.VertexSize, vertexOverrides);
#endif
			auto createIndexBuffer = [](Graphics::RHI_Buffer*& buffer, const u64 sizeInBytes, const std::string name, u64 index)
				{
					if (buffer == nullptr)
					{
						buffer = Renderer::CreateIndexBuffer(sizeInBytes);
						buffer->SetName(name + "_Index_" + std::to_string(index));
					}
				};

			const u64 kLargestIndexBufferSize = Mesh::kMeshIndexType == Graphics::IndexType::Uint32 ? _UI32_MAX : _UI16_MAX;
			const u32 kIndexTypeSize = Mesh::kMeshIndexType == Graphics::IndexType::Uint32 ? sizeof(u32) : sizeof(u16);

			u32 indexBufferIndex = 0;
			u64 indexBufferCount = modelNode.IndexSize;

			while (indexBufferCount > 0)
			{
				modelNode.RHI_IndexBuffers.push_back(nullptr);
				Graphics::RHI_Buffer*& indexBuffer = modelNode.RHI_IndexBuffers.back();

				const u64 bufferSize = indexBufferCount > kLargestIndexBufferSize ? kLargestIndexBufferSize : indexBufferCount;
				const u64 indexBufferDataOffset = modelNode.IndexSize - indexBufferCount;

				createIndexBuffer(indexBuffer, bufferSize * kIndexTypeSize, aiScene->mName.C_Str(), indexBufferIndex);
				indexBufferCount -= bufferSize;
			}
		}

		void ModelImporter::ProcessNode(const aiScene* assimpScene, const aiNode* assimpNode, ModelAsset* modelAsset, ModelNode& modelNode) const
		{
			if (modelAsset->GetSkeleton(0))
			{
				modelAsset->GetSkeleton(0)->m_skeletonNodes.push_back(SkeletonNode
					{
						assimpNode->mName.C_Str(),
						AssimpToInsightMatrix4(assimpNode->mTransformation),
						assimpNode->mParent != nullptr ? assimpNode->mParent->mName.C_Str() : "",
						{  },

						_UI32_MAX,
						""
					});

				if (SkeletonNode* skeletonNode = modelAsset->GetSkeleton(0)->GetNode(assimpNode->mName.C_Str()))
				{
					if (!skeletonNode->ParentName.empty())
					{
						SkeletonNode* parentSkeletonNode = modelAsset->GetSkeleton(0)->GetNode(skeletonNode->ParentName);
						parentSkeletonNode->ChildrenNames.push_back(skeletonNode->Name);
					}
				}
			}

			for (size_t meshIdx = 0; meshIdx < assimpNode->mNumMeshes; ++meshIdx)
			{
				ProcessMesh(assimpScene, assimpNode, assimpScene->mMeshes[assimpNode->mMeshes[meshIdx]], modelAsset, modelNode);
			}

			for (size_t childIdx = 0; childIdx < assimpNode->mNumChildren; ++childIdx)
			{
				ProcessNode(assimpScene, assimpNode->mChildren[childIdx], modelAsset, modelNode);
			}
		}

		void ModelImporter::ProcessMesh(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, ModelAsset* modelAsset, ModelNode& modelNode) const
		{
			Mesh* mesh = ::New<Mesh>();
			mesh->m_assetInfo = modelAsset->GetAssetInfo();
			modelAsset->m_meshes.push_back(mesh);

			MeshData meshData = { };
			ParseMeshData(aiScene, aiNode, aiMesh, meshData, modelAsset, modelNode);

			mesh->m_mesh_name = aiMesh->mName.C_Str();
			mesh->m_transform_offset = AssimpToInsightMatrix4(aiNode->mTransformation);
#if VERTEX_SPLIT_STREAMS
			mesh->m_boundingBox = Graphics::BoundingBox((float*)meshData.Vertices.GetData(Graphics::Vertices::Stream::Position)
				, meshData.Vertices.GetStride(Graphics::Vertices::Stream::Position),
				meshData.Vertices.VerticesCount());
#else
			mesh->m_boundingBox = Graphics::BoundingBox((float*)meshData.Vertices.GetData(Graphics::Vertices::Stream::Position)
				, meshData.Vertices.GetStride(Graphics::Vertices::Stream::Interleaved),
				meshData.Vertices.VerticesCount());
#endif

			mesh->m_boundingBox = Graphics::BoundingBox(
				Maths::Vector3(aiMesh->mAABB.mMin.x, aiMesh->mAABB.mMin.y, aiMesh->mAABB.mMin.z),
				Maths::Vector3(aiMesh->mAABB.mMax.x, aiMesh->mAABB.mMax.y, aiMesh->mAABB.mMax.z));

			if (!meshData.Vertices.IsEmpty() && !meshData.Indices.empty())
			{
				//meshData.Optimise();
				//meshData.GenerateLODs();

				ASSERT(mesh);

				const auto UploadVertexData =[](const ModelNode& modelNode, Graphics::RHI_Buffer* buffer, Graphics::RHI_BufferView& bufferView, void* data, u64 stride, u64 verticesCount, u64 offset)
				{
					ASSERT(buffer != nullptr);
					const u64 bufferSize = stride * verticesCount;
					buffer->Upload(data, bufferSize, offset, 0);
					bufferView = buffer;
				};

#if VERTEX_SPLIT_STREAMS
				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.Position
						, modelNode.RHI_VertexBuffers.PositionView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::Position)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::Position)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::Position));

				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.Normal
						, modelNode.RHI_VertexBuffers.NormalView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::Normal)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::Normal)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::Normal));

				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.Colour
						, modelNode.RHI_VertexBuffers.ColourView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::Colour)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::Colour)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::Colour));

				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.UV
						, modelNode.RHI_VertexBuffers.UVView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::UV)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::UV)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::UV));

				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.BoneIds
						, modelNode.RHI_VertexBuffers.BoneIdsView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::BoneId)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::BoneId)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::BoneId));

				UploadVertexData(modelNode
						, modelNode.RHI_VertexBuffers.BoneWeights
						, modelNode.RHI_VertexBuffers.BoneWeightsView
						, meshData.Vertices.GetData(Graphics::Vertices::Stream::BoneWeight)
						, meshData.Vertices.GetStride(Graphics::Vertices::Stream::BoneWeight)
						, meshData.Vertices.VerticesCount()
						, modelNode.VertexOffset * meshData.Vertices.GetStride(Graphics::Vertices::Stream::BoneWeight));
#else
				ASSERT(modelNode.RHI_VertexBuffer != nullptr);
				// TODO: Look into why when using the Sponza model and QueueUpload if the editor camera is in certain positions then the mesh disappears.
				//meshData.RHI_VertexBuffer->QueueUpload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
				modelNode.RHI_VertexBuffer->Upload(meshData.Vertices.GetData(Graphics::Vertices::Stream::Interleaved), modelNode.RHI_VertexBuffer->GetSize());
#endif

				auto UploadIndexData = [](Graphics::RHI_Buffer*& buffer, void* data, u64 sizeInBytes, u64 offset)
					{
						ASSERT(buffer != nullptr);
						buffer->Upload(data, sizeInBytes, offset, 0);
					};

				const u64 kLargestIndexBufferSize = Mesh::kMeshIndexType == Graphics::IndexType::Uint32 ? _UI32_MAX : _UI16_MAX;
				const u32 kIndexTypeSize = Mesh::kMeshIndexType == Graphics::IndexType::Uint32 ? sizeof(u32) : sizeof(u16);

				u32 indexBufferIndex = 0;
				u64 indexBufferCount = meshData.Indices.size();

				while (indexBufferCount > 0)
				{
					Graphics::RHI_Buffer*& indexBuffer = modelNode.RHI_IndexBuffers[indexBufferIndex++];

					const u64 bufferSize = indexBufferCount > kLargestIndexBufferSize ? kLargestIndexBufferSize : indexBufferCount;
					const u64 indexBufferDataOffset = meshData.Indices.size() - indexBufferCount;

					if (Mesh::kMeshIndexType == Graphics::IndexType::Uint32)
					{
						UploadIndexData(indexBuffer, meshData.Indices.data() + indexBufferDataOffset, bufferSize * kIndexTypeSize, modelNode.IndexOffset * kIndexTypeSize);
					}
					else
					{
						std::vector<u16> u16Indices(meshData.Indices.size());
						for (size_t i = 0; i < u16Indices.size(); ++i)
						{
							u16Indices[i] = static_cast<u16>(meshData.Indices[i]);
						}
						UploadIndexData(indexBuffer, u16Indices.data() + indexBufferDataOffset, bufferSize * kIndexTypeSize, modelNode.IndexOffset * kIndexTypeSize);
					}
					indexBufferCount -= bufferSize;
				}

				/*
				if (!meshData.RHI_IndexBuffer)
				{
					meshData.RHI_IndexBuffer = Renderer::CreateIndexBuffer(meshData.Indices.size() * sizeof(u32));
					//meshData.RHI_IndexBuffer->QueueUpload(meshData.Indices.data(), meshData.RHI_IndexBuffer->GetSize());
					meshData.RHI_IndexBuffer->Upload(meshData.Indices.data(), meshData.RHI_IndexBuffer->GetSize());
					meshData.RHI_IndexBuffer->SetName(std::string(aiNode->mName.C_Str()) + "_" + aiMesh->mName.C_Str() + "_Index");
				}
				else
				{
					// We already have a buffer, just upload our data.
					FAIL_ASSERT();
				}
				*/

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

#if VERTEX_SPLIT_STREAMS
					meshLod.VertexBuffers = modelNode.RHI_VertexBuffers;
#else
					meshLod.VertexBuffer = modelNode.RHI_VertexBuffer;
					meshLod.VertexBufferView = modelNode.VertexBuffer;
					const std::string vertexBufferName = std::string(aiNode->mName.C_Str()) + "_" + aiMesh->mName.C_Str() + "_Veretx";
					meshLod.VertexBuffer->SetName(vertexBufferName);
#endif

					for (size_t i = 0; i < modelNode.RHI_IndexBuffers.size(); ++i)
					{
						meshLod.IndexBuffers.push_back(modelNode.RHI_IndexBuffers[i]);
						meshLod.IndexBufferViews.push_back(modelNode.RHI_IndexBuffers[i]);
					}
				}
			}

			if (aiScene->HasMaterials())
			{
				const aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
				Ref<MaterialAsset> material = ProcessMaterial(aiScene, aiNode, aiMaterial, modelAsset);

				modelAsset->m_materials.push_back(material);
				mesh->SetMaterial(material);
			}

			modelNode.VertexOffset += meshData.Vertices.VerticesCount();
			modelNode.IndexOffset += meshData.Indices.size();
		}

		void ModelImporter::ParseMeshData(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, MeshData& meshData, ModelAsset* modelAsset, ModelNode& modelNode) const
		{
			IS_PROFILE_FUNCTION();

			//meshData.Vertices.reserve(meshData.Vertices.size() + static_cast<u64>(aiMesh->mNumVertices));
			/// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
			{
				IS_PROFILE_SCOPE("Add Vertex");

				Maths::Vector4 position; /// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
				/// so we transfer the data to this placeholder Maths::Vector3 first.
				/// positions
				position.x = aiMesh->mVertices[i].x;
				position.y = aiMesh->mVertices[i].y;
				position.z = aiMesh->mVertices[i].z;
				position.w = 1.0f;
				//vertex.Position += Maths::Vector3(meshData.TransformOffset[3].xyz);

				Maths::Vector4 normal;
				/// Normals
				if (aiMesh->HasNormals())
				{
					normal.x = aiMesh->mNormals[i].x;
					normal.y = aiMesh->mNormals[i].y;
					normal.z = aiMesh->mNormals[i].z;
					normal.w = 1.0f;
				}

				Maths::Vector4 colour;
				if (aiMesh->mColors[0])
				{
					colour.x = aiMesh->mColors[0]->r;
					colour.y = aiMesh->mColors[0]->g;
					colour.z = aiMesh->mColors[0]->b;
					colour.w = aiMesh->mColors[0]->a;
				}
				else
				{
					colour.x = (rand() % 100 + 1) * 0.01f;
					colour.y = (rand() % 100 + 1) * 0.01f;
					colour.z = (rand() % 100 + 1) * 0.01f;
					colour.w = 1.0f;
				}

				Maths::Vector2 uv;
				/// texture coordinates
				if (aiMesh->mTextureCoords[0]) /// does the mesh contain texture coordinates?
				{
					Maths::Vector2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					uv.x = aiMesh->mTextureCoords[0][i].x;
					uv.y = aiMesh->mTextureCoords[0][i].y;

#ifdef VERTEX_DISABLE_TILE_UVS
					uv.x = uv.x > 1.0f ? uv.x - ((int)uv.x) : uv.x;
					uv.x = uv.x < 0.0f ? uv.x + (std::abs((int)uv.x)) : uv.x;

					uv.y = uv.y > 1.0f ? uv.y - ((int)uv.y) : uv.y;
					uv.y = uv.y < 0.0f ? uv.y + (std::abs((int)uv.y)) : uv.y;
#endif
				}
				else
				{
					uv = 0.0f;
				}

				//Graphics::VertexOptomised vertexOptomised(vertex);
				meshData.Vertices.AddVertex(position, normal, colour, uv, 0, Maths::Vector4::Zero);
				meshData.VerticesBoneInfluence.push_back(Graphics::VertexBoneInfluence());
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

			ExtractBoneWeights(aiScene, aiNode, aiMesh, &meshData, modelAsset);

			meshData.LODs.push_back(
				MeshData::LOD(
					0,
					static_cast<u32>(modelNode.VertexOffset),
					static_cast<u32>(meshData.Vertices.VerticesCount()),
					static_cast<u32>(modelNode.IndexOffset),
					static_cast<u32>(meshData.Indices.size())));
		}

		Ref<MaterialAsset> ModelImporter::ProcessMaterial(const aiScene* aiScene, const aiNode* aiNode, const aiMaterial* aiMaterial, ModelAsset* modelAsset) const
		{
			IS_PROFILE_FUNCTION();

			if (auto iter = MaterialCache.find(aiMaterial);
				iter != MaterialCache.end())
			{
				return iter->second;
			}
			ASSERT_MSG(aiMaterial, "[ModelImporter::ProcessMaterial] AssimpMaterial from MeshNode was nullptr. This shouldn't happen.");
			
			Ref<MaterialAsset> material = ::New<MaterialAsset>(modelAsset->GetAssetInfo());

			const std::string materialname = aiMaterial->GetName().C_Str();
			const std::string_view Directory = modelAsset->GetAssetInfo()->FilePath;

#define AYNSC_TEXTURE_LOAD 1
#if AYNSC_TEXTURE_LOAD
			std::unordered_map<TextureAssetTypes, Ref<AssetAsyncRequest>> loadedTexturesAsync;
			loadedTexturesAsync[TextureAssetTypes::Diffuse] = LoadTextureAsync(aiScene, aiMaterial, aiTextureType::aiTextureType_BASE_COLOR, aiTextureType::aiTextureType_DIFFUSE, modelAsset);
			loadedTexturesAsync[TextureAssetTypes::Normal] = LoadTextureAsync(aiScene, aiMaterial, aiTextureType::aiTextureType_NORMAL_CAMERA, aiTextureType::aiTextureType_NORMALS, modelAsset);
#endif

			aiColor4D colour(1.0f);
			aiGetMaterialColor(aiMaterial, AI_MATKEY_COLOR_DIFFUSE, &colour);

			float opacity(1.0f);
			aiGetMaterialFloat(aiMaterial, AI_MATKEY_OPACITY, &opacity);

			material->SetName(materialname);

			material->SetProperty(MaterialAssetProperty::Colour_R, colour.r);
			material->SetProperty(MaterialAssetProperty::Colour_G, colour.g);
			material->SetProperty(MaterialAssetProperty::Colour_B, colour.b);
			material->SetProperty(MaterialAssetProperty::Colour_A, colour.a);

			material->SetProperty(MaterialAssetProperty::Opacity, opacity);


			const auto Load_Texture = [&](const TextureAssetTypes textureType, const aiTextureType legacyType, const aiTextureType pbrType)
				{
#if AYNSC_TEXTURE_LOAD
					loadedTexturesAsync[textureType]->Wait();
					if (nullptr != loadedTexturesAsync[textureType]->GetAsset())
#else
#endif
					{
						material->SetTexture(textureType, LoadTexture(aiScene, aiMaterial, legacyType, pbrType, modelAsset));
					}
				};

			Load_Texture(TextureAssetTypes::Diffuse, aiTextureType::aiTextureType_BASE_COLOR, aiTextureType::aiTextureType_DIFFUSE);
			Load_Texture(TextureAssetTypes::Normal, aiTextureType::aiTextureType_NORMAL_CAMERA, aiTextureType::aiTextureType_NORMALS);

#undef AYNSC_TEXTURE_LOAD

			MaterialCache[aiMaterial] = material;
			return material;
		}

		Ref<TextureAsset> ModelImporter::LoadTexture(const aiScene* assimpScene, const aiMaterial* assimpMaterial, const aiTextureType PBRType, const aiTextureType legacyType, ModelAsset* modelAsset) const
		{
			const std::string texturePath = GetTexturePath(assimpMaterial, modelAsset->GetAssetInfo()->FilePath, PBRType, legacyType);
			Ref<TextureAsset> texture = LoadEmbeddedTexture(assimpScene, assimpMaterial, PBRType, legacyType, modelAsset);
			if (!texture)
			{
				texture = AssetRegistry::Instance().LoadAsset(texturePath).As<TextureAsset>();
			}
			return texture.As<TextureAsset>();
		}

		Ref<AssetAsyncRequest> ModelImporter::LoadTextureAsync(const aiScene* assimpScene, const aiMaterial* assimpMaterial, const aiTextureType PBRType, const aiTextureType legacyType, ModelAsset* modelAsset) const
		{
			const std::string texturePath = GetTexturePath(assimpMaterial, modelAsset->GetAssetInfo()->FilePath, PBRType, legacyType);
			Ref<TextureAsset> texture = LoadEmbeddedTexture(assimpScene, assimpMaterial, PBRType, legacyType, modelAsset);
			if (!texture)
			{
				return AssetRegistry::Instance().LoadAssetAsync(texturePath);
			}
			return Ref<AssetAsyncRequest>(::New<AssetAsyncRequest>(texture.As<TextureAsset>(), true));
		}

		Ref<TextureAsset> ModelImporter::LoadEmbeddedTexture(const aiScene* assimpScene, const aiMaterial* assimpMaterial, const aiTextureType PBRType, const aiTextureType legacyType, ModelAsset* modelAsset) const
		{
			const std::string texturePath = GetTexturePath(assimpMaterial, modelAsset->GetAssetInfo()->FilePath, PBRType, legacyType);
			Ref<TextureAsset> texture;

			const aiTexture* embededTexture = assimpScene->GetEmbeddedTexture(texturePath.c_str());
			if (embededTexture != nullptr)
			{
				texture = ::New<TextureAsset>(modelAsset->GetAssetInfo());

				const bool isCompressed = embededTexture->mHeight == 0;
				const u64 dataSize = isCompressed ? embededTexture->mWidth : embededTexture->mWidth * embededTexture->mHeight;

				TextureImporter textureImporter;
				if (!textureImporter.IsValidImporterForFileExtension(embededTexture->achFormatHint))
				{
					IS_LOG_CORE_ERROR("[ModelImporter::LoadTexture] Unable to load texture format '{}', path '{}'.", embededTexture->achFormatHint, texturePath);
					return nullptr;
				}
				textureImporter.ImportFromMemory(texture.Ptr(), embededTexture->pcData, dataSize);
				modelAsset->m_embeddedTextures.push_back(texture);
			}

			return texture;
		}

		void ModelImporter::ExtractSkeleton(const aiScene* aiScene, const aiNode* aiNode, Maths::Matrix4 parentTransform, ModelAsset* modelAsset) const
		{
			bool hasBones = false;
			for (size_t meshIdx = 0; meshIdx < aiNode->mNumMeshes; ++meshIdx)
			{

				const aiMesh* aiMesh = aiScene->mMeshes[meshIdx];
				
				if (aiMesh->HasBones() && !modelAsset->GetSkeleton(0))
				{
					const aiBone* bone = aiMesh->mBones[0];
					modelAsset->m_skeletons.push_back(Ref<Skeleton>(::New<Skeleton>(modelAsset->GetAssetInfo(), bone->mName.C_Str())));
				}

				Ref<Skeleton> skeleton = aiMesh->HasBones() ? modelAsset->GetSkeleton(0) : nullptr;

				for (size_t boneIdx = 0; boneIdx < aiMesh->mNumBones; ++boneIdx)
				{
					hasBones = true;
					const aiBone* aiBone = aiMesh->mBones[boneIdx];
					std::string boneName = aiBone->mName.C_Str();
					SkeletonBone newBone(skeleton->GetNumberOfBones(), boneName, parentTransform * AssimpToInsightMatrix4(aiBone->mOffsetMatrix));
					skeleton->AddBone(newBone);
					parentTransform = Maths::Matrix4::Identity;
				}
			}

			if (!hasBones)
			{
				parentTransform = parentTransform * AssimpToInsightMatrix4(aiNode->mTransformation);
			}

			for (size_t i = 0; i < aiNode->mNumChildren; ++i)
			{
				ExtractSkeleton(aiScene, aiNode->mChildren[i], parentTransform, modelAsset);
			}
		}

		const aiNode* ModelImporter::FindRootBone(const aiScene* aiScene, const aiNode* node, ModelAsset* modelAsset) const
		{
			Ref<Skeleton> skeleton = modelAsset->m_skeletons.size() > 0 ? modelAsset->GetSkeleton(0) : nullptr;

			if (skeleton && skeleton->HasBone(node->mName.C_Str()))
			{
				const aiNode* currentNode = node;
				const aiNode* parentNode = node->mParent;
				while (skeleton && skeleton->HasBone(parentNode->mName.C_Str()))
				{
					currentNode = parentNode;
					parentNode = parentNode->mParent;
				}

				skeleton->m_rootBoneIdx = skeleton->GetBone(currentNode->mName.C_Str()).Id;
				ASSERT(skeleton->m_rootBoneIdx < _UI32_MAX);
				return currentNode;
			}

			for (size_t i = 0; i < node->mNumChildren; ++i)
			{
				const aiNode* rootNode = FindRootBone(aiScene, node->mChildren[i], modelAsset);
				if (rootNode != nullptr)
				{
					return rootNode;
				}
			}

			return nullptr;
		}

		void ModelImporter::BuildBoneHierarchy(const aiScene* aiScene, const aiNode* bone, Maths::Matrix4 transform, SkeletonBone* parentBone, ModelAsset* modelAsset) const
		{
			Ref<Skeleton> skeleton = modelAsset->m_skeletons.size() > 0 ? modelAsset->GetSkeleton(0) : nullptr;

			if (skeleton)
			{
				SkeletonBone& skeletonBone = skeleton->GetBone(bone->mName.C_Str());
				if (skeletonBone)
				{
					skeletonBone.ParentTransform = transform;
					//skeletonBone.Offset = transform * skeletonBone.Offset;
					transform = Maths::Matrix4::Identity;

					if (parentBone)
					{
						parentBone->ChildrenBoneIds.push_back(skeletonBone.Id);
						skeletonBone.ParentBoneId = parentBone->Id;
					}
				}
				else
				{
					transform = transform * AssimpToInsightMatrix4(bone->mTransformation);
				}

				for (size_t i = 0; i < bone->mNumChildren; ++i)
				{
					BuildBoneHierarchy(aiScene, bone->mChildren[i], transform, skeletonBone.IsValid() ? &skeletonBone : parentBone, modelAsset);
				}
			}
		}

		void ModelImporter::ExtractBoneWeights(const aiScene* aiScene, const aiNode* aiNode, const aiMesh* aiMesh, MeshData* meshData, ModelAsset* modelAsset) const
		{
			IS_PROFILE_FUNCTION();

			if (aiMesh->HasBones() && !modelAsset->GetSkeleton(0))
			{
				const aiBone* bone = aiMesh->mBones[0];
				modelAsset->m_skeletons.push_back(Ref<Skeleton>(::New<Skeleton>(modelAsset->GetAssetInfo(), bone->mName.C_Str())));
			}

			Ref<Skeleton> skeleton = aiMesh->HasBones() ? modelAsset->GetSkeleton(0) : nullptr;

			for (size_t boneIdx = 0; boneIdx < aiMesh->mNumBones; ++boneIdx)
			{
				int boneId = -1;
				const aiBone* aiBone = aiMesh->mBones[boneIdx];
				std::string boneName = aiBone->mName.C_Str();
				const SkeletonBone& skeletonBone = skeleton->GetBone(boneName);

				if (!skeletonBone.IsValid())
				{
					//FAIL_ASSERT_MSG("[ModelImporter::ExtractBoneWeights] All skeleton/bone data should have already been found.");
					SkeletonBone newBone(skeleton->GetNumberOfBones(), boneName, AssimpToInsightMatrix4(aiBone->mOffsetMatrix));
					skeleton->AddBone(newBone);
					boneId = newBone.Id;


					if (SkeletonNode* skeletonNode = skeleton->GetNode(boneName))
					{
						skeletonNode->BoneName = boneName;
						skeletonNode->BoneId = boneId;
					}
				}
				else
				{
					boneId = skeletonBone.Id;
				}
				ASSERT(boneId != -1);
				
				auto weights = aiMesh->mBones[boneIdx]->mWeights;
				int numWeights = aiMesh->mBones[boneIdx]->mNumWeights;

				for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
				{
					int vertexId = weights[weightIndex].mVertexId;
					float weight = weights[weightIndex].mWeight;
					assert(vertexId <= meshData->Vertices.VerticesCount());
					SetVertexBoneData(meshData->Vertices, vertexId, meshData->VerticesBoneInfluence[vertexId], boneId, weight);
				}
				/*
				const aiVertexWeight* aiBonewights = aiBone->mWeights;
				// clamp the number of weights to evaluate between our max number we allow and the numer on the model.
				const u32 numWeights = aiBone->mNumWeights;

				for (size_t weightIdx = 0; weightIdx < numWeights; ++weightIdx)
				{
					const u32 vertexId = aiBonewights[weightIdx].mVertexId;
					const float weight = aiBonewights[weightIdx].mWeight;

					SetVertexBoneData(meshData->Vertices[vertexId], boneId, weight);
				}
				*/
			}
		}

		void ModelImporter::ProcessAnimations(const aiScene* aiScene, ModelAsset* modelAsset) const
		{
			IS_PROFILE_FUNCTION();

			if (!aiScene->HasAnimations())
			{
				return;
			}
#if ANIMATION_NODE_TRANSFORMS
			AnimationNode rootNode;
			ReadHierarchyData(rootNode, aiScene->mRootNode);
			for (size_t animIdx = 0; animIdx < aiScene->mNumAnimations; ++animIdx)
			{
				ReadMissingBones(aiScene->mAnimations[animIdx], modelAsset);
			}
			modelAsset->m_animationClips[0]->m_rootNode = rootNode;
#else
			Ref<Skeleton> skeleton = modelAsset->GetSkeleton(0);
			ASSERT(skeleton);

			for (size_t animIdx = 0; animIdx < aiScene->mNumAnimations; ++animIdx)
			{
				const aiAnimation* aiAnimation = aiScene->mAnimations[animIdx];

				Ref<AnimationClip> animationClip = Ref<AnimationClip>(::New<AnimationClip>(modelAsset->GetAssetInfo(), skeleton, aiAnimation->mName.C_Str()));
				modelAsset->m_animationClips.push_back(animationClip);

				for (size_t animChannelIdx = 0; animChannelIdx < aiAnimation->mNumChannels; ++animChannelIdx)
				{
					auto channel = aiAnimation->mChannels[animChannelIdx];
					std::string boneName = channel->mNodeName.data;

					if (!skeleton->GetBone(boneName))
					{
						SkeletonBone newBone(skeleton->GetNumberOfBones(), channel->mNodeName.C_Str(), Maths::Matrix4::Identity);
						//skeleton->AddBone(newBone);
					}
				}

				animationClip->m_duration = aiAnimation->mDuration;
				animationClip->m_ticksPerSecond = aiAnimation->mTicksPerSecond;

				for (size_t animChannelIdx = 0; animChannelIdx < aiAnimation->mNumChannels; ++animChannelIdx)
				{
					const aiNodeAnim* aiChannelAnim = aiAnimation->mChannels[animChannelIdx];

					std::vector<AnimationBoneTrack::PositionKeyFrame> positions;
					std::vector<AnimationBoneTrack::RotationKeyFrame> rotations;
					std::vector<AnimationBoneTrack::ScaleKeyFrame> scales;

					positions.reserve(aiChannelAnim->mNumPositionKeys);
					rotations.reserve(aiChannelAnim->mNumRotationKeys);
					scales.reserve(aiChannelAnim->mNumScalingKeys);

					for (size_t posIdx = 0; posIdx < aiChannelAnim->mNumPositionKeys; ++posIdx)
					{
						const aiVectorKey aiPosition = aiChannelAnim->mPositionKeys[posIdx];
						positions.push_back(AnimationBoneTrack::PositionKeyFrame(AssimpToInsightVector3(aiPosition.mValue), aiPosition.mTime));
					}

					for (size_t rotIdx = 0; rotIdx < aiChannelAnim->mNumRotationKeys; ++rotIdx)
					{
						const aiQuatKey aiRotation = aiChannelAnim->mRotationKeys[rotIdx];
						rotations.push_back(AnimationBoneTrack::RotationKeyFrame(AssimpToInsightQuaternion(aiRotation.mValue), aiRotation.mTime));
					}

					for (size_t scaleIdx = 0; scaleIdx < aiChannelAnim->mNumScalingKeys; ++scaleIdx)
					{
						const aiVectorKey aiScale = aiChannelAnim->mScalingKeys[scaleIdx];
						scales.push_back(AnimationBoneTrack::ScaleKeyFrame(AssimpToInsightVector3(aiScale.mValue), aiScale.mTime));
					}

					const SkeletonBone& bone = skeleton->GetBone(aiChannelAnim->mNodeName.C_Str());
					if (bone)
					{
						AnimationBoneTrack boneTrack(bone.Id, aiChannelAnim->mNodeName.C_Str(), positions, rotations, scales);
						animationClip->AddBoneTrack(boneTrack);
					}
					else
					{
						FAIL_ASSERT();

						SkeletonBone newBone(skeleton->GetNumberOfBones(), aiChannelAnim->mNodeName.C_Str(), Maths::Matrix4::Identity);
						skeleton->AddBone(newBone);

						AnimationBoneTrack boneTrack(skeleton->GetNumberOfBones(), aiChannelAnim->mNodeName.C_Str(), positions, rotations, scales);
						animationClip->AddBoneTrack(boneTrack);
					}
				}
			}
#endif
		}

#if ANIMATION_NODE_TRANSFORMS
		void ModelImporter::ReadHierarchyData(AnimationNode& node, const aiNode* assimpNode) const
		{
			node.Name = assimpNode->mName.data;
			node.Transform = AssimpToInsightMatrix4(assimpNode->mTransformation);
			node.ChildrenCount = assimpNode->mNumChildren;

			for (int i = 0; i < assimpNode->mNumChildren; i++)
			{
				AnimationNode newData;
				ReadHierarchyData(newData, assimpNode->mChildren[i]);
				node.Children.push_back(newData);
			}
		}

		void ModelImporter::ReadMissingBones(const aiAnimation* aiAnimation, ModelAsset* modelAsset) const
		{
			auto boneMap = modelAsset->GetSkeleton(0)->m_boneMaps;
			auto boneCount = modelAsset->GetSkeleton(0)->GetNumberOfBones();

			Ref<AnimationClip> animationClip = Ref<AnimationClip>(::New<AnimationClip>());
			modelAsset->m_animationClips.push_back(animationClip);

			animationClip->m_duration = aiAnimation->mDuration;
			animationClip->m_ticksPerSecond = aiAnimation->mTicksPerSecond;

			for (size_t animChannelIdx = 0; animChannelIdx < aiAnimation->mNumChannels; ++animChannelIdx)
			{
				const aiNodeAnim* aiChannelAnim = aiAnimation->mChannels[animChannelIdx];
				std::string boneName = aiChannelAnim->mNodeName.data;

				std::vector<AnimationBoneTrack::PositionKeyFrame> positions;
				std::vector<AnimationBoneTrack::RotationKeyFrame> rotations;
				std::vector<AnimationBoneTrack::ScaleKeyFrame> scales;

				positions.reserve(aiChannelAnim->mNumPositionKeys);
				rotations.reserve(aiChannelAnim->mNumRotationKeys);
				scales.reserve(aiChannelAnim->mNumScalingKeys);

				for (size_t posIdx = 0; posIdx < aiChannelAnim->mNumPositionKeys; ++posIdx)
				{
					const aiVectorKey aiPosition = aiChannelAnim->mPositionKeys[posIdx];
					positions.push_back(AnimationBoneTrack::PositionKeyFrame(AssimpToInsightVector3(aiPosition.mValue), aiPosition.mTime));
				}

				for (size_t rotIdx = 0; rotIdx < aiChannelAnim->mNumRotationKeys; ++rotIdx)
				{
					const aiQuatKey aiRotation = aiChannelAnim->mRotationKeys[rotIdx];
					rotations.push_back(AnimationBoneTrack::RotationKeyFrame(AssimpToInsightQuaternion(aiRotation.mValue), aiRotation.mTime));
				}

				for (size_t scaleIdx = 0; scaleIdx < aiChannelAnim->mNumScalingKeys; ++scaleIdx)
				{
					const aiVectorKey aiScale = aiChannelAnim->mScalingKeys[scaleIdx];
					scales.push_back(AnimationBoneTrack::ScaleKeyFrame(AssimpToInsightVector3(aiScale.mValue), aiScale.mTime));
				}

				if (boneMap.find(boneName) == boneMap.end())
				{
					boneMap[boneName].Id = boneCount;
					++boneCount;

				}
				AnimationBoneTrack boneTrack(boneMap[boneName].Id, aiChannelAnim->mNodeName.C_Str(), positions, rotations, scales);
				animationClip->AddBoneTrack(boneTrack);
			}
			animationClip->m_BoneInfoMap = boneMap;
		}
#endif
#else
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
					meshNode->MeshData->TransformOffset = AssimpToInsightMatrix4(aiNode->mTransformation);

					ProcessMesh(aiScene, aiMesh, meshNode->MeshData, meshNode);
					meshNode->Mesh->m_mesh_name = aiMesh->mName.C_Str();
					meshNode->Mesh->m_transform_offset = AssimpToInsightMatrix4(aiNode->mTransformation);

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
					Graphics::RHI_Buffer_Overrides vertexOverrides;
					vertexOverrides.AllowUnorderedAccess = true;

					meshData->RHI_VertexBuffer = Renderer::CreateVertexBuffer(meshData->Vertices.size() * sizeof(Graphics::Vertex), sizeof(Graphics::Vertex), vertexOverrides);
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

		void ModelImporter::ProcessMesh(const aiScene* aiScene, const aiMesh* aiMesh, MeshData* meshData, MeshNode* meshNode) const
		{
			IS_PROFILE_FUNCTION();

			meshData->Vertices.reserve(meshData->Vertices.size() + static_cast<u64>(aiMesh->mNumVertices));
			/// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
			{
				IS_PROFILE_SCOPE("Add Vertex");

				Maths::Vector4 position; /// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
				/// so we transfer the data to this placeholder Maths::Vector3 first.
				/// positions
				position.x = aiMesh->mVertices[i].x;
				position.y = aiMesh->mVertices[i].y;
				position.z = aiMesh->mVertices[i].z;
				position.w = 1.0f;
				//vertex.Position += Maths::Vector3(meshData.TransformOffset[3].xyz);

				Maths::Vector4 normal;
				/// Normals
				if (aiMesh->HasNormals())
				{
					normal.x = aiMesh->mNormals[i].x;
					normal.y = aiMesh->mNormals[i].y;
					normal.z = aiMesh->mNormals[i].z;
					normal.w = 1.0f;
				}

				Maths::Vector4 colour;
				if (aiMesh->mColors[0])
				{
					colour.x = aiMesh->mColors[0]->r;
					colour.y = aiMesh->mColors[0]->g;
					colour.z = aiMesh->mColors[0]->b;
					colour.w = aiMesh->mColors[0]->a;
				}
				else
				{
					colour.x = (rand() % 100 + 1) * 0.01f;
					colour.y = (rand() % 100 + 1) * 0.01f;
					colour.z = (rand() % 100 + 1) * 0.01f;
					colour.w = 1.0f;
				}

				Maths::Vector2 uv;
				/// texture coordinates
				if (aiMesh->mTextureCoords[0]) /// does the mesh contain texture coordinates?
				{
					Maths::Vector2 vec;
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					uv.x = aiMesh->mTextureCoords[0][i].x;
					uv.y = aiMesh->mTextureCoords[0][i].y;
				}
				else
				{
					FAIL_ASSERT();
					///vertex.UV = Maths::Vector2(0.0f, 0.0f);
				}

				//Graphics::VertexOptomised vertexOptomised(vertex);
				meshData->Vertices.push_back(Graphics::Vertex(position, normal, colour, uv));
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
			IS_PROFILE_SCOPE_TEXT("Material name: '%s'", materialname.c_str());

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
#endif
		/*
		void ModelImporter::ProcessMesh(MeshData& meshData, ModelAsset* modelAsset) const
		{
			Mesh* mesh = ::New<Mesh>();
			modelAsset->m_meshes.push_back(mesh);

			mesh->m_mesh_name = meshData.Name;
			mesh->m_transform_offset = meshData.TransformOffset;
			//mesh->m_boundingBox = Graphics::BoundingBox(meshData.Vertices.data(), static_cast<u32>(meshData.Vertices.size()));
			//mesh->m_boundingBox = Graphics::BoundingBox(
			//	Maths::Vector3(aiMesh->mAABB.mMin.x, aiMesh->mAABB.mMin.y, aiMesh->mAABB.mMin.z),
			//	Maths::Vector3(aiMesh->mAABB.mMax.x, aiMesh->mAABB.mMax.y, aiMesh->mAABB.mMax.z));

			if (!meshData.Vertices.empty() && !meshData.Indices.empty())
			{
				//meshData.Optimise();
				//meshData.GenerateLODs();

				ASSERT(mesh);

#if 0
				if (!meshData.VertexBuffersCreated)
				{
					Graphics::RHI_Buffer_Overrides vertexOverrides;
					vertexOverrides.AllowUnorderedAccess = true;

					meshData.RHI_VertexBuffer = Renderer::CreateVertexBuffer(meshData.Vertices.size() * sizeof(Graphics::Vertex), sizeof(Graphics::Vertex), vertexOverrides);
					// TODO: Look into why when using the Sponza model and QueueUpload if the editor camera is in certain positions then the mesh disappears.
					//meshData.RHI_VertexBuffer->QueueUpload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
					meshData.RHI_VertexBuffer->Upload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}
#else
				if (!meshData.RHI_VertexBuffer)
				{
					Graphics::RHI_Buffer_Overrides vertexOverrides;
					vertexOverrides.AllowUnorderedAccess = true;

					meshData.RHI_VertexBuffer = Renderer::CreateVertexBuffer(meshData.Vertices.size() * sizeof(Graphics::Vertex), sizeof(Graphics::Vertex), vertexOverrides);
					// TODO: Look into why when using the Sponza model and QueueUpload if the editor camera is in certain positions then the mesh disappears.
					//meshData.RHI_VertexBuffer->QueueUpload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
					meshData.RHI_VertexBuffer->Upload(meshData.Vertices.data(), meshData.RHI_VertexBuffer->GetSize());
				}
				else
				{
					// We already have a buffer, just upload out data.
					FAIL_ASSERT();
				}

#endif
				if (!meshData.RHI_IndexBuffer)
				{
					meshData.RHI_IndexBuffer = Renderer::CreateIndexBuffer(meshData.Indices.size() * sizeof(u32));
					//meshData.RHI_IndexBuffer->QueueUpload(meshData.Indices.data(), meshData.RHI_IndexBuffer->GetSize());
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

					const std::string vertexBufferName = mesh->m_mesh_name + "_Veretx";
					const std::string indexBufferName = mesh->m_mesh_name + "_Index";
					meshLod.Vertex_buffer->SetName(vertexBufferName);
					meshLod.Index_buffer->SetName(indexBufferName);
				}
			}
		}
*/
		void ModelImporter::SetVertexBoneData(Graphics::Vertices& vertices, const u64 vertexId, Graphics::VertexBoneInfluence& vertexBoneInfluence, const u32 boneId, const float boneWeight) const
		{
			IS_PROFILE_FUNCTION();

			for (int i = 0; i < Graphics::Vertices::MAX_BONE_COUNT; ++i)
			{
				const float currentBoneWeight = vertices.GetBoneWeight(vertexId, i);
				if (currentBoneWeight == 0.0f)
				{
					ASSERT(boneId < 72);
					vertices.SetBoneId(vertexId, boneId, i);
					vertices.SetBoneWeight(vertexId, boneWeight, i);

					vertexBoneInfluence.SetBoneId(boneId, i);
					vertexBoneInfluence.SetBoneWeight(boneWeight, i);

					break;
				}
			}
		}

		std::string ModelImporter::GetTexturePath(const aiMaterial* aiMaterial, const std::string_view directory, const aiTextureType textureTypePBR, const aiTextureType textureTypeLegacy) const
		{
			aiTextureType textureType = aiTextureType::aiTextureType_NONE;
			if (aiMaterial->GetTextureCount(textureTypePBR) > 0)
			{
				textureType = textureTypePBR;
			}
			else if (aiMaterial->GetTextureCount(textureTypeLegacy) > 0)
			{
				textureType = textureTypeLegacy;
			}

			if (textureType == aiTextureType::aiTextureType_NONE)
			{
				//IS_LOG_CORE_ERROR("[ModelImporter::GetTexturePath] Unable to find PRB/Legacy textures for material '{}'.", aiMaterial->GetName().C_Str());
				return std::string();
			}

			aiString texturePath;
			aiMaterial->GetTexture(textureType, 0, &texturePath);
			if (texturePath.length == 0)
			{
				//IS_LOG_CORE_ERROR("[ModelImporter::GetTexturePath] Texture path for texture type '{}' was empty.", aiTextureTypeToString(textureType));
				return std::string();
			}

			return std::string(directory) + "/" + texturePath.C_Str();
		}

		Maths::Vector3 ModelImporter::AssimpToInsightVector3(const aiVector3D& vector) const
		{
			return Maths::Vector3(vector.x, vector.y, vector.z);
		}

		Maths::Quaternion ModelImporter::AssimpToInsightQuaternion(const aiQuaternion& quaternion) const
		{
			return Maths::Quaternion(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
		}

		Maths::Matrix4 ModelImporter::AssimpToInsightMatrix4(const aiMatrix4x4& transform) const
		{
			constexpr auto flipNegativeZeroFunc = [](const float f)
				{
					if (f == 0.0f && std::signbit(f))
					{
						return -f;
					}
					return f;
				};
			return Maths::Matrix4
			(
				flipNegativeZeroFunc(transform.a1), flipNegativeZeroFunc(transform.b1), flipNegativeZeroFunc(transform.c1), flipNegativeZeroFunc(transform.d1),
				flipNegativeZeroFunc(transform.a2), flipNegativeZeroFunc(transform.b2), flipNegativeZeroFunc(transform.c2), flipNegativeZeroFunc(transform.d2),
				flipNegativeZeroFunc(transform.a3), flipNegativeZeroFunc(transform.b3), flipNegativeZeroFunc(transform.c3), flipNegativeZeroFunc(transform.d3),
				flipNegativeZeroFunc(transform.a4), flipNegativeZeroFunc(transform.b4), flipNegativeZeroFunc(transform.c4), flipNegativeZeroFunc(transform.d4)
			);
		}
	}
}