#include "Graphics/Mesh.h"
#include "Graphics/RenderContext.h"
#include "Graphics/GraphicsManager.h"
#ifdef RENDER_GRAPH_ENABLED
#include "Graphics/RHI/RHI_CommandList.h"
#endif
#include "Core/Logger.h"

#include "Core/Profiler.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

#include "meshoptimizer.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

namespace Insight
{
	namespace Graphics
	{
		static const uint32_t importer_flags =
			/// Switch to engine conventions
			aiProcess_FlipUVs | /// DirectX style.
			aiProcess_MakeLeftHanded | /// DirectX style.
			/// Validate and clean up
			aiProcess_ValidateDataStructure | /// Validates the imported scene data structure. This makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct
			aiProcess_FindDegenerates | /// Convert degenerate primitives to proper lines or points.
			aiProcess_FindInvalidData | /// This step searches all meshes for invalid data, such as zeroed normal vectors or invalid UV coords and removes / fixes them
			aiProcess_RemoveRedundantMaterials | /// Searches for redundant/unreferenced materials and removes them
			aiProcess_Triangulate | /// Triangulates all faces of all meshes
			aiProcess_JoinIdenticalVertices | /// Triangulates all faces of all meshes
			aiProcess_SortByPType | /// Splits meshes with more than one primitive type in homogeneous sub-meshes.
			aiProcess_FindInstances | /// This step searches for duplicate meshes and replaces them with references to the first mesh
			/// Generate missing normals or UVs
			aiProcess_CalcTangentSpace | /// Calculates the tangents and bitangents for the imported meshes
			///aiProcess_GenSmoothNormals | /// Ignored if the mesh already has normals
			aiProcess_GenNormals |
			aiProcess_GenUVCoords;               /// Converts non-UV mappings (such as spherical or cylindrical mapping) to proper texture coordinate channels


		static glm::mat4 ConvertMatrix(const aiMatrix4x4& transform)
		{
			return glm::mat4
			(
				transform.a1, transform.b1, transform.c1, transform.d1,
				transform.a2, transform.b2, transform.c2, transform.d2,
				transform.a3, transform.b3, transform.c3, transform.d3,
				transform.a4, transform.b4, transform.c4, transform.d4
			);
		}

		Submesh::~Submesh()
		{
			Destroy();
		}

#ifdef RENDER_GRAPH_ENABLED
		void Submesh::Draw(RHI_CommandList* cmdList) const
		{
			/// TODO: To be removed when entities are added with components
			glm::mat4 transform = m_draw_info.Transform;
			///transform = glm::scale(transform, glm::vec3(5, 5, 5));
			cmdList->SetPushConstant(0, sizeof(glm::mat4), static_cast<const void*>(glm::value_ptr(transform)));

			cmdList->SetVertexBuffer(m_draw_info.Vertex_Buffer);
			cmdList->SetIndexBuffer(m_draw_info.Index_Buffer, IndexType::Uint32);
			cmdList->DrawIndexed(m_draw_info.Index_Count, 1, m_draw_info.First_Index, m_draw_info.Vertex_Offset, 0);
		}
#endif /// RENDER_GRAPH_ENABLED

		void Submesh::SetDrawInfo(SubmeshDrawInfo info)
		{
			m_draw_info = info;
		}

		void Submesh::Destroy()
		{
			m_draw_info = { };
		}

		int vertexOffset = 0;
		int indexOffset = 0;

		Mesh::Mesh()
		{

		}

