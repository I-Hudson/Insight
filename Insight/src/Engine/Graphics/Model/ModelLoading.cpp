#include "ispch.h"
#include "Engine/Graphics/Model/ModelLoading.h"
#include "glm/gtc/type_ptr.hpp"

#include "Engine/Module/GraphicsModule.h"
#define STB_IMAGE_IMPLEMENTATION
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"
#include "stb_image.h"

namespace Insight::Graphics::ModelLoading
{
	/// <summary>
	/// AssimpLoader
	/// </summary>
	/// <param name="model"></param>
	/// <param name="filePath"></param>
	void AssimpLoader::LoadFromFile(Model& model, const std::string& filePath)
	{
		IS_PROFILE_FUNCTION();
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate |aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
			return;
		}
		// retrieve the directory path of the filepath
		model.m_fileDirectory = filePath.substr(0, filePath.find_last_of('/'));
		model.m_mesh.m_meshName = scene->mRootNode->mName.C_Str();
		ProcessNode(model.m_mesh, scene->mRootNode, scene, model.m_fileDirectory);
	}

	void AssimpLoader::ProcessNode(Mesh& mesh, aiNode* aiNode, const aiScene* aiScene, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		// process all the node's meshes (if any)
		for (unsigned int i = 0; i < aiNode->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aiScene->mMeshes[aiNode->mMeshes[i]];
			ProcessMesh(mesh, aiMesh, aiScene, directory);
		}
		// then do the same for each of its children
		for (unsigned int i = 0; i < aiNode->mNumChildren; i++)
		{
			ProcessNode(mesh, aiNode->mChildren[i], aiScene, directory);
		}
	}

	void AssimpLoader::ProcessMesh(Mesh& mesh, aiMesh* aiMesh, const aiScene* aiScene, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		std::vector<Vertex> vertices;
		std::vector<u32> indices;

		u32 vertexStart = static_cast<u32>(mesh.m_vertices.size());
		u32 indexStart = static_cast<u32>(mesh.m_indices.size());
		u32 vertexCount = 0;
		u32 indexCount = 0;

		for (unsigned int i = 0; i < aiMesh->mNumVertices; ++i)
		{
			Vertex vertex;
			// process vertex positions, normals and texture coordinates
			glm::vec3 position;
			position.x = aiMesh->mVertices[i].x;
			if (Module::GraphicsModule::Instance()->IsVulkan())
			{
				position.y = aiMesh->mVertices[i].y *= -1;
			}
			else
			{
				position.y = aiMesh->mVertices[i].y;
			}
			position.z = aiMesh->mVertices[i].z;
			vertex.Position = position;

			glm::vec4 colour = glm::vec4();
			colour.r = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.g = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.b = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			colour.a = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			vertex.Colour = colour;

			glm::vec3 normal;
			if (aiMesh->mNormals != nullptr)
			{
				normal.x = aiMesh->mNormals[i].x;
				if (Module::GraphicsModule::Instance()->IsVulkan())
				{
					normal.y = aiMesh->mNormals[i].y *= -1;
				}
				else
				{
					normal.y = aiMesh->mNormals[i].y;
				}
				normal.z = aiMesh->mNormals[i].z;
			}
			vertex.Normal = normal;

			glm::vec2 uv;
			if (aiMesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				uv.x = aiMesh->mTextureCoords[0][i].x;
				uv.y = aiMesh->mTextureCoords[0][i].y;
			}
			else
			{
				uv.x = 0.0f;
				uv.y = 0.0f;
			}
			vertex.UV1 = uv;

			mesh.m_vertices.push_back(vertex);
			vertices.push_back(vertex);
			++vertexCount;
		}

		for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
		{
			aiFace face = aiMesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; ++j)
			{
				mesh.m_indices.push_back(face.mIndices[j]);
				indices.push_back(face.mIndices[j]);
				++indexCount;
			}
		}
		mesh.m_subMeshes.push_back(SubMesh(vertexStart, vertexCount, indexStart, indexCount, vertices, indices));

		if (aiMesh->mMaterialIndex >= 0)
		{
			aiMaterial* aiMaterial = aiScene->mMaterials[aiMesh->mMaterialIndex];
			mesh.m_subMeshes.back().m_textures = LoadMateials(aiMaterial, aiTextureType_DIFFUSE, "texture_diffuse", directory);
		}
	}

	MeshTextures AssimpLoader::LoadMateials(aiMaterial* aiMaterial, aiTextureType aiType, const std::string& typeName, const std::string& directory)
	{
		IS_PROFILE_FUNCTION();

		MeshTextures textures;
		for (unsigned int i = 0; i < aiMaterial->GetTextureCount(aiType); ++i)
		{
			aiString str;
			aiMaterial->GetTexture(aiType, i, &str);
			bool skip = false;
			std::string formatedString = str.C_Str();
			if (formatedString.find(":") != std::string::npos)
			{
				formatedString = formatedString.substr(formatedString.find_last_of('\\') + 1);
			}

			//for (unsigned int j = 0; j < loadedTextures.size(); ++j)
			//{
			//	if (std::strcmp(m_loadedTextures[j].GetFilePath().c_str(), str.C_Str()) == 0)
			//	{
			//		textures.push_back(m_loadedTextures[j]);
			//		skip = true;
			//		break;
			//	}
			//}
			if (!skip)
			{
				//if texture hasn't been loaded already, load it
				//Texture* texture = Texture::New();
				//texture->Init(m_directory + "/" + str.C_Str());
				//textures.emplace_back(typeName, texture);
				//m_loadedTextures.push_back(texture); // add to loaded textures
				textures.insert({ typeName, directory + "/" + formatedString });
			}
		}
		return textures;
	}
}