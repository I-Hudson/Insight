#include "Resource/Loaders/AssimpLoader.h"
#include "Resource/Model.h"
#include "Resource/Mesh.h"
#include "Resource/Texture2D.h"
#include "Resource/Material.h"
#include "Resource/ResourceManager.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "Platforms/Platform.h"

#include "Graphics/RenderContext.h"

#include <assimp/Importer.hpp>
//#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <meshoptimizer.h>

#include <glm/gtc/type_ptr.hpp>

#include <ppl.h>

namespace Insight
{
	namespace Runtime
	{
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

		bool AssimpLoader::LoadModel(Model* model, std::string file_path, u32 importer_flags)
		{
			Assimp::Importer importer;
			// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			// Remove cameras and lights
			importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

			const aiScene* scene = importer.ReadFile(file_path,
				importer_flags
			);

			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			{
				IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
				return false;
			}

			model->m_source_file_path = file_path;
			model->m_file_path = file_path;

			AssimpLoaderData loader_data;
			loader_data.Model = model;
			loader_data.Directoy = file_path.substr(0, file_path.find_last_of('/'));
			ProcessNode(scene->mRootNode, scene, "", loader_data);
			GenerateLODs(loader_data);
			loader_data.Model->m_materials = loader_data.Materials;

			Graphics::RenderContext::Instance().GetDeferredManager().Instance().Push([model, loader_data](Graphics::RHI_CommandList* cmdList)
				{
					UploadGPUData(const_cast<AssimpLoaderData&>(loader_data));

					for (size_t i = 0; i < model->m_meshes.size(); ++i)
					{
						for (auto& lod : model->m_meshes.at(i)->m_lods)
						{
							lod.Vertex_buffer = model->m_vertex_buffer;
							lod.Index_buffer = model->m_index_buffer;
						}
					}
				});

			bool allTexturesLoaded = false;
			while (!allTexturesLoaded)
			{
				allTexturesLoaded = true;
				for (size_t i = 0; i < model->m_materials.size(); ++i)
				{
					const Material* mat = model->m_materials.at(i);
					for (size_t textureTypeIdx = 0; textureTypeIdx < (size_t)TextureTypes::Count; ++textureTypeIdx)
					{
						Texture2D* texture = mat->GetTexture((TextureTypes)textureTypeIdx);
						if (texture != nullptr && texture->GetResourceState() != EResoruceStates::Loaded)
						{
							allTexturesLoaded = false;
							break;
						}
					}
				}
			}

			LoadMaterialTextures(loader_data);

			return true;
		}

		bool AssimpLoader::LoadMesh(Mesh* mesh, std::string file_path, u32 importer_flags)
		{
			Assimp::Importer importer;
			// Remove points and lines.
			importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
			// Remove cameras and lights
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
			loader_data.Mesh = mesh;
			loader_data.Directoy = file_path.substr(0, file_path.find_last_of('/'));
			loader_data.Mesh->m_lods.at(0).Vertex_buffer = Renderer::CreateVertexBuffer(1, 0);
			loader_data.Mesh->m_lods.at(0).Index_buffer = Renderer::CreateIndexBuffer(1);
			ProcessNode(scene->mRootNode, scene, "", loader_data, false);
			Optimize(loader_data);
			UploadGPUData(loader_data);
			LoadMaterialTextures(loader_data);

			return true;
		}

		bool AssimpLoader::ExportModel(Model* model, const std::string& file_path)
		{
			//Assimp::Exporter exporter;
			//if (exporter.Export(nullptr, "", file_path) != aiReturn_SUCCESS)
			//{
			//	IS_CORE_ERROR("[AssimpLoader::ExportModel] Assimp error code '{}'.", exporter.GetErrorString());
				return false;
			//}
			//return true;
		}

