#include "ispch.h"
#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Model/ModelLoading.h"
#include "Engine/Graphics/RenderList.h"
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
		, m_vertexBuffer(nullptr)
		, m_indexBuffer(nullptr)
	{ }

	SubMesh::SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount, std::vector<Vertex>& vertices , std::vector<u32>& indices)
		: m_firstVertex(firstVertex)
		, m_vertexCount(vertexCount)
		, m_firstIndex(firstIndex)
		, m_indexCount(indexCount)
	{
		m_vertexBuffer = GPUBuffer::New();
		m_indexBuffer = GPUBuffer::New();

		m_vertexBuffer->Init(GPUBufferDesc::Vertex(sizeof(Vertex), vertexCount, vertices.data()));
		m_indexBuffer->Init(GPUBufferDesc::Index(sizeof(u32), m_indexCount, indices.data()));
	}

	SubMesh::~SubMesh()
	{ }

	void SubMesh::Release()
	{
		m_vertexBuffer->ReleaseGPU();
		::Delete(m_vertexBuffer);
		m_indexBuffer->ReleaseGPU();
		::Delete(m_indexBuffer);
	}

	std::string SubMesh::GetTexture(const std::string& textureId)
	{
		auto itr = m_textures.find(textureId);
		if (itr == m_textures.end())
		{
			return "";
		}
		return itr->second;
	}

	void SubMesh::Draw(RenderList* drawList)
	{
		DrawCall drawCall;
		drawCall.Geometry.VertexBuffer = m_vertexBuffer;
		drawCall.Geometry.IndexBuffer = m_indexBuffer;
		drawCall.Draw.IndciesStart = 0;
		drawCall.Draw.IndicesCount = GetVertexCount();
		drawCall.Draw.IndciesStart = 0;
		drawCall.Draw.IndicesCount = GetIndexCount();
	}

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
		//m_vertexBuffer = GPUBuffer::New();
		//m_indexBuffer = GPUBuffer::New();
	}

	Mesh::~Mesh()
	{ }

	void Mesh::Draw(RenderList* drawList)
	{
		for (auto& mesh : m_subMeshes)
		{
			mesh.Draw(drawList);
		}
	}

	void Mesh::Release()
	{
		for (auto& subMesh : m_subMeshes)
		{
			subMesh.Release();
		}
	}

	void Mesh::SetupGPUBuffers()
	{
		m_vertexCount = m_vertices.size();
		m_indexCount = m_indices.size();
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
	{
		m_mesh.Release();
	}

	void Model::LoadFromFile(const std::string& filePath)
	{
		ModelLoading::AssimpLoader::LoadFromFile(*this, filePath);
		//m_mesh.SetupGPUBuffers();
	}
}