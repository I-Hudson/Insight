#pragma once
#include "Engine/Templates/TSingleton.h"
#include "glm/glm.hpp"

namespace Insight::Graphics
{
	class GPUCommandBuffer;
}

namespace Insight::Graphics::Debug
{
	enum class DebugObject
	{
		Unknown,
		Instance,
		Physical_Device,
		Device, 
		Queue,
		Seamaphone,
		Command_Buffer,
		Fence, 
		Device_Memory,
		Buffer,
		Image,
		Event,
		Query_Pool,
		Buffer_View,
		Image_View,
		Shader_Module,
		Pipeline_Cache,
		Pipeline_Layout,
		Render_Pass,
		Pipeline,
		Descriptor_Set_Layout,
		Sampler,
		Descriptor_Pool,
		Descriptor_Set,
		Framebuffer,
		Command_Pool,
		Surface,
		Swapchain,
		Debug_Report_Callback,
		Dispaly,
		Display_MOde,
		Validation_Cache,
	};

	class DebugMarker : public Core::TSingleton<DebugMarker>
	{
	public:
		virtual void Init() = 0;

		virtual void SetObjectName(const std::string& name, DebugObject debugObject, u64 objectHandle) = 0;
		virtual void SetObjectTag(const std::string& name, DebugObject debugObject, u64 objectHandle) = 0;

		virtual void BeginRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) = 0;
		virtual void Insert(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) = 0;
		virtual void EndRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) = 0;
	};
}