		void AssimpLoader::ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, AssimpLoaderData& loader_data, bool recursive)
		{
			IS_PROFILE_FUNCTION();
			if (aiNode->mNumMeshes > 0)
			{
				for (u32 i = 0; i < aiNode->mNumMeshes; ++i)
				{
					IS_PROFILE_SCOPE("Mesh evaluated");
					aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];

					AssimpLoaderData mesh_data;					
					ProcessMesh(aiMesh, aiScene, mesh_data);
					//Optimize(mesh_data);
					if (aiScene->HasMaterials() && aiMesh->mMaterialIndex < aiScene->mNumMaterials)
					{
						ExtractMaterialTextures(aiScene->mMaterials[aiMesh->mMaterialIndex], loader_data, mesh_data);
					}

					Mesh* new_mesh = nullptr;
					if (loader_data.Model)
					{
						//TODO Link the mesh as a resource and link all the textures as resources.
						// We are in the process of loading a model, add a new mesh to the model.
						if (loader_data.Model->m_meshes.size() <= loader_data.MeshIndex)
						{
							std::string path = loader_data.Model->GetFilePath() + "/";
							path += std::string(aiMesh->mName.C_Str(), aiMesh->mName.length);

							ResourceId meshResourceId(path, Mesh::GetStaticResourceTypeId());
							new_mesh = static_cast<Mesh*>(ResourceManager::CreateDependentResource(meshResourceId).Get());
							loader_data.Model->AddDependentResource(new_mesh);
							loader_data.Model->m_meshes.push_back(new_mesh);
						}
						else
						{
							new_mesh = loader_data.Model->m_meshes.at(loader_data.MeshIndex);
							loader_data.Model->AddDependentResource(new_mesh);
							std::string guid = new_mesh->GetGuid().ToString();
							IS_UNUSED(guid);
						}
						++loader_data.MeshIndex;

						new_mesh->m_mesh_name = aiMesh->mName.C_Str();
						new_mesh->m_source_file_path = loader_data.Model->m_source_file_path;
						new_mesh->m_file_path = loader_data.Directoy + '/' + new_mesh->m_mesh_name;
						new_mesh->m_boundingBox = Graphics::BoundingBox(mesh_data.Vertices.data(), static_cast<u32>(mesh_data.Vertices.size()));

						new_mesh->m_lods.at(0).Vertex_offset = static_cast<u32>(loader_data.Vertices.size());
						new_mesh->m_lods.at(0).First_index = static_cast<u32>(loader_data.Indices.size());

						// Move our vertices/indices which have been optimized, into the overall vectors.
						std::move(mesh_data.Vertices.begin(), mesh_data.Vertices.end(), std::back_inserter(loader_data.Vertices));
						std::move(mesh_data.Indices.begin(), mesh_data.Indices.end(), std::back_inserter(loader_data.Indices));

						new_mesh->m_lods.at(0).Vertex_buffer = loader_data.Model->m_vertex_buffer;
						new_mesh->m_lods.at(0).Index_buffer = loader_data.Model->m_index_buffer;

						new_mesh->m_lods.at(0).Vertex_count = static_cast<u32>(loader_data.Vertices.size()) - new_mesh->m_lods.at(0).Vertex_offset;
						new_mesh->m_lods.at(0).Index_count = static_cast<u32>(loader_data.Indices.size()) - new_mesh->m_lods.at(0).First_index;
					}
					else if (loader_data.Mesh)
					{
						new_mesh = loader_data.Mesh;
						new_mesh->m_boundingBox = Graphics::BoundingBox(mesh_data.Vertices.data(), static_cast<u32>(mesh_data.Vertices.size()));

						new_mesh->m_lods.at(0).Vertex_offset = static_cast<u32>(loader_data.Vertices.size());
						new_mesh->m_lods.at(0).First_index = static_cast<u32>(loader_data.Indices.size());

						// Move our vertices/indices which have been optimized, into the overall vectors.
						std::move(mesh_data.Vertices.begin(), mesh_data.Vertices.end(), std::back_inserter(loader_data.Vertices));
						std::move(mesh_data.Indices.begin(), mesh_data.Indices.end(), std::back_inserter(loader_data.Indices));

						new_mesh->m_lods.at(0).Vertex_count = static_cast<u32>(loader_data.Vertices.size()) - loader_data.Mesh->m_lods.at(0).Vertex_offset;
						new_mesh->m_lods.at(0).Index_count = static_cast<u32>(loader_data.Indices.size()) - loader_data.Mesh->m_lods.at(0).First_index;
					}
					new_mesh->m_resource_state = EResoruceStates::Loaded;
					new_mesh->m_transform_offset = ConvertMatrix(aiNode->mTransformation);

					for (size_t material_index = 0; material_index < mesh_data.Materials.size(); ++material_index)
					{
						new_mesh->SetMaterial(mesh_data.Materials.at(material_index));
					}

					//BoundingBox bounding_box = BoundingBox(vertices_optomized.data(), static_cast<u32>(vertices_optomized.size()));
					//SubmeshDrawInfo submesh_draw_info = { };
					//submesh_draw_info.Vertex_Offset = static_cast<u32>(vertices.size());
					//submesh_draw_info.First_Index = static_cast<u32>(indices.size());

					//submesh_draw_info.Transform = ConvertMatrix(aiNode->mTransformation);
					//submesh_draw_info.Vertex_Count = static_cast<u32>(vertices.size()) - submesh_draw_info.Vertex_Offset;
					//submesh_draw_info.Index_Count = static_cast<u32>(indices.size()) - submesh_draw_info.First_Index;

					/// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
					/// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
					/// Same applies to other texture as the following list summarizes:
					/// diffuse: texture_diffuseN
					/// specular: texture_specularN
					/// normal: texture_normalN
					//std::vector<Ptr<RHI_Texture>> diffuse_textures = LoadMaterialTextures(aiScene->mMaterials[aiMesh->mMaterialIndex], aiTextureType_DIFFUSE, "texture_diffuse");
					//submesh_draw_info.Textures.insert(submesh_draw_info.Textures.end(), diffuse_textures.begin(), diffuse_textures.end());
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

			// Then do the same for each of its children
			for (u32 i = 0; i < aiNode->mNumChildren; i++)
			{
				if (recursive || loader_data.Vertices.size() == 0)
				{
					ProcessNode(aiNode->mChildren[i], aiScene, directory, loader_data);
				}
			}
		}

		void AssimpLoader::ProcessMesh(aiMesh* mesh, const aiScene* aiScene, AssimpLoaderData& loader_data)
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

				Graphics::Vertex vertex = { };
				glm::vec4 vector = { }; /// we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class 
				/// so we transfer the data to this placeholder glm::vec3 first.
				/// positions
				vector.x = mesh->mVertices[i].x;
				vector.y = mesh->mVertices[i].y;
				vector.z = mesh->mVertices[i].z;
				vector.w = 1.0f;
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
					// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
					// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
					vec.x = mesh->mTextureCoords[0][i].x;
					vec.y = mesh->mTextureCoords[0][i].y;
					vertex.UV = glm::vec4(vec, 0, 0);

					// tangent
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
					//vertex.Bitangent = vector;
				}
				else
				{
					///vertex.UV = glm::vec2(0.0f, 0.0f);
				}

