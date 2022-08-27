#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include <memory>


#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
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

		struct SubmeshDrawInfo
		{
			u32 Index_Count = 0;
			u32 First_Index = 0;
			u32 Vertex_Offset = 0;
			u32 Vertex_Count = 0;
			glm::mat4 Transform = glm::mat4(1);

			RHI_Buffer* Vertex_Buffer = nullptr;
			RHI_Buffer* Index_Buffer = nullptr;
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
			void SetDrawInfo(SubmeshDrawInfo info);

			u32 GetVertexCount() const { return m_draw_info.Vertex_Count; }
			u32 GetIndexCount() const { return m_draw_info.Index_Count; }

			void Destroy();

		private:
			SubmeshDrawInfo m_draw_info;
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
			void CreateGPUBuffers(const aiScene* scene, std::string_view filePath, std::vector<Vertex>& vertices, std::vector<u32>& indices);
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, std::vector<Vertex>& vertices, std::vector<u32>& indices);
			void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<u32>& indices);
		
		private:
			UPtr<RHI_Buffer> m_vertex_buffer;
			UPtr<RHI_Buffer> m_index_buffer;
			std::vector<Submesh*> m_submeshes;
		};
	}
}