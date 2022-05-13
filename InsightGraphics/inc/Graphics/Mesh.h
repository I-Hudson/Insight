#pragma once

#include "Core/TypeAlias.h"
#include "Graphics/Defines.h"
#include "Core/Memory.h"

#include "Graphics/RHI/RHI_Buffer.h"

#include "glm/glm.hpp"

struct aiNode;
struct aiScene;

namespace Insight
{
	namespace Graphics
	{
		class Mesh;

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Colour;
		};

		/// <summary>
		/// Sub mesh. 
		/// </summary>
		class IS_GRAPHICS Submesh
		{
		public:
			Submesh() = delete;
			Submesh(Mesh* mesh) { m_mesh = mesh; }

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

		private:
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
			void CreateGPUBuffers(const aiScene* scene);

		private:
			UPtr<RHI_Buffer> m_vertexBuffer;
			UPtr<RHI_Buffer> m_indexBuffer;
		};
	}
}