		bool Mesh::LoadFromFile(std::string filePath)
		{
			IS_PROFILE_FUNCTION();

			Destroy();

			vertexOffset = 0;
			indexOffset = 0;

			Assimp::Importer importer;
			/// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			/// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

			const aiScene* scene = importer.ReadFile(filePath,
				importer_flags
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
				return false;
			}

			std::vector<Vertex> vertices;
			std::vector<u32> indices;

			ProcessNode(scene->mRootNode, scene, "", vertices, indices);

			const u64 vertex_byte_size = vertices.size() * sizeof(Vertex);
			const u64 index_byte_size = indices.size() * sizeof(u32);

			/// Create our gpu buffers so they can be set for each sub mesh.
			m_vertex_buffer = Renderer::CreateVertexBuffer(vertex_byte_size, sizeof(Vertex));
			m_index_buffer = Renderer::CreateIndexBuffer(index_byte_size);

			/// Set the vertex and index buffers for all our sub meshes. Doing this here
			/// allows us to create the gpu buffers after all the mesh processing has compelted
			/// and only do that once so no need to resize the gpu buffers.
			for (Submesh* sub_mesh : m_submeshes)
			{
				sub_mesh->m_draw_info.Vertex_Buffer = m_vertex_buffer.Get();
				sub_mesh->m_draw_info.Index_Buffer = m_index_buffer.Get();
			}

			m_vertex_buffer->Upload(vertices.data(), vertex_byte_size);
			m_index_buffer->Upload(indices.data(), index_byte_size);

			const u64 file_name_start = filePath.find_last_of('/');
			const u64 file_name_end = filePath.find_last_of('.');
			std::string file_name = filePath.substr(file_name_start, file_name_end - file_name_start);

			std::wstring file_name_w = Platform::WStringFromString(file_name);
			m_vertex_buffer->SetName(file_name_w + L" Vertex_Buffer");
			m_index_buffer->SetName(file_name_w + L" Index_Buffer");

			return true;
		}

		void Mesh::Destroy()
		{
			for (Submesh* submesh : m_submeshes)
			{
				submesh->Destroy();
				DeleteTracked(submesh);
			}

			Renderer::FreeVertexBuffer(*m_vertex_buffer);
			Renderer::FreeIndexBuffer(*m_index_buffer);

			m_vertex_buffer.Release();
			m_index_buffer.Release();

			m_submeshes.clear();
		}

#ifdef RENDER_GRAPH_ENABLED
		void Mesh::Draw(RHI_CommandList* cmdList) const
		{
			IS_PROFILE_FUNCTION();
			for (Submesh* submesh : m_submeshes)
			{
				submesh->Draw(cmdList);
			}
		}
#endif ///RENDER_GRAPH_ENABLED

		void Mesh::CreateGPUBuffers(const aiScene* scene, std::string_view filePath, std::vector<Vertex>& vertices, std::vector<u32>& indices)
		{
			IS_PROFILE_FUNCTION();

			u32 vertex_count = 0;
			u32 index_count = 0;

			std::function<void(aiNode* node, const aiScene* scene)> getVertexAndIndexCount;
			getVertexAndIndexCount = [&getVertexAndIndexCount, &vertex_count, &index_count](aiNode* node, const aiScene* scene)
			{
				/// process all the node's meshes (if any)
				for (unsigned int i = 0; i < node->mNumMeshes; i++)
				{
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					vertex_count += mesh->mNumVertices;
					for (size_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex)
					{
						aiFace face = mesh->mFaces[faceIndex];
						index_count += face.mNumIndices;
					}
				}
				/// then do the same for each of its children
				for (unsigned int i = 0; i < node->mNumChildren; i++)
				{
					getVertexAndIndexCount(node->mChildren[i], scene);
				}
			};
			getVertexAndIndexCount(scene->mRootNode, scene);

			const u64 vertex_byte_size = vertex_count * sizeof(Vertex);
			const u64 index_byte_size = index_count * sizeof(u32);

			vertices.reserve(vertex_count);
			m_vertex_buffer = UPtr(Renderer::CreateVertexBuffer(vertex_byte_size, sizeof(Vertex)));

			indices.reserve(index_count);
			m_index_buffer = UPtr(Renderer::CreateIndexBuffer(index_byte_size));

			std::string_view shortFilePath = filePath.substr(filePath.find_last_of('/') + 1);
			std::wstring wShortFileName;
			std::transform(shortFilePath.begin(), shortFilePath.end(), std::back_inserter(wShortFileName), [](const char c)
				{
					return (wchar_t)c;
				});
		}

