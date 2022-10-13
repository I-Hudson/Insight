#pragma once

#include "Graphics/Defines.h"
#include "Graphics/RHI/RHI_Buffer.h"
#include "Graphics/RHI/RHI_Texture.h"
#include "Graphics/BoundingBox.h"

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <memory>
#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;
struct aiMaterial;
enum aiTextureType;

namespace Insight
{
	namespace Graphics
	{
		class Mesh;
		class RHI_CommandList;

		struct IS_GRAPHICS Vertex
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


		struct IS_GRAPHICS SubmeshDrawInfo
		{
			u32 Index_Count = 0;
			u32 First_Index = 0;
			u32 Vertex_Offset = 0;
			u32 Vertex_Count = 0;
			glm::mat4 Transform = glm::mat4(1);

			RHI_Buffer* Vertex_Buffer = nullptr;
			RHI_Buffer* Index_Buffer = nullptr;

			std::vector<Ptr<RHI_Texture>> Textures;
		};

		//// <summary>
		//// Sub mesh. 
		//// </summary>
		class IS_GRAPHICS Submesh
		{
		public:
			Submesh() = delete;
			Submesh(Mesh* mesh) { m_mesh = mesh; }
			~Submesh();

			void Draw(RHI_CommandList* cmdList) const;
			void SetDrawInfo(SubmeshDrawInfo info);

			BoundingBox GetBoundingBox() const { return m_bounding_box.Transform(m_draw_info.Transform); }
			u32 GetVertexCount() const { return m_draw_info.Vertex_Count; }
			u32 GetIndexCount() const { return m_draw_info.Index_Count; }

			void Destroy();

		private:
			SubmeshDrawInfo m_draw_info;
			Mesh* m_mesh = nullptr;
			BoundingBox m_bounding_box;

			friend class Mesh;
		};

		//// <summary>
		//// Mesh. Contains the vertex and index buffer for all sub meshes.
		//// </summary>
		class IS_GRAPHICS Mesh
		{
		public:
			Mesh();

			bool LoadFromFile(std::string filePath);
			void Destroy();
		
			std::vector<Submesh*> GetSubMeshes() const { return m_submeshes; }

			void Draw(RHI_CommandList* cmdList) const;

		private:
			void CreateGPUBuffers(const aiScene* scene, std::string_view filePath, std::vector<Vertex>& vertices, std::vector<u32>& indices);
			void ProcessNode(aiNode* aiNode, const aiScene* aiScene, const std::string& directory, std::vector<Vertex>& vertices, std::vector<u32>& indices);
			void ProcessMesh(aiMesh* mesh, const aiScene* aiScene, std::vector<Vertex>& vertices, std::vector<u32>& indices);
			std::vector<Ptr<RHI_Texture>> LoadMaterialTextures(aiMaterial* ai_material, aiTextureType ai_texture_type, const char* texture_id);

			void Optimize(std::vector<Vertex>& src_vertices, std::vector<u32>& src_indices);

		private:
			std::string m_file_path;
			std::string m_file_name;

			UPtr<RHI_Buffer> m_vertex_buffer;
			UPtr<RHI_Buffer> m_index_buffer;
			std::vector<Ptr<RHI_Texture>> m_textures; //TODO This should be a resource Texture not graphics texture later when asset models are implementated.
			std::vector<std::string> m_texture_paths;
			std::vector<Submesh*> m_submeshes;
		};
	}
}