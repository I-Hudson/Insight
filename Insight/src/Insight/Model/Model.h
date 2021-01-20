#pragma once

#include "Insight/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
#include "Insight/Model/Mesh.h"
#include "Graphics/Texture.h"
#include "Insight/Core/Object.h"

#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"

using Texture = Insight::Render::Texture;

class IS_API Model : public Insight::Object
{
public:
	Model();
	Model(const std::string& filePath, const std::string& uuid = "");
	~Model();

	void Create(const std::string& filepath);
	//void Draw(VkCommandBuffer commandBuffer);

	WeakPtr<Mesh> GetMesh();
	const std::vector<WeakPtr<Material>> GetMaterals();

	const std::string& GetFilePath() const;
	const std::string& GetModelName() const;

	void SetMaterials(const std::vector<std::pair<std::string, SharedPtr<Texture>>>& textures);
private:

	SharedPtr<Mesh> m_mesh;
	std::vector<SharedPtr<Material>> m_materials;
	std::vector<SharedPtr<Texture>> m_textures;
	std::string m_directory;
	std::string m_modelName;
	std::string m_path;

	friend class Mesh;
};

