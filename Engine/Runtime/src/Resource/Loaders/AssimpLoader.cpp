#include "Resource/Loaders/AssimpLoader.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "Platform/Platform.h"

#include "Graphics/RenderContext.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <meshoptimizer.h>

#include <glm/gtc/type_ptr.hpp>

#include <ppl.h>

namespace Insight
{
	namespace Runtime
	{
		bool AssimpLoader::LoadModel(Model* model, std::string file_path, u32 importer_flags)
		{
			Assimp::Importer importer;
			/// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			/// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

			const aiScene* scene = importer.ReadFile(file_path,
				importer_flags
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
				return false;
			}

			AssimpLoaderData loader_data;
			ProcessNode(scene->mRootNode, scene, "", loader_data);

			return true;
		}

		bool AssimpLoader::LoadMesh(Mesh* mesh, std::string file_path, u32 importer_flags)
		{
			Assimp::Importer importer;
			/// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			/// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

			const aiScene* scene = importer.ReadFile(file_path,
				importer_flags
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
				return false;
			}

			AssimpLoaderData loader_data;
			ProcessNode(scene->mRootNode, scene, "", loader_data, false);

			return true;
		}

		void AssimpLoader::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, AssimpLoaderData& loader_data, bool recursive)
		{
		}

		void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* aiScene, AssimpLoaderData& loader_data)
		{
		}


		void AssimpLoader::ExtractMaterialTextures(aiMaterial* ai_material, aiTextureType ai_texture_type, const char* texture_id, AssimpLoaderData& loader_data)
		{
			IS_PROFILE_FUNCTION();

			for (u32 i = 0; i < ai_material->GetTextureCount(ai_texture_type); ++i)
			{
				aiString str;
				ai_material->GetTexture(ai_texture_type, i, &str);
				std::string file_path = str.C_Str();

				if (file_path.find("./") != std::string::npos)
				{
					file_path = file_path.substr(2);
				}

				bool skip = false;
				for (u32 texture_path_index = 0; texture_path_index < loader_data.Texture_File_Paths.size(); ++texture_path_index)
				{
					if (loader_data.Texture_File_Paths.at(texture_path_index) == file_path)
					{
						loader_data.Textures.push_back(loader_data.Textures.at(texture_path_index));
						skip = true;
						break;
					}
				}

				if (!skip)
				{
					IS_PROFILE_SCOPE("Create new texture");
					Graphics::RHI_Texture* texture = Renderer::CreateTexture();

					loader_data.Textures.push_back(texture);
					loader_data.Texture_File_Paths.push_back(file_path);
				}
			}
		}

		std::string GetFileNameFromPath(const std::string& path)
		{
			const u64 file_name_start = path.find_last_of('/') + 1;
			const u64 file_name_end = path.find_last_of('.');
			return path.substr(file_name_start, file_name_end - file_name_start);
		}
		std::wstring GetFileNameFromPath(const std::wstring& path)
		{
			const u64 file_name_start = path.find_last_of('/') + 1;
			const u64 file_name_end = path.find_last_of('.');
			return path.substr(file_name_start, file_name_end - file_name_start);
		}


		void AssimpLoader::LoadMaterialTextures(AssimpLoaderData& loader_data)
		{
			/// EXPERIMENTAL: Trying to load textures in parallel.
			// TODO: Texture loading is one of the main issues of performance. (The uploading takes a lot of time)
			/// Maybe add a new system to defer GPU resource uploads, this would require something on CPU/Host
			/// side to track the current state of a resource. Or the GPU resource could track this it self??? Thoughts?
			concurrency::task_group task_group;
			for (size_t i = 0; i < loader_data.Textures.size(); ++i)
			{
				Graphics::RHI_Texture* texture = loader_data.Textures.at(i);
				std::string texture_file_path = loader_data.Texture_File_Paths.at(i);
				task_group.run([texture, texture_file_path]()
				{
					texture->LoadFromFile(texture_file_path);
					texture->SetName(Platform::WStringFromString(GetFileNameFromPath(texture_file_path)));
				});
			}
			task_group.wait();
		}

		void AssimpLoader::Optimize(AssimpLoaderData& loader_data)
		{
			IS_PROFILE_FUNCTION();

			const u64 vertex_byte_size = loader_data.Vertices.size() * sizeof(Graphics::Vertex);
			const u64 index_byte_size = loader_data.Indices.size() * sizeof(u32);

			const u64 vertex_count = vertex_byte_size / sizeof(Graphics::Vertex);
			const u64 vertex_size = sizeof(Graphics::Vertex);
			const u64 index_count = index_byte_size / sizeof(u32);

			/// The optimization order is important

			std::vector<u32> remap(index_count); /// allocate temporary memory for the remap table
			size_t total_vertices_optimized = meshopt_generateVertexRemap(remap.data(), loader_data.Indices.data(), index_count, loader_data.Vertices.data(), index_count, sizeof(Graphics::Vertex));

			std::vector<Graphics::Vertex> dst_vertices(total_vertices_optimized);
			std::vector<u32> dst_indices(index_count);

			meshopt_remapIndexBuffer(dst_indices.data(), loader_data.Indices.data(), index_count, remap.data());
			meshopt_remapVertexBuffer(dst_vertices.data(), loader_data.Vertices.data(), index_count, sizeof(Graphics::Vertex), remap.data());

			/// Vertex cache optimization - reordering triangles to maximize cache locality
			IS_INFO("Optimizing vertex cache...");
			meshopt_optimizeVertexCache(dst_indices.data(), dst_indices.data(), index_count, total_vertices_optimized);

			/// Overdraw optimizations - reorders triangles to minimize overdraw from all directions
			IS_INFO("Optimizing overdraw...");
			meshopt_optimizeOverdraw(dst_indices.data(), dst_indices.data(), index_count, glm::value_ptr(loader_data.Vertices.data()->Position), total_vertices_optimized, vertex_size, 1.05f);

			/// Vertex fetch optimization - reorders triangles to maximize memory access locality
			IS_INFO("Optimizing vertex fetch...");
			meshopt_optimizeVertexFetch(dst_vertices.data(), dst_indices.data(), index_count, dst_vertices.data(), total_vertices_optimized, vertex_size);

			loader_data.Vertices = dst_vertices;
			loader_data.Indices = dst_indices;
		}
	}
}