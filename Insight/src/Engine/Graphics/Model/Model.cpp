#include "ispch.h"
#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Model/ModelLoading.h"
#include <filesystem>

namespace Insight::Graphics
{
	/// <summary>
	/// SubMesh
	/// </summary>
	SubMesh::SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount)
		: m_firstVertex(firstVertex)
		, m_vertexCount(vertexCount)
		, m_firstIndex(firstIndex)
		, m_indexCount(indexCount)
	{ }

	SubMesh::~SubMesh()
	{ }

	void SubMesh::SetDimensions(glm::vec3 min, glm::vec3 max)
	{

	}

	/// <summary>
	/// Mesh
	/// </summary>
	/// <param name="subMeshCount"></param>
	Mesh::Mesh()
		: m_vertexBuffer(nullptr)
		, m_indexBuffer(nullptr)
	{
		m_vertexBuffer = GPUBuffer::New();
		m_indexBuffer = GPUBuffer::New();
	}

	Mesh::~Mesh()
	{ }

	void Mesh::SetupGPUBuffers()
	{
		m_vertexBuffer->Init(GPUBufferDesc::Vertex(sizeof(Vertex), m_vertices.size(), m_vertices.data()));
		m_indexBuffer->Init(GPUBufferDesc::Index(sizeof(u32), m_indices.size(), m_indices.data()));
	}


	/// <summary>
	/// Model
	/// </summary>
	Model::Model()
	{
		SetType<Model>();
	}

	Model::Model(const std::string& filePath)
	{
		SetType<Model>();
		LoadFromFile(filePath);
	}

	Model::~Model()
	{ }

	void Model::LoadFromFile(const std::string& filePath)
	{
		std::string fileExtension = std::filesystem::path(filePath).extension().u8string();
		if (fileExtension == ".gltf")
		{
			ModelLoading::GLTFModelLoader::LoadFromFile(*this, filePath);
		}
		else if (fileExtension == ".fbx")
		{
			ModelLoading::FBXModelLoader::LoadFromFile(*this, filePath);
		}
		else if (fileExtension == ".obj")
		{
			ModelLoading::OBJModelLoader::LoadFromFile(*this, filePath);
		}

		for (u64 i = 0; i < m_meshes.size(); ++i)
		{
			m_meshes.at(i).SetupGPUBuffers();
		}
	}
}