#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Graphics/GPUBuffer.h"
#include <glm/glm.hpp>

namespace Insight::Graphics
{
	using MeshTextures = std::unordered_map<std::string, std::string>;
	struct RenderList;

	namespace ModelLoading
	{
		struct AssimpLoader;
	}

	class Model;

	struct MeshDimensions
	{
		glm::vec3 Min = glm::vec3(FLOAT_MIN);
		glm::vec3 Max = glm::vec3(FLOAT_MAX);
		glm::vec3 Size = glm::vec3(0);
		glm::vec3 Center = glm::vec3(0);
		float Radius = 0;
	};

	/// <summary>
	/// 
	/// </summary>
	class SubMesh
	{
	public:
		SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount);
		SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount, std::vector<Vertex>& vertices, std::vector<u32>& indices);
		~SubMesh();

		const MeshDimensions& GetDimensions() const { return m_dimensions; }

		const u32& GetFirstVertex() const { return m_firstVertex; }
		const u32& GetVertexCount() const { return m_vertexCount; }
		const u32& GetFirstIndex() const { return m_firstIndex; }
		const u32& GetIndexCount() const { return m_indexCount; }

		std::string GetTexture(const std::string& textureId);

		void Draw(RenderList* drawList);
		GPUBuffer* GetGPUVertexBuffer() const { return m_vertexBuffer; }
		GPUBuffer* GetGPUIndexBuffer() const { return m_indexBuffer; }

		void Release();

	private:
		void SetDimensions(glm::vec3 min, glm::vec3 max);

	private:

		u32 m_firstVertex;
		u32 m_vertexCount;
		u32 m_firstIndex;
		u32 m_indexCount;
		MeshDimensions m_dimensions;

		MeshTextures m_textures;
		GPUBuffer* m_vertexBuffer;
		GPUBuffer* m_indexBuffer;

		friend Insight::Graphics::ModelLoading::AssimpLoader;
	};

	/// <summary>
	/// Store all the data needed for a single mesh. This should include all the sub meshes.
	/// </summary>
	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		const MeshDimensions& GetDimensions() const { return m_dimensions; }

		u32 GetSubMeshCount() const { return static_cast<u32>(m_subMeshes.size()); }
		const SubMesh& GetSubMesh(const u32& index) const { return m_subMeshes.at(index); }

		const u32& GetVertexCount() const { return m_vertexCount; }
		const u32& GetIndexCount() const { return m_indexCount; }

		void Draw(RenderList* drawList);
		GPUBuffer* GetGPUVertexBuffer() const { return m_vertexBuffer; }
		GPUBuffer* GetGPUIndexBuffer() const { return m_indexBuffer; }

		void Release();

	private:
		void SetupGPUBuffers();

	private:
		std::string	m_meshName;

		std::vector<SubMesh> m_subMeshes;
		MeshDimensions m_dimensions;

		std::vector<Vertex> m_vertices;
		std::vector<u32> m_indices;

		u32 m_vertexCount;
		GPUBuffer* m_vertexBuffer;
		u32 m_indexCount;
		GPUBuffer* m_indexBuffer;

		friend Model;
		friend Insight::Graphics::ModelLoading::AssimpLoader;
	};



	/// <summary>
	/// Store all the data needed for a complete model. This should include all the textures and meshes
	/// </summary>
	class Model : public Object
	{
	public:
		Model();
		Model(const std::string& filePath);
		~Model();

		void LoadFromFile(const std::string& filePath);

		const Mesh& GetMesh() const { return m_mesh; }

	private:
		Mesh m_mesh;
		
		std::string m_fileName;
		std::string m_fileDirectory;

		friend Insight::Graphics::ModelLoading::AssimpLoader;
	};
}