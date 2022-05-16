#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include <memory>


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

		/// <summary>
		/// Sub mesh. 
		/// </summary>
		class IS_GRAPHICS Submesh
		{
		public:
			Submesh() = delete;
			Submesh(Mesh* mesh) { m_mesh = mesh; }
			~Submesh();

			void Draw() const;

			void SetVertexBuffer(RHI_Buffer* buffer);
			void SetIndexBuffer(RHI_Buffer* buffer);

			int GetVertexCount() const { return (int)(m_vertexBuffer->GetSize() / sizeof(Vertex)); }
			int GetIndexCount() const { return (int)(m_indexBuffer->GetSize() / sizeof(int)); }

			void Destroy();

		private:
			UPtr<RHI_Buffer> m_vertexBuffer;
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
		
			void Draw() const;
		
		private:
			void CreateGPUBuffers(const aiScene* scene, std::string_view filePath);
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory);
			void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<int>& indices);
		
		private:
			std::vector<Submesh*> m_submeshes;
		};
	}
}