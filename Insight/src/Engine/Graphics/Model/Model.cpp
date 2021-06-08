#include "ispch.h"
#include "Engine/Graphics/Model/Model.h"
#include "Engine/Graphics/Model/ModelLoading.h"
#include "Engine/Graphics/RenderList.h"
#include "Engine/Graphics/Graphics.h"
#include <filesystem>

namespace Insight
{
	/// <summary>
	/// SubMesh
	/// </summary>
	SubMesh::SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount, Graphics::GPUBuffer* vertexBuffer, Graphics::GPUBuffer* indexBuffer)
		: m_firstVertex(firstVertex)
		, m_vertexCount(vertexCount)
		, m_firstIndex(firstIndex)
		, m_indexCount(indexCount)
		, m_releaseGPUBuffers(false)
		, m_vertexBuffer(vertexBuffer)
		, m_indexBuffer(indexBuffer)
	{ }

	SubMesh::SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount, std::vector<Vertex>& vertices , std::vector<u32>& indices)
		: m_firstVertex(firstVertex)
		, m_vertexCount(vertexCount)
		, m_firstIndex(firstIndex)
		, m_indexCount(indexCount)
		, m_releaseGPUBuffers(true)
	{
		m_vertexBuffer = Graphics::GPUBuffer::New();
		m_indexBuffer = Graphics::GPUBuffer::New();
		m_vertexBuffer->Init(Graphics::GPUBufferDesc::Vertex(sizeof(Vertex), vertexCount, vertices.data()));
		m_indexBuffer->Init(Graphics::GPUBufferDesc::Index(sizeof(u32), m_indexCount, indices.data()));
	}

	SubMesh::~SubMesh()
	{ }

	void SubMesh::Release()
	{
		if (m_releaseGPUBuffers)
		{
			m_vertexBuffer->ReleaseGPU();
			::Delete(m_vertexBuffer);
			m_indexBuffer->ReleaseGPU();
			::Delete(m_indexBuffer);
		}
		m_vertexBuffer = nullptr;
		m_indexBuffer = nullptr;
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

	std::string SubMesh::GetTexture(MaterialTextureType type)
	{
		auto itr = m_textureStrings.find(type);
		if (itr != m_textureStrings.end())
		{
			return itr->second;
		}
		return "";
	}

	void SubMesh::Draw(Graphics::RenderListView* drawList, const glm::mat4& worldTransform, const Maths::Frustum& cameraFrustum)
	{
		//TODO: look into per submesh culling.

		Graphics::DrawCall drawCall;
		drawCall.Geometry.VertexBuffer = m_vertexBuffer;
		drawCall.Geometry.IndexBuffer = m_indexBuffer;
		drawCall.Draw.VertexStart = m_firstIndex;
		drawCall.Draw.VertexCount = GetVertexCount();
		drawCall.Draw.IndciesStart = m_firstIndex;
		drawCall.Draw.IndicesCount = GetIndexCount();
		drawCall.WorldTransform = worldTransform;
		drawCall.Dimensions = m_dimensions;
		drawCall.DiffuseTexture = GetTexture("texture_diffuse");
		drawList->AddDrawCall(Graphics::MaterialDrawMode::Opaque, drawCall);
	}

	void SubMesh::SetDimensions(glm::vec3 min, glm::vec3 max)
	{

	}

	/// <summary>
	/// Mesh
	/// </summary>
	/// <param name="subMeshCount"></param>
	Mesh::Mesh()
	{
		m_vertexBuffer = Graphics::GPUBuffer::New();
		m_indexBuffer = Graphics::GPUBuffer::New();
	}

	Mesh::~Mesh()
	{ 
		Release();
		::Delete(m_vertexBuffer);
		::Delete(m_indexBuffer);
	}

	std::vector<std::string> Mesh::GetAllSubMeshTextures(MaterialTextureType type)
	{
		std::vector<std::string> vec;
		vec.reserve(m_subMeshes.size());
		for (auto& sm : m_subMeshes)
		{
			vec.push_back(std::move(sm.GetTexture(MaterialTextureType::Diffuse)));
			if (vec.back() == "")
			{
				vec.back() = "./data/embed2.jpg";
			}
		}
		return vec;
	}

	void Mesh::Draw(Graphics::RenderListView* drawList, const glm::mat4& worldTransform, const Maths::Frustum& cameraFrustum)
	{
		if (::Graphics::MeshBatchingExt())
		{
			Graphics::DrawCall drawCall;
			drawCall.Geometry.VertexBuffer = m_vertexBuffer;
			drawCall.Geometry.IndexBuffer = m_indexBuffer;
			drawCall.Draw.IndciesStart = 0;
			drawCall.Draw.IndicesCount = GetVertexCount();
			drawCall.Draw.IndciesStart = 0;
			drawCall.Draw.IndicesCount = GetIndexCount();
			drawCall.WorldTransform = worldTransform;
			drawCall.DiffuseTextureMeshBatch = GetAllSubMeshTextures(MaterialTextureType::Diffuse);
			drawList->AddDrawCall(Graphics::MaterialDrawMode::Opaque, drawCall);
		}
		else
		{
			for (auto& mesh : m_subMeshes)
			{
				mesh.Draw(drawList, worldTransform, cameraFrustum);
			}
		}
	}

	void Mesh::Release()
	{
		for (auto& subMesh : m_subMeshes)
		{
			subMesh.Release();
		}
		m_subMeshes.clear();

		m_vertexBuffer->ReleaseGPU();
		m_indexBuffer->ReleaseGPU();
	}

	void Mesh::SetupGPUBuffers()
	{
		m_vertexCount = (u32)m_vertices.size();
		m_indexCount = (u32)m_indices.size();
		m_vertexBuffer->Init(Graphics::GPUBufferDesc::Vertex(sizeof(Vertex), (u32)m_vertices.size(), m_vertices.data()));
		m_indexBuffer->Init(Graphics::GPUBufferDesc::Index(sizeof(u32), (u32)m_indices.size(), m_indices.data()));
	}


	/// <summary>
	/// Model
	/// </summary>
	Model::Model()
	{
		SetType<Model>();
	}

	Model::~Model()
	{
		m_mesh.Release();
	}

	void Model::LoadAsset(std::string path)
	{
		m_absolutePath = std::move(path);
		IS_CORE_INFO("[Model::LoadAsset] Asset loading: '{0}'.", m_absolutePath);
		ModelLoading::AssimpLoader::LoadFromFile(*this, m_absolutePath);
		m_mesh.SetupGPUBuffers();
		IS_CORE_INFO("[Model::LoadAsset] Asset loaded: '{0}'.", m_absolutePath);
	}

	void Model::UnloadAsset()
	{
		m_mesh.Release();
	}
}