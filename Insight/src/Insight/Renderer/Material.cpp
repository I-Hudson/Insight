#include "ispch.h"
#include "Material.h"

#include "Insight/Memory/MemoryManager.h"
#include "Insight/Renderer/VulkanMaterial.h"


Material* Material::Create()
{
	return Insight::Memory::MemoryManager::NewOnFreeList<Insight::Render::VulkanMaterial>();
}
