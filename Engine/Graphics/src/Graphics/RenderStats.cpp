#include "Graphics/RenderStats.h"
#include "Graphics/RenderContext.h"

#include <imgui.h>

std::string FormatU64ToCommaString(const Insight::DoubleBufferVector<u64>& value)
{
	std::stringstream ss;
	ss.imbue(std::locale(std::locale(), new comma_numpunct()));
	ss << std::fixed << value.GetCurrent();
	return ss.str();
};

namespace Insight
{
	namespace Graphics
	{
		void RenderStats::Draw()
		{
			ImGui::Begin("Render Stats");

			RenderTime.Stop();

			u32 rawRenderTime = static_cast<u32>(RenderTime.GetElapsedTimeMill().count());
			float floatRenderTime = static_cast<float>(rawRenderTime);
			float renderTime = floatRenderTime / 1000.0f;
			float fps = 1000.0f / static_cast<float>(rawRenderTime);

			AverageRenderTime[AverageRenderTimeIndex] = renderTime;
			AverageRenderTimeIndex = (AverageRenderTimeIndex + 1) % AverageRenderTimeCount;

			float averageRenderTimer = 0.0f;
			for (size_t i = 0; i < AverageRenderTimeCount; ++i)
			{
				averageRenderTimer += AverageRenderTime[i];
			}
			averageRenderTimer = averageRenderTimer / AverageRenderTimeCount;

			std::string vendor_name = PhysicalDeviceInformation::Instance().Vendor;
			std::string device_name = PhysicalDeviceInformation::Instance().Device_Name;
			ImGui::Text("Vendor: %s", vendor_name.data());
			ImGui::Text("Device: %s", device_name.data());
			ImGui::Text("VRAM: %u MB", PhysicalDeviceInformation::Instance().VRam_Size / 1024 / 1024);

			ImGui::Text("Render Timer: %f", renderTime);
			ImGui::Text("Average Render Timer: %f", averageRenderTimer);
			ImGui::Text("Render Fps: %f", fps);

			ImGui::Text(DrawCallsFormated().c_str());
			ImGui::Text(DrawIndexedCallsFormated().c_str());
			ImGui::Text(DispatchCallsFormated().c_str());
			ImGui::Text(IndexBufferBindingsFormated().c_str());
			ImGui::Text(VertexBufferBindingsFormated().c_str());
			ImGui::Text(DrawIndexedIndicesCountFormated().c_str());
			ImGui::Text(FrameUniformBufferSizeFormated().c_str());
			ImGui::Text(DescriptorSetBindingsFormated().c_str());
			ImGui::Text(DescriptorSetUpdatesFormated().c_str());
			ImGui::Text(DescriptorSetUsedCountFormated().c_str());
			ImGui::Text(PipelineBarriersFormated().c_str());

			if (RenderContext::Instance().GetGraphicsAPI() == GraphicsAPI::DX12)
			{
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Text("DX12");
				ImGui::Separator();
				ImGui::Text(DescriptorTableResourceCreationsFormated().c_str());
				ImGui::Text(DescriptorTableResourceReuseFormated().c_str());
				ImGui::Text(DescriptorTableSamplerCreationsFormated().c_str());
				ImGui::Text(DescriptorTableSamplerReuseFormated().c_str());
			}

			ImGui::End();
		}

		void RenderStats::Reset()
		{
			RenderTime.Reset();
			DrawCalls.Swap();
			DrawCalls = 0;

			DrawIndexedCalls.Swap();
			DrawIndexedCalls = 0;

			DispatchCalls.Swap();
			DispatchCalls = 0;

			IndexBufferBindings.Swap();
			IndexBufferBindings = 0;

			VertexBufferBindings.Swap();
			VertexBufferBindings = 0;

			DrawIndexedIndicesCount.Swap();
			DrawIndexedIndicesCount = 0;

			FrameUniformBufferSize.Swap();
			FrameUniformBufferSize = 0;

			DescriptorSetBindings.Swap();
			DescriptorSetBindings = 0;

			DescriptorSetUpdates.Swap();
			DescriptorSetUpdates = 0;

			DescriptorSetUsedCount.Swap();
			DescriptorSetUsedCount = 0;

			PipelineBarriers.Swap();
			PipelineBarriers = 0;

			DescriptorTableResourceCreations.Swap();
			DescriptorTableResourceCreations = 0;

			DescriptorTableResourceReuse.Swap();
			DescriptorTableResourceReuse = 0;

			DescriptorTableSamplerCreations.Swap();
			DescriptorTableSamplerCreations = 0;

			DescriptorTableSamplerReuse.Swap();
			DescriptorTableSamplerReuse = 0;
		}
	}
}
