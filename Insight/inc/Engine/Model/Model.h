#pragma once

#include "Engine/Core/Common.h"
#include "Engine/Assets/Asset.h"
#include "Engine/Graphics/GPUBuffer.h"
#include "Engine/Core/Maths/Frustum.h"
#include "Engine/Model/Bone.h"
#include "Engine/Model/Animation.h"
#include <glm/glm.hpp>

class MeshComponent;

namespace Insight
{
	namespace Graphics
	{
		struct RenderListView;
	}
	namespace ModelLoading
	{
		struct AssimpLoader;
		struct GltfLoader;
	}

	using MeshTextures = std::unordered_map<std::string, std::string>;
	class Model;

	struct MeshDimensions
	{
		glm::vec3 Min = glm::vec3(FLOAT_MAX);
		glm::vec3 Max = glm::vec3(FLOAT_MIN);
		glm::vec3 Size = glm::vec3(0);
		glm::vec3 Center = glm::vec3(0);
		float Radius = 0;
	};

	enum class MaterialTextureType
	{
		Diffuse,
		Normal,
	};

	/// <summary>
	/// Single sub mesh. Stores all data in relation to this sub mesh. Currently this holds it's
	/// own vertex and index buffer but this should be move to it's parent Mesh class.
	/// </summary>
	class SubMesh
	{
	public:
		SubMesh(u32 firstVertex, u32 vertexCount, u32 firstIndex, u32 indexCount, Graphics::GPUBuffer* vertexBuffer, Graphics::GPUBuffer* indexBuffer);
		~SubMesh();

		const MeshDimensions& GetDimensions() const { return m_dimensions; }

		const u32& GetFirstVertex() const { return m_firstVertex; }
		const u32& GetVertexCount() const { return m_vertexCount; }
		const u32& GetFirstIndex() const { return m_firstIndex; }
		const u32& GetIndexCount() const { return m_indexCount; }

		std::string GetTexture(const std::string& textureId);
		std::string GetTexture(MaterialTextureType type);

		void Draw(Graphics::RenderListView* drawList, const glm::mat4& worldTransform, const Maths::Frustum& cameraFrustum);
		void Release();

	private:
		void SetDimensions(glm::vec3 min, glm::vec3 max);

	private:
		u32 m_firstVertex;
		u32 m_vertexCount;
		u32 m_firstIndex;
		u32 m_indexCount;
		MeshDimensions m_dimensions;
		std::unordered_map<MaterialTextureType, std::string> m_textureStrings;

		MeshTextures m_textures;
		bool m_releaseGPUBuffers;
		bool m_isSkinnedMesh;

		Graphics::GPUBuffer* m_vertexBuffer;
		Graphics::GPUBuffer* m_indexBuffer;

		friend Insight::ModelLoading::AssimpLoader;
		friend Insight::ModelLoading::GltfLoader;
	};

	/// <summary>
	/// Store all the data needed for a single mesh. This should include all the sub meshes.
	/// </summary>
	class Mesh
	{
	public:
		Mesh();
		~Mesh(); 

		//Model& operator== (const Model& other) = default;
		//Model& operator== (Model&& other) = default;

		const MeshDimensions& GetDimensions() const { return m_dimensions; }

		u32 GetSubMeshCount() const { return static_cast<u32>(m_subMeshes.size()); }
		const SubMesh& GetSubMesh(const u32& index) const { return m_subMeshes.at(index); }
		std::vector<std::string> GetAllSubMeshTextures(MaterialTextureType type);

		const u32& GetVertexCount() const { return m_vertexCount; }
		const u32& GetIndexCount() const { return m_indexCount; }

		void SetVerticesAndIndcies(const std::vector<Vertex>& vertices, const std::vector<u32>& indcies);

		Animation::Skeleton& GetSkeleton() { return m_skeleton; }
		Animation::Animation* GetAnimation(u32 index) { return &m_animations.at(index); }

		void Release();

	private:
		void Draw(Graphics::RenderListView* drawList, const glm::mat4& worldTransform, const Maths::Frustum& cameraFrustum);
		void SetupGPUBuffers();

	private:
		std::string	m_meshName;

		std::vector<SubMesh> m_subMeshes;
		Mesh::MeshDimensions m_dimensions;

		Animation::Skeleton m_skeleton;
		std::vector<Animation::Animation> m_animations;
		bool m_isSkinnedMesh;

		std::vector<Vertex> m_vertices;
		std::vector<u32> m_indices;

		u32 m_vertexCount = 0;
		Graphics::GPUBuffer* m_vertexBuffer = nullptr;
		u32 m_indexCount = 0;
		Graphics::GPUBuffer* m_indexBuffer = nullptr;

		friend Model;
		friend MeshComponent;
		friend Insight::ModelLoading::AssimpLoader;
		friend Insight::ModelLoading::GltfLoader;
	};



	/// <summary>
	/// Store all the data needed for a complete model. This should include all the textures and meshes
	/// </summary>
	class Model : public Assets::Asset
	{
	public:
		Model();
		~Model();

		Mesh& GetMesh() { return m_mesh; }

	protected:
		virtual void LoadAsset(std::string path) override;
		virtual void UnloadAsset() override;

	private:
		Mesh m_mesh;
		friend Insight::ModelLoading::AssimpLoader;
		friend Insight::ModelLoading::GltfLoader;
	};
}