#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include <memory>


#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

namespace Insight
{
	namespace Graphics
	{
		class Mesh;
		class RHI_CommandList;

		struct Vertex
		{
			Vertex() 
			{ }
			Vertex(glm::vec4 pos, glm::vec4 nor, glm::vec4 col, glm::vec2 uv)
				: Position(pos), Normal(nor), Colour(col), UV(uv)
			{ }

			glm::vec4 Position;
			glm::vec4 Normal;
			glm::vec4 Colour;
			glm::vec2 UV;

			constexpr int GetStride() { return sizeof(Vertex); }
		};

		struct SubmeshVertexInfo
		{
			int VertexOffset = 0;
			int VertexCount = 0;
			RHI_Buffer* Buffer = nullptr;
		};

		/// <summary>
		/// Sub mesh. 
		/// </summary>
		class IS_GRAPHICS Submesh
		{
		public:
			Submesh() = delete;
			Submesh(Mesh* mesh) { m_mesh = mesh; }
			~Submesh();

#ifdef RENDER_GRAPH_ENABLED
			void Draw(RHI_CommandList* cmdList) const;
#endif //RENDER_GRAPH_ENABLED
			void SetVertexInfo(SubmeshVertexInfo info);
			void SetIndexBuffer(RHI_Buffer* buffer);

			int GetVertexCount() const { return m_vertexInfo.VertexCount; }
			int GetIndexCount() const { return (int)(m_indexBuffer->GetSize() / sizeof(int)); }

			void Destroy();

		private:
			SubmeshVertexInfo m_vertexInfo;
			UPtr<RHI_Buffer> m_indexBuffer;
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
			void Destroy();
		
			std::vector<Submesh*> GetSubMeshes() const { return m_submeshes; }

#ifdef RENDER_GRAPH_ENABLED
			void Draw(RHI_CommandList* cmdList) const;
#endif //RENDER_GRAPH_ENABLED

		private:
			void CreateGPUBuffers(const aiScene* scene, std::string_view filePath, std::vector<Vertex>& vertices);
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, std::vector<Vertex>& vertices);
			void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<int>& indices);
		
		private:
			UPtr<RHI_Buffer> m_vertexBuffer;
			std::vector<Submesh*> m_submeshes;
		};
	}
}