		void Mesh::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, std::vector<Vertex>& vertices, std::vector<u32>& indices)
		{
			IS_PROFILE_FUNCTION();

			if (aiNode->mNumMeshes > 0)
			{
				for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];

					std::vector<Vertex> vertices_optomized;
					std::vector<u32> indices_optomized;
					ProcessMesh(aiMesh, aiScene, vertices_optomized, indices_optomized);
					Optimize(vertices_optomized, indices_optomized);

					BoundingBox bounding_box = BoundingBox(vertices_optomized.data(), static_cast<u32>(vertices_optomized.size()));

					SubmeshDrawInfo submesh_draw_info = { };
					submesh_draw_info.Vertex_Offset = static_cast<u32>(vertices.size());
					submesh_draw_info.First_Index = static_cast<u32>(indices.size());

					/// Move our vertices/indices which have been optimized, into the overall vectors.
					std::move(vertices_optomized.begin(), vertices_optomized.end(), std::back_inserter(vertices));
					std::move(indices_optomized.begin(), indices_optomized.end(), std::back_inserter(indices));

					submesh_draw_info.Vertex_Buffer = m_vertex_buffer.Get();
					submesh_draw_info.Index_Buffer = m_index_buffer.Get();

					submesh_draw_info.Transform = ConvertMatrix(aiNode->mTransformation);
					submesh_draw_info.Vertex_Count = static_cast<u32>(vertices.size()) - submesh_draw_info.Vertex_Offset;
					submesh_draw_info.Index_Count = static_cast<u32>(indices.size()) - submesh_draw_info.First_Index;

					Submesh* subMesh = NewArgsTracked(Submesh, this);
					subMesh->SetDrawInfo(submesh_draw_info);
					subMesh->m_bounding_box = bounding_box;
					m_submeshes.push_back(std::move(subMesh));
				}

				///Mesh* mesh = ::New<Mesh, MemoryCategory::Core>(&model, static_cast<u32>(model.m_meshes.size()));
				///Animation::Skeleton* skeleton = ::New<Animation::Skeleton, MemoryCategory::Core>();;
				////// process all the node's meshes (if any)
				///for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				///{
				///	aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
				///	mesh->m_subMeshes.push_back(ProcessMesh(*mesh, aiMesh, aiNode, aiScene, directory));

				///	ExtractSkeleton(*skeleton, mesh->m_vertices, aiMesh, aiScene, mesh);
				///}
				///model.m_meshes.push_back(mesh);
				///model.m_skeletons.push_back(skeleton);