				loader_data.Vertices.push_back(vertex);
			}

			/// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
			for (unsigned int i = 0; i < mesh->mNumFaces; i++)
			{
				aiFace face = mesh->mFaces[i];
				/// retrieve all indices of the face and store them in the indices vector
				for (unsigned int j = 0; j < face.mNumIndices; j++)
				{
					IS_PROFILE_SCOPE("Add index");
					loader_data.Indices.push_back(face.mIndices[j]);
				}
			}
		}

		void AssimpLoader::ExtractMaterialTextures(aiMaterial* ai_material, AssimpLoaderData& known_data, AssimpLoaderData& mesh_data)
		{
			IS_PROFILE_FUNCTION();

			std::string material_path = known_data.Directoy + '/' + ai_material->GetName().C_Str();
			ResourceId materailResourceId(material_path, Material::GetStaticResourceTypeId());

			if (ResourceManager::HasResource(materailResourceId) && ResourceManager::GetResource(materailResourceId)->IsLoaded())
			{
				mesh_data.Materials.push_back(static_cast<Material*>(ResourceManager::GetResource(materailResourceId).Get()));
				return;
			}

			Material* material = nullptr;
			if (known_data.Model->m_materials.size() <= known_data.MaterialIndex)
			{
				material = static_cast<Material*>(ResourceManager::CreateDependentResource(materailResourceId).Get());
				known_data.Model->AddDependentResource(material);
			}
			else
			{
				material = known_data.Model->m_materials.at(known_data.MaterialIndex++);
			}
			ASSERT(material);

			ExtractMaterialType(ai_material, aiTextureType_BASE_COLOR, aiTextureType_DIFFUSE, "texture_diffuse", known_data.Directoy, TextureTypes::Diffuse, material);
			ExtractMaterialType(ai_material, aiTextureType_NORMAL_CAMERA, aiTextureType_NORMALS, "texture_normal", known_data.Directoy, TextureTypes::Normal, material);

			aiColor4D color_diffuse(1.0f, 1.0f, 1.0f, 1.0f);
			aiGetMaterialColor(ai_material, AI_MATKEY_COLOR_DIFFUSE, &color_diffuse);

			aiColor4D opacity(1.0f, 1.0f, 1.0f, 1.0f);
			aiGetMaterialColor(ai_material, AI_MATKEY_OPACITY, &opacity);

			material->SetProperty(MaterialProperty::Colour_R, color_diffuse.r);
			material->SetProperty(MaterialProperty::Colour_G, color_diffuse.g);
			material->SetProperty(MaterialProperty::Colour_B, color_diffuse.b);
			material->SetProperty(MaterialProperty::Colour_A, opacity.r);

			material->m_resource_state = Runtime::EResoruceStates::Loaded;

			mesh_data.Materials.push_back(material);
			known_data.Materials.push_back(material);
		}

		void AssimpLoader::ExtractMaterialType(aiMaterial* ai_material, aiTextureType ai_texture_type_pbr, aiTextureType ai_texture_type_legcy, const char* material_id, 
			const std::string& directory, TextureTypes textureType, Material* material)
		{
			// Determine if this is a pbr material or not
			aiTextureType ai_texture_type = aiTextureType_NONE;
			ai_texture_type = ai_material->GetTextureCount(ai_texture_type_pbr) > 0 ? ai_texture_type_pbr : ai_texture_type;
			ai_texture_type = (ai_texture_type == aiTextureType_NONE) ? (ai_material->GetTextureCount(ai_texture_type_legcy) > 0 ? ai_texture_type_legcy : ai_texture_type) : ai_texture_type;

			const u32 texture_count = ai_material->GetTextureCount(ai_texture_type);
			if (texture_count == 0)
			{
				return;
			}

			aiString ai_material_path;
			ai_material->GetTexture(ai_texture_type, 0, &ai_material_path);
			if (ai_material_path.length == 0)
			{
				return;
			}
			std::string material_file_path = ai_material_path.C_Str();

			if (material_file_path.find("./") != std::string::npos)
			{
				material_file_path = material_file_path.substr(2);
			}

			// Use absolute paths.
			// TODO: Should probably change this to be relative. Paths lengths are getting very long.
			material_file_path = directory + '/' + material_file_path;

			{
				IS_PROFILE_SCOPE("Create new texture");
				Texture2D* texture = static_cast<Texture2D*>(ResourceManager::Load(ResourceId(material_file_path, Texture2D::GetStaticResourceTypeId())).operator Insight::Runtime::IResource *());
				material->SetTexture(textureType, texture);
				material->AddReferenceResource(texture);
			}

			//for (u32 i = 0; i < ai_material->GetTextureCount(ai_texture_type); ++i)
			//{
			//	aiString str;
			//	ai_material->GetTexture(ai_texture_type, i, &str);
			//	std::string file_path = str.C_Str();

			//	if (file_path.find("./") != std::string::npos)
			//	{
			//		file_path = file_path.substr(2);
			//	}

			//	// Use absolute paths.
			//	// TODO: Should probably change this to be relative. Paths lengths are getting very long.
			//	file_path = known_data.Directoy + '/' + file_path;

			//	bool skip = false;
			//	// Check if mesh_data knows about the texture.
			//	for (u32 texture_path_index = 0; texture_path_index < mesh_data.Texture_File_Paths.size(); ++texture_path_index)
			//	{
			//		if (mesh_data.Texture_File_Paths.at(texture_path_index) == file_path)
			//		{
			//			mesh_data.Textures.push_back(mesh_data.Textures.at(texture_path_index));
			//			mesh_data.Texture_File_Paths.push_back("");
			//			skip = true;
			//			break;
			//		}
			//	}
			//	// Check if known_data knows about the texture.
			//	for (u32 texture_path_index = 0; texture_path_index < known_data.Texture_File_Paths.size(); ++texture_path_index)
			//	{
			//		if (known_data.Texture_File_Paths.at(texture_path_index) == file_path)
			//		{
			//			mesh_data.Textures.push_back(known_data.Textures.at(texture_path_index));
			//			mesh_data.Texture_File_Paths.push_back("");
			//			skip = true;
			//			break;
			//		}
			//	}

			//	if (!skip)
			//	{
			//		IS_PROFILE_SCOPE("Create new texture");
			//		Texture2D* texture = static_cast<Texture2D*>(ResourceManager::Instance().Load(file_path, Texture2D::GetStaticResourceTypeId()));
			//		mesh_data.Textures.push_back(texture);
			//		mesh_data.Texture_File_Paths.push_back(file_path);
			//	}
			//}
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
			//concurrency::task_group task_group;
			//for (size_t i = 0; i < loader_data.Textures.size(); ++i)
			//{
			//	Texture2D* texture = loader_data.Textures.at(i);
			//	std::string texture_file_path = loader_data.Texture_File_Paths.at(i);
			//	if (!texture_file_path.empty())
			//	{
			//		task_group.run([texture, texture_file_path]()
			//			{
			//				//texture->LoadFromFile(texture_file_path);
			//				texture->GetRHITexture()->SetName(Platform::WStringFromString(GetFileNameFromPath(texture_file_path)));
			//			});
			//	}
			//}
			//task_group.wait();
		}

		void AssimpLoader::GenerateLODs(AssimpLoaderData& loader_data)
		{
			for (u32 lod_index = 1; lod_index < Mesh::s_LOD_Count; ++lod_index)
			{
				for (auto& mesh : loader_data.Model->m_meshes)
				{
					std::vector<u32> indcies_to_lod;

					const float error_rate = 1.0f;
					const u32 target_index_count = static_cast<u32>(static_cast<float>(mesh->m_lods.at(0).Index_count) * (1.0f - (0.4f * lod_index)));

					const u64 indices_start = mesh->m_lods.at(0).First_index;
					const u64 indices_count = mesh->m_lods.at(0).Index_count;
					const u64 vertex_start = mesh->m_lods.at(0).Vertex_offset;
					const u64 vertex_count = mesh->m_lods.at(0).Vertex_count;

					std::vector<u32>::iterator indices_begin = loader_data.Indices.begin() + indices_start;
					std::vector<u32> result_lod;
					result_lod.resize(indices_count);

					// Try and simplify the mesh, try and preserve mesh topology.
					u64 result_index_count = meshopt_simplify(
						result_lod.data()
						, &*indices_begin
						, static_cast<u64>(indices_count)
						, &(loader_data.Vertices.data() + vertex_start)->Position.x
						, vertex_count
						, sizeof(Graphics::Vertex)
						, target_index_count
						, error_rate);

					if (false && result_index_count > target_index_count)
					{
						// Try and simplify the mesh, doesn't preserve mesh topology.
						result_index_count = meshopt_simplifySloppy(
							  result_lod.data()
							, &*indices_begin
							, static_cast<u64>(indices_count)
							, &(loader_data.Vertices.data() + vertex_start)->Position.x
							, vertex_count
							, sizeof(Graphics::Vertex)
							, target_index_count
							, error_rate);
					}
					result_lod.resize(result_index_count);

					mesh->m_lods.push_back(mesh->m_lods.at(0));
					MeshLOD& mesh_lod = mesh->m_lods.back();
					mesh_lod.First_index = static_cast<u32>(loader_data.Indices.size());
					mesh_lod.Index_count = static_cast<u32>(result_index_count);

					std::move(result_lod.begin(), result_lod.end(), std::back_inserter(loader_data.Indices));
				}
			}
		}

		void AssimpLoader::Optimize(AssimpLoaderData& loader_data)
		{
			IS_PROFILE_FUNCTION();

			const u64 vertex_count = loader_data.Vertices.size();
			const u64 vertex_size = sizeof(Graphics::Vertex);
			const u64 index_count = loader_data.Indices.size();

			/// The optimization order is important

			//std::vector<u32> remap(index_count); /// allocate temporary memory for the remap table
			//size_t total_vertices_optimized = meshopt_generateVertexRemap(remap.data(), NULL, index_count, loader_data.Vertices.data(), index_count, sizeof(Graphics::Vertex));

			std::vector<Graphics::Vertex> temp_vertices = loader_data.Vertices;//(total_vertices_optimized);
			std::vector<u32> temp_indices = loader_data.Indices; //(index_count);

			//meshopt_remapIndexBuffer(dst_indices.data(), NULL, index_count, remap.data());
			//meshopt_remapVertexBuffer(dst_vertices.data(), loader_data.Vertices.data(), index_count, sizeof(Graphics::Vertex), remap.data());

			/// Vertex cache optimization - reordering triangles to maximize cache locality
			IS_INFO("Optimizing vertex cache...");
			meshopt_optimizeVertexCache(temp_indices.data(), loader_data.Indices.data(), index_count, vertex_count);

			/// Overdraw optimizations - reorders triangles to minimize overdraw from all directions
			IS_INFO("Optimizing overdraw...");
			meshopt_optimizeOverdraw(loader_data.Indices.data(), temp_indices.data(), index_count, glm::value_ptr(loader_data.Vertices.data()->Position), vertex_count, vertex_size, 1.05f);

			/// Vertex fetch optimization - reorders triangles to maximize memory access locality
			IS_INFO("Optimizing vertex fetch...");
			meshopt_optimizeVertexFetch(loader_data.Vertices.data(), loader_data.Indices.data(), index_count, temp_vertices.data(), vertex_count, vertex_size);

			//loader_data.Vertices = dst_vertices;
			//loader_data.Indices = dst_indices;
		}

		void AssimpLoader::UploadGPUData(AssimpLoaderData& loader_data)
		{
			Graphics::RHI_Buffer** vertex_buffer = nullptr;
			Graphics::RHI_Buffer** index_buffer = nullptr;
			if (loader_data.Model)
			{
				vertex_buffer = &loader_data.Model->m_vertex_buffer;
				index_buffer = &loader_data.Model->m_index_buffer;
			}
			else if (loader_data.Mesh)
			{
				vertex_buffer = &loader_data.Mesh->m_lods.at(0).Vertex_buffer;
				index_buffer = &loader_data.Mesh->m_lods.at(0).Index_buffer;
			}

			const u64 cpu_vertex_buffer_size = sizeof(Graphics::Vertex) * loader_data.Vertices.size();
			const u64 cpu_index_buffer_size = sizeof(u32) * loader_data.Indices.size();

			if (!(*vertex_buffer))
			{
				*vertex_buffer = Renderer::CreateVertexBuffer(cpu_vertex_buffer_size, sizeof(Graphics::Vertex));
			}
			if (!(*index_buffer))
			{
				*index_buffer = Renderer::CreateIndexBuffer(cpu_index_buffer_size);
			}

			if (cpu_vertex_buffer_size > (*vertex_buffer)->GetSize())
			{
				// Resize buffer as it is too small.
				(*vertex_buffer)->Resize(cpu_vertex_buffer_size);
			}
			if (cpu_index_buffer_size > (*index_buffer)->GetSize())
			{
				// Resize buffer as it is too small.
				(*index_buffer)->Resize(cpu_index_buffer_size);
			}

			// Upload the vertices and indics data to the GPU.
			(*vertex_buffer)->Upload(loader_data.Vertices.data(), cpu_vertex_buffer_size);
			(*index_buffer)->Upload(loader_data.Indices.data(), cpu_index_buffer_size);
		}
	}
}