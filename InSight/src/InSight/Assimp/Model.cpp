#include "ispch.h"
#include "Model.h"

#include "stb_image.h"
#include "Insight/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Instrumentor/Instrumentor.h"

Model::Model(const std::string& filePath)
{
	IS_INFO("Loading model");
	LoadMesh(filePath);
}

Model::~Model()
{
	for (auto it = m_meshes.begin(); it != m_meshes.end(); ++it)
	{
		DELETE_ON_HEAP(*it);
	}
}

Mesh* Model::GetSubMesh(int index)
{
	IS_ASSERT(index >= 0 && index < m_meshes.size(), "Model: GetSubMesh: Out of range.");
	return m_meshes[index];
}

const std::string& Model::GetName() const
{
	return m_modelName;
}

void Model::LoadMesh(const std::string& filePath)
{
	IS_PROFILE_FUNCTION();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		IS_CORE_ERROR("Assimp model load: {0}", importer.GetErrorString());
		return;
	}

	m_modelName = filePath.substr(filePath.find_last_of('/'));
	m_directory = filePath.substr(0, filePath.find_last_of('/'));
	ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene)
{
	IS_PROFILE_FUNCTION();

	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(ProcessMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh* Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	IS_PROFILE_FUNCTION();

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// process vertex positions, normals and texture coordinates
		glm::vec4 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		position.w = 1.0f;
		vertex.Position = position;

		glm::vec4 colour = glm::vec4();
		colour.r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colour.a = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		vertex.Colour = colour;

		glm::vec4 normal;
		if (mesh->mNormals != nullptr)
		{
			normal.x = mesh->mNormals[i].x;
			normal.y = mesh->mNormals[i].y;
			normal.z = mesh->mNormals[i].z;
			normal.w = 0.0f;
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
		std::vector<Texture> diffuseMaps = LoadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = LoadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return NEW_ON_HEAP(Mesh, vertices, indices, textures, mesh->mName.C_Str());
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
{
	IS_PROFILE_FUNCTION();

	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		for (unsigned int j = 0; j < m_texturesLoaded.size(); j++)
		{
			if (std::strcmp(m_texturesLoaded[j].Path.data(), str.C_Str()) == 0)
			{
				textures.push_back(m_texturesLoaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			texture.ID = TextureFromFile(str.C_Str(), m_directory);
			texture.Type = typeName;
			texture.Path = str.C_Str();
			textures.push_back(texture);
			m_texturesLoaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}

unsigned int Model::TextureFromFile(const std::string& path, const std::string& directory, bool gamma)
{
	std::string filename = std::string(path);
	filename = directory + '/' + filename;

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return 0;
}
