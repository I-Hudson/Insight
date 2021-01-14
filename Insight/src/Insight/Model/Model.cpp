#include "ispch.h"
#include "Model.h"

#include "stb_image.h"
#include "Insight/Core/Log.h"
#include "Insight/Memory/MemoryManager.h"
#include "Insight/Library/ModelLibrary.h"
#include "Insight/Instrumentor/Instrumentor.h"

Model::Model()
{
}

Model::Model(const std::string& filePath, const std::string& uuid)
	: m_path(filePath)
{
	if (!uuid.empty())
	{
		SetUUID(uuid);
	}

	Create(filePath);
}

Model::~Model()
{
	m_mesh.reset();
	for (auto& mat : m_materials)
	{
		mat.reset();
	}
	m_materials.clear();
}

void Model::Create(const std::string& filepath)
{
	if (!filepath.empty())
	{
		IS_INFO("Loading model: '{0}'", filepath);
		m_mesh = Object::CreateObject<Mesh>();
		m_mesh->LoadSubMeshes(filepath, *this);
		IS_INFO("Model Loaded: '{0}'", filepath);
	}
}

WeakPtr<Mesh> Model::GetMesh()
{
	return m_mesh;
}

const std::vector<WeakPtr<Material>> Model::GetMaterals()
{
	std::vector<WeakPtr<Material>> wpMaterials;
	for (auto& sp : m_materials)
	{
		wpMaterials.push_back(sp);
	}
	return wpMaterials;
}

const std::string& Model::GetFilePath() const
{
	return m_path;
}

const std::string& Model::GetModelName() const
{
	return m_modelName;
}

void Model::SetMaterials(const std::vector<std::pair<std::string, SharedPtr<Texture>>>& textures)
{
	for (auto& tex : textures)
	{
		m_textures.push_back(tex.second);

		SharedPtr<Material> material = Material::Create();
		material->CreateDefault();
		material->UploadTexture(tex.first, tex.second);
		m_materials.push_back(material);
	}
}