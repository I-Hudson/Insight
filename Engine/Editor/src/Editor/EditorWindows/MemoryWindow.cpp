#include "Editor/EditorWindows/MemoryWindow.h"

#include "Core/Defines.h"
#include "Core/Memory.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include <implot.h>

namespace Insight
{
    namespace Editor
    {
        MemoryWindow::MemoryWindow()
            : IEditorWindow()
        { }

        MemoryWindow::MemoryWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        MemoryWindow::MemoryWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        MemoryWindow::~MemoryWindow()
        { }

        void MemoryWindow::OnDraw()
        {
            static const char* memoryUnits[] = { "8192 KB", "4096 KB", "2048 KB", "1024 KB", "512 KB", 
                                                 "256 KB", "128 KB", "64 KB", "32 KB", "16 KB", "8 KB", "4 KB", "2 KB", "0 KB" };
            struct OneMB
            {
                Byte B[1_MB];
            };
            static OneMB* oneMBPtr = nullptr;
            if (Input::InputSystem::Instance().GetKeyboardMouseDevice()->WasPressed(Input::KeyboardButtons::Key_Enter))
            {
                if (oneMBPtr)
                {
                    Delete(oneMBPtr);
                }
                else
                {
                    oneMBPtr = New<OneMB, Core::MemoryAllocCategory::Input>();
                }
            }

            u64 currentMemoryUsageForCategory[static_cast<u64>(Core::MemoryAllocCategory::Size)];
            u64 currentAllocationsForCategory[static_cast<u64>(Core::MemoryAllocCategory::Size)];

            for (size_t i = 0; i < static_cast<u64>(Core::MemoryAllocCategory::Size); ++i)
            {
                Core::MemoryAllocCategory category = static_cast<Core::MemoryAllocCategory>(i);
                m_memoryUsage.SetCategoryFrameValue(category, Core::MemoryTracker::Instance().GetUsage(category));
                
                currentMemoryUsageForCategory[i] = m_memoryUsage.GetCategoryFrameValue(category) / 1024;
                currentAllocationsForCategory[i] = Core::MemoryTracker::Instance().GetTotalNumberOfAllocationsForCategory(category);
            }

            if (ImPlot::BeginPlot("Memory Usage"))
            {
                ImPlot::SetupAxes("Memory Category", "Memory Usage KB", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisTicks(ImAxis_X1, 0, ARRAY_COUNT(Core::MemoryAllocCategoryToString) - 1, ARRAY_COUNT(Core::MemoryAllocCategoryToString), Core::MemoryAllocCategoryToString, false);
                ImPlot::PlotBars("##MemoryCategory", currentMemoryUsageForCategory, ARRAY_COUNT(currentMemoryUsageForCategory), 0.75);
                ImPlot::EndPlot();
            }

            if (ImPlot::BeginPlot("Memory Allocations"))
            {
                ImPlot::SetupAxes("Memory Category", "Allocation Count", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxisTicks(ImAxis_X1, 0, ARRAY_COUNT(Core::MemoryAllocCategoryToString) - 1, ARRAY_COUNT(Core::MemoryAllocCategoryToString), Core::MemoryAllocCategoryToString, false);
                ImPlot::PlotBars("##MemoryCategory", currentAllocationsForCategory, ARRAY_COUNT(currentAllocationsForCategory), 0.75);
                ImPlot::EndPlot();
            }

            ImGui::Text("Total number of allocations: %u.", Core::MemoryTracker::Instance().GetTotalNumberOfAllocations());
            ImGui::Text("Total allocated amount KB: %u.", Core::MemoryTracker::Instance().GetTotalAllocatedInBytes() / 1024);

            m_memoryUsage.AdvanceFrame();
        }
    }
}