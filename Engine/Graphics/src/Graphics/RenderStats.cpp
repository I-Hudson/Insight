#include "Graphics/RenderStats.h"
#include "Graphics/RenderContext.h"

#include "Core/Profiler.h"

#include <imgui.h>

#include <sstream>

std::string FormatU64ToCommaString(u64 value)
{
    if constexpr (false)
    {
        std::stringstream ss;
        ss.imbue(std::locale(std::locale(), new comma_numpunct()));
        ss << std::fixed << value;
        return ss.str();
    }
    else
    {
        if (value == 0)
        {
            return "";
        }

        std::function<void(std::string& str, u64 digitSize, u64 value)> placeDigitInToString =
            [](std::string& str, u64 digitSize, u64 value)
        {
            if (digitSize > 0 && digitSize % 3 == 0)
            {
                str.push_back(',');
            }
            const char c = '0' + static_cast<char>(value);
            str.push_back(c);
        };

        std::string str;
        u64 strDigitSize = 0;
        while (value > 10)
        {
            const u64 lastDigit = value % 10;
            value /= 10;
            placeDigitInToString(str, strDigitSize, lastDigit);
            ++strDigitSize;
        }
        u64 lastDigit = value % 10;
        placeDigitInToString(str, strDigitSize, lastDigit);
        std::reverse(str.begin(), str.end());
        return str;
    }
}

namespace Insight
{
    namespace Graphics
    {
        void RenderStats::Draw()
        {
            IS_PROFILE_FUNCTION();

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

            {
                IS_PROFILE_SCOPE("IMGUI");
                ImGui::Begin("Render Stats");

                ImGui::Text("Vendor: %s", vendor_name.data());
                ImGui::Text("Device: %s", device_name.data());
                ImGui::Text("VRAM: %u MB", PhysicalDeviceInformation::Instance().VRam_Size / 1024 / 1024);
                if (ImGui::CollapsingHeader("VRam"))
                {
                    RHI_MemoryInfo rhiMemoryInfo = RenderContext::Instance().GetVRamInfo();
                    ImGui::Text("   Block Count: %u",          rhiMemoryInfo.BlockCount);
                    ImGui::SetItemTooltip("Number of D3D12 memory blocks allocated - `ID3D12Heap` objects and committed resources.");

                    ImGui::Text("   Allocation Count: %u",     rhiMemoryInfo.AllocationCount);
                    ImGui::SetItemTooltip("Number of D3D12MA::Allocation objects allocated.");

                    ImGui::Text("   Block Bytes: %u MB",       rhiMemoryInfo.BlockBytes / 1024 / 1024);
                    ImGui::SetItemTooltip("Number of bytes allocated in memory blocks.");

                    ImGui::Text("   Allocation Bytes: %u MB",  rhiMemoryInfo.AllocationBytes / 1024 / 1024);
                    ImGui::SetItemTooltip("Total number of bytes occupied by all D3D12MA::Allocation objects.");

                    ImGui::Text("   Free Bytes: %u MB",        (rhiMemoryInfo.BlockBytes - rhiMemoryInfo.AllocationBytes) / 1024 / 1024);
                    ImGui::SetItemTooltip("Number of bytes currently not in use. BlockBytes - AllocationBytes.");

                    ImGui::Text("   Est Usage Bytes: %u MB",       rhiMemoryInfo.UsageBytes / 1024 / 1024);
                    ImGui::SetItemTooltip("Estimated current memory usage of the program.");

                    ImGui::Text("   Est Budget Bytes: %u MB",      rhiMemoryInfo.BudgetBytes / 1024 / 1024);
                    ImGui::SetItemTooltip("Estimated amount of memory available to the program.");

                }

                ImGui::Text("Render Timer: %f", renderTime);
                ImGui::Text("Average Render Timer: %f", averageRenderTimer);
                ImGui::Text("Render Fps: %f", fps);

                ImGui::Text(MeshCountFormated().c_str());
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
            Reset();
        }

        void RenderStats::Reset()
        {
            RenderTime.Reset();

            //MeshCount.Swap();
            MeshCount = 0;

            //DrawCalls.Swap();
            DrawCalls = 0;

            //DrawIndexedCalls.Swap();
            DrawIndexedCalls = 0;

            //DispatchCalls.Swap();
            DispatchCalls = 0;

            //IndexBufferBindings.Swap();
            IndexBufferBindings = 0;

            //VertexBufferBindings.Swap();
            VertexBufferBindings = 0;

            //DrawIndexedIndicesCount.Swap();
            DrawIndexedIndicesCount = 0;

            //FrameUniformBufferSize.Swap();
            FrameUniformBufferSize = 0;

            //DescriptorSetBindings.Swap();
            DescriptorSetBindings = 0;

            //DescriptorSetUpdates.Swap();
            DescriptorSetUpdates = 0;

            //DescriptorSetUsedCount.Swap();
            DescriptorSetUsedCount = 0;

            //PipelineBarriers.Swap();
            PipelineBarriers = 0;

            //DescriptorTableResourceCreations.Swap();
            DescriptorTableResourceCreations = 0;

            //DescriptorTableResourceReuse.Swap();
            DescriptorTableResourceReuse = 0;

            //DescriptorTableSamplerCreations.Swap();
            DescriptorTableSamplerCreations = 0;

            //DescriptorTableSamplerReuse.Swap();
            DescriptorTableSamplerReuse = 0;
        }
    }
}
