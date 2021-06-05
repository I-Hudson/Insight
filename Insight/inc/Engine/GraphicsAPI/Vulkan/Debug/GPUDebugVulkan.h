#pragma once

#include "Engine/Graphics/Debug/GPUDebug.h"

namespace Insight::GraphicsAPI::Vulkan
{
	class GPUDebugMarkerVulkan : public Graphics::Debug::DebugMarker
	{
		virtual void Init() override;

		virtual void SetObjectName(const std::string& name, Graphics::Debug::DebugObject debugObject, u64 objectHandle) override;
		virtual void SetObjectTag(const std::string& name, Graphics::Debug::DebugObject debugObject, u64 objectHandle) override;

		virtual void BeginRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) override;
		virtual void Insert(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) override;
		virtual void EndRegion(Graphics::GPUCommandBuffer* commandBuffer, const std::string& name, glm::vec4 colour) override;
	};
}