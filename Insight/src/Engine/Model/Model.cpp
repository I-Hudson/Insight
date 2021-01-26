#include "ispch.h"
#include "Model.h"

#include "stb_image.h"
#include "Engine/Core/Log.h"
#include "Engine/Memory/MemoryManager.h"
#include "Engine/Library/ModelLibrary.h"
#include "Engine/Instrumentor/Instrumentor.h"

Model::Model()
{
	SetType<Model>();
}

Model::Model(const std::string& filePath, const std::string& uuid)
	: m_path(filePath)
{
	SetType<Model>();

	if (!uuid.empty())
	{
		SetUUID(uuid);
	}

	Create(filePath);
}

Model::~Model()
{
	::Delete(m_mesh);
	for (auto& mat : m_materials)
	{
		::Delete(mat);
	}
	for (auto& texture : m_textures)
	{
		::Delete(texture);
	}

	m_materials.clear();
	m_textures.clear();
}

void Model::Create(const std::string& filepath)
{
	if (!filepath.empty())
	{
		IS_INFO("Loading model: '{0}'", filepath);
		m_mesh = ::New<Mesh>();
		m_mesh->LoadSubMeshes(filepath, *this);
		IS_INFO("Model Loaded: '{0}'", filepath);
	}
}

Mesh* Model::GetMesh()
{
	return m_mesh;
}

const std::vector<Material*>& Model::GetMaterals()
{
	return m_materials;
}

const std::string& Model::GetFilePath() const
{
	return m_path;
}

const std::string& Model::GetModelName() const
{
	return m_modelName;
}

void Model::SetMaterials(const std::vector<std::pair<std::string, Texture*>>& textures)
{
		Material* material = Material::New();
		material->CreateDefault();
		m_materials.push_back(material);
		int texturesSet = 0;

		for (auto& tex : textures)
		{
			m_textures.push_back(tex.second);
			material->UploadTexture(tex.first, tex.second);
			if (tex.first == "texture_diffuse")
			{
				texturesSet = 1;
			}
			else
			{
				IS_CORE_INFO("");
			}
		}
		MaterialBlockData mbd;
		material->UploadUniform("TextureLookup", &texturesSet, sizeof(int), mbd);
		material->Update();
}