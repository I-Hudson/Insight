#include "ispch.h"
#include "Mesh.h"

#include "Model.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

#include "Insight/FileSystem/FileSystem.h"
#include "Platform/Vulkan/VulkanRenderer.h"
#include "Insight/Component/MeshComponent.h"

Mesh::~Mesh()
{
	for (auto& subMesh : m_subMeshes)
	{
		subMesh.reset();
	}
	m_subMeshes.clear();
}

void Mesh::SetVertices(const std::vector<Vertex>& vertices, const U32& submeshIndex)
{
	SharedPtr<SubMesh> subMesh;
	if (submeshIndex >= m_subMeshes.size())
	{
		subMesh = CreateSharedPtr<SubMesh>();
		m_subMeshes.push_back(subMesh);
	}
	else
	{
		subMesh = m_subMeshes.at(submeshIndex);
	}
	subMesh->SetVertices(vertices);
}

void Mesh::SetIndices(const std::vector<U32>& indices, const U32& submeshIndex)
{
	SharedPtr<SubMesh> subMesh;
	if (submeshIndex >= m_subMeshes.size())
	{
		subMesh = CreateSharedPtr<SubMesh>();
		m_subMeshes.push_back(subMesh);
	}
	else
	{
		subMesh = m_subMeshes.at(submeshIndex);
	}
	subMesh->SetIndices(indices);
}

void Mesh::Rebuild()
{
	for (auto& subMesh : m_subMeshes)
	{
		subMesh->Rebuild();
	}
}

void Mesh::Draw(VkCommandBuffer cmd)
{
	for (auto& mesh : m_subMeshes)
	{
		mesh->Draw(cmd);
	}
}

void Mesh::Draw(VkCommandBuffer cmd, const std::vector<WeakPtr<Material>>& materials, const std::vector<MaterialBlockData>& materialBlockDatas, MeshMaterialUpdateFunc materialUpdateFunc, MeshComponent* meshCompoennt)
{
	for (U64 i = 0; i < m_subMeshes.size(); ++i)
	{
		if (i >= materials.size())
		{
			return;
		}

		if (auto materialSP = materials.at(i).lock())
		{
			MaterialBlockData& materialBlockData = i < materialBlockDatas.size() ? const_cast<MaterialBlockData&>(materialBlockDatas.at(i)) : MaterialBlockData();
			auto vMaterialSP = DynamicPointerCast<vks::VulkanMaterial>(materialSP);
			if (materialUpdateFunc)
			{
				materialUpdateFunc(meshCompoennt, vMaterialSP, materialBlockData);
			}
			vMaterialSP->Bind(cmd, &materialBlockData);
			m_subMeshes.at(i)->Draw(cmd);
		}
	}
}

void Mesh::LoadSubMeshes(const std::string& filePath, Model& model)
{
	IS_PROFILE_FUNCTION();

	m_model = &model;
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
		return;
	}
	// retrieve the directory path of the filepath
	m_directory = filePath.substr(0, filePath.find_last_of('/'));
	m_meshName = scene->mRootNode->mName.C_Str();
	ProcessNode(scene->mRootNode, scene);
}

void Mesh::ProcessNode(aiNode* node, const aiScene* scene)
{
	IS_PROFILE_FUNCTION();

	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_subMeshes.push_back(ProcessMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

SharedPtr<SubMesh> Mesh::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	IS_PROFILE_FUNCTION();

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	LoadedTextures textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates
		glm::vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertex.Position = position;

		glm::vec4 colour = glm::vec4();
		colour.r = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.g = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.b = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.a = 1.0f; // static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		vertex.Colour = colour;

		glm::vec3 normal;
		if (mesh->mNormals != nullptr)
		{
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
		}
		vertex.Normal = normal;

		glm::vec2 uv;
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			uv.x = mesh->mTextureCoords[0][i].x;
			uv.y = mesh->mTextureCoords[0][i].y;
		}
		else
		{
			uv.x = 0.0f;
			uv.y = 0.0f;
		}
		vertex.UV1 = uv;

		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		LoadedTextures diffuseMaps = LoadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		LoadedTextures specularMaps = LoadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	m_model->SetMaterials(textures);
	return CreateSharedPtr<SubMesh>(vertices, indices);
}

LoadedTextures Mesh::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
{
	IS_PROFILE_FUNCTION();

	LoadedTextures textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		//bool skip = false;
		//for (unsigned int j = 0; j < m_loadedTextures.size(); j++)
		//{
		//	if (std::strcmp(m_loadedTextures[j].GetFilePath().c_str(), str.C_Str()) == 0)
		//	{
		//		textures.push_back(m_loadedTextures[j]);
		//		skip = true;
		//		break;
		//	}
		//}
		//if (!skip)
		{   // if texture hasn't been loaded already, load it
			textures.emplace_back(typeName,  FileSystem::FileSystemManager::Instance()->LoadObject<Texture>(m_directory + "/" + str.C_Str()));
			//m_loadedTextures.push_back(texture); // add to loaded textures
		}
	}

	return textures;
}
