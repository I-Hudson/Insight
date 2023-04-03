#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Core/TypeAlias.h"
#include "Core/MemoryTracker.h"

#include <array>

namespace Insight
{
    namespace Editor
    {
        struct MemoryUsageCategoryFrame
        {
            constexpr static u64 c_FrameCount = 100;
            std::array<u64, c_FrameCount> m_frames = {0};

            void SetFrameValue(u64 v, u64 frameIndex)
            {
                m_frames[frameIndex] = v;
            }
        };

        struct MemoryUsage
        {
            u64 m_currentFrameIndex = 0;
            std::array<MemoryUsageCategoryFrame, static_cast<u64>(Core::MemoryAllocCategory::Size)> m_categories;

            void SetCategoryFrameValue(Core::MemoryAllocCategory category, u64 v)
            {
                m_categories[static_cast<u64>(category)].SetFrameValue(v, m_currentFrameIndex);
            }

            u64 GetCategoryFrameValue(Core::MemoryAllocCategory category)
            {
                return m_categories[static_cast<u64>(category)].m_frames[m_currentFrameIndex];
            }

            void AdvanceFrame()
            {
                m_currentFrameIndex = ++m_currentFrameIndex % MemoryUsageCategoryFrame::c_FrameCount;
            }
        };

        class MemoryWindow : public IEditorWindow
        {
        public:
            MemoryWindow();
            MemoryWindow(u32 minWidth, u32 minHeight);
            MemoryWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            virtual ~MemoryWindow() override;

            EDITOR_WINDOW(MemoryWindow, EditorWindowCategories::Windows);
            virtual void OnDraw() override;

        private:
            MemoryUsage m_memoryUsage;
        };
    }
}