				///if (skeleton->GetBoneCount() > 0)
				///{
				///	model.m_meshToSkeleton.emplace((u32)model.m_meshes.size() - 1, (u32)model.m_skeletons.size() - 1);
				///	model.m_skeletonToMesh.emplace((u32)model.m_skeletons.size() - 1, (u32)model.m_meshes.size() - 1);
				///}
			}

			/// then do the same for each of its children
			for (u32 i = 0; i < aiNode->mNumChildren; i++)
			{
				ProcessNode(aiNode->mChildren[i], aiScene, directory, vertices, indices);
			}
		}

		void Mesh::ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<u32>& indices)
		{
			IS_PROFILE_FUNCTION();

			glm::vec4 vertexColour;
			vertexColour.x = (rand() % 100 + 1) * 0.01f;
			vertexColour.y = (rand() % 100 + 1) * 0.01f;
			vertexColour.z = (rand() % 100 + 1) * 0.01f;
			vertexColour.w = 1.0f;
 
			/// walk through each of the mesh's vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++)
			{
				IS_PROFILE_SCOPE("Add Vertex");

				Vertex vertex = { };
				glm::vec4 vector = { }; /// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
				/// so we transfer the data to this placeholder glm::vec3 first.
				/// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vertex.Position = vector;

				/// normals
				if (mesh->HasNormals())
				{
					vector = { };
					vector.x = mesh->mNormals[i].x;
					vector.y = mesh->mNormals[i].y;
					vector.z = mesh->mNormals[i].z;
					vector.w = 1.0f;
					vector = glm::normalize(vector);
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
				/// texture coordinates
				if (mesh->mTextureCoords[0]) /// does the mesh contain texture coordinates?
				{
					glm::vec2 vec;
					/// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					/// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					///vertex.UV = vec;
					/// tangent
					if (mesh->mTangents)
					{
						vector.x = mesh->mTangents[i].x;
						vector.y = mesh->mTangents[i].y;
						vector.z = mesh->mTangents[i].z;
					}
					///vertex.Tangent = vector;
					/// bitangent
					if (mesh->mBitangents)
					{
						vector.x = mesh->mBitangents[i].x;
						vector.y = mesh->mBitangents[i].y;
						vector.z = mesh->mBitangents[i].z;
					}
					///vertex.Bitangent = vector;
				}
				else
				{
					///vertex.UV = glm::vec2(0.0f, 0.0f);
				}

				vertices.push_back(vertex);
			}

			/// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				/// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; j++)
				{
					IS_PROFILE_SCOPE("Add index");
					indices.push_back(face.mIndices[j]);
				}
			}

			/// process materials
			aiMaterial* material = aiScene->mMaterials[mesh->mMaterialIndex];
			/// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
			/// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
			/// Same applies to other texture as the following list summarizes:
			/// diffuse: texture_diffuseN
			/// specular: texture_specularN
			/// normal: texture_normalN

			/// 1. diffuse maps
			///vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			///textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
			/// 2. specular maps
			///vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			///textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			/// 3. normal maps
			///std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
			///textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
			/// 4. height maps
			///std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
			///textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
		}
		
		void Mesh::Optimize(std::vector<Vertex>& src_vertices, std::vector<u32>& src_indices)
		{
			const u64 vertex_byte_size = src_vertices.size() * sizeof(Vertex);
			const u64 index_byte_size = src_indices.size() * sizeof(u32);

			const u64 vertex_count = vertex_byte_size / sizeof(Vertex);
			const u64 vertex_size = sizeof(Vertex);
			const u64 index_count = index_byte_size / sizeof(u32);

			/// The optimization order is important

			std::vector<u32> remap(index_count); /// allocate temporary memory for the remap table
			size_t total_vertices_optimized = meshopt_generateVertexRemap(remap.data(), src_indices.data(), index_count, src_vertices.data(), index_count, sizeof(Vertex));

			std::vector<Vertex> dst_vertices(total_vertices_optimized);
			std::vector<u32> dst_indices(index_count);

			meshopt_remapIndexBuffer(dst_indices.data(), src_indices.data(), index_count, remap.data());
			meshopt_remapVertexBuffer(dst_vertices.data(), src_vertices.data(), index_count, sizeof(Vertex), remap.data());

			/// Vertex cache optimization - reordering triangles to maximize cache locality
			IS_INFO("Optimizing vertex cache...");
			meshopt_optimizeVertexCache(dst_indices.data(), dst_indices.data(), index_count, total_vertices_optimized);

			/// Overdraw optimizations - reorders triangles to minimize overdraw from all directions
			IS_INFO("Optimizing overdraw...");
			meshopt_optimizeOverdraw(dst_indices.data(), dst_indices.data(), index_count, glm::value_ptr(src_vertices.data()->Position), total_vertices_optimized, vertex_size, 1.05f);

			/// Vertex fetch optimization - reorders triangles to maximize memory access locality
			IS_INFO("Optimizing vertex fetch...");
			meshopt_optimizeVertexFetch(dst_vertices.data(), dst_indices.data(), index_count, dst_vertices.data(), total_vertices_optimized, vertex_size);

			src_vertices = dst_vertices;
			src_indices = dst_indices;
		}
	}
}