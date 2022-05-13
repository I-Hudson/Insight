#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Defines.h"
#include "Core/Memory.h"

#include "Graphics/RHI/RHI_Buffer.h"

#include "glm/glm.hpp"
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Insight
{
	namespace Graphics
	{
		class Mesh;

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec3 Colour;
			glm::vec2 UV;

			constexpr int GetStride() { return sizeof(Vertex); }
		};

		/// <summary>
		/// Sub mesh. 
		/// </summary>
		class IS_GRAPHICS Submesh
		{
		public:
			Submesh() = delete;
			Submesh(Mesh* mesh) { m_mesh = mesh; }

			void Draw() const;

			void SetVertexView(RHI_BufferView view) { m_indexView = view; }
			void SetIndexView(RHI_BufferView view) { m_indexView = view; }

			RHI_BufferView GetVertexView() const { return m_vertexView; }
			RHI_BufferView GetIndexView() const { return m_indexView; }

			int GetVertexCount() const { return m_vertexView.GetSize() / sizeof(Vertex); }
			int GetIndexCount() const { return m_indexView.GetSize() / sizeof(int); }

		private:
			RHI_BufferView m_vertexView;
			RHI_BufferView m_indexView;
			Mesh* m_mesh = nullptr;
		};

		/// <summary>
		/// Mesh. Contains the vertex and index buffer for all sub meshes.
		/// </summary>
		class IS_GRAPHICS Mesh
		{
		public:
			Mesh();

			bool LoadFromFile(std::string filePath);

			void Draw() const;

		private:
			void CreateGPUBuffers(const aiScene* scene, std::string_view filePath);
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
			void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<int>& indices);

		private:
			std::vector<Submesh> m_submeshes;

			UPtr<RHI_Buffer> m_vertexBuffer;
			UPtr<RHI_Buffer> m_indexBuffer;
		};
	}
}