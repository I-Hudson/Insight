#include "Graphics/RenderStats.h"
#include "Graphics/RenderContext.h"

#include <imgui.h>

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

			std::string vendor_name = Platform::StringFromWString(PhysicalDeviceInformation::Instance().Vendor);
			std::string device_name = Platform::StringFromWString(PhysicalDeviceInformation::Instance().Device_Name);
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
			ImGui::Text(DescriptorSetBindingsFormated().c_str());
			ImGui::Text(DescriptorSetUpdatesFormated().c_str());
			ImGui::Text(DescriptorSetUsedCountFormated().c_str());
			ImGui::Text(PipelineBarriersFormated().c_str());

			ImGui::End();
			Reset();
		}

		void RenderStats::Reset()
		{
			RenderTime.Reset();
			DrawCalls = 0;
			DrawIndexedCalls = 0;
			DispatchCalls = 0;
			IndexBufferBindings = 0;
			VertexBufferBindings = 0;
			DescriptorSetBindings = 0;
			DescriptorSetUpdates = 0;
			DescriptorSetUsedCount = 0;
			PipelineBarriers = 0;
		}
	}
}