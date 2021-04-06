#pragma once

#include "Engine/Core/Core.h"
#include "Platform/Vulkan/VulkanHeader.h"
#include "Engine/Model/Mesh.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Core/Object.h"

#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"

using Texture = Render::Texture;

class IS_API Model : public Object
{
public:
	Model();
	Model(const std::string& filePath, const std::string& uuid = "");
	~Model();

	void Create(const std::string& filepath);
	//void Draw(VkCommandBuffer commandBuffer);

	Mesh* GetMesh();
	const std::vector<Material*>& GetMaterals();

	const std::string& GetFilePath() const;
	const std::string& GetModelName() const;

	void SetMaterials(const std::vector<std::pair<std::string, Texture*>>& textures);
private:

	Mesh* m_mesh;
	std::vector<Material*> m_materials;
	std::vector<Texture*> m_textures;
	std::string m_directory;
	std::string m_modelName;
	std::string m_path;

	friend class Mesh;
};

