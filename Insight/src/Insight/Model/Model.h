#pragma once

#include "Insight/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
#include "Insight/Model/Mesh.h"
#include "Insight/Object.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "Assimp/mesh.h"
#include "assimp/postprocess.h"

#include "Platform/Vulkan/VulkanBuffer.h"


class IS_API Model : public Insight::Object
{
public:
	Model();
	Model(const std::string& filePath, const std::string& uuid = "");
	~Model();

	Model* Create(const std::string& filepath);

	void Draw(VkCommandBuffer commandBuffer);

	Mesh* GetSubMesh(int index);
	unsigned int GetSubMeshCount() { return static_cast<unsigned int>(m_meshes.size()); }
	const std::string& GetName() const;
	const std::string& GetFilePath() const;

private:
	void LoadMesh(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh* ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

	vks::VulkanBuffer m_vertexBuffer;
	vks::VulkanBuffer m_indexBuffer;
	bool m_buffersBound;

	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_textures;

	std::vector<Mesh*> m_meshes;
	std::vector<Texture> m_texturesLoaded;
	std::string m_directory;
	std::string m_modelName;
	std::string m_path;
};

