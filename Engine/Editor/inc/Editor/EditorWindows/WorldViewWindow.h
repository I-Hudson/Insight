#pragma once

#include "Editor/EditorWindows/IEditorWindow.h"

#include "Graphics/RenderFrame.h"
#include "Renderpass.h"

#include "Editor/EditorWindows/Generated/WorldViewWindow_reflect_generated.h"

namespace Insight
{
    namespace Editor
    {
        struct RenderData
        {
            RenderFrame RenderFrame;
            Graphics::BufferFrame BufferFrame;
            Graphics::BufferSamplers BufferSamplers;

            Graphics::RHI_BufferView FrameView;
        };

        REFLECT_CLASS()
        class WorldViewWindow : public IEditorWindow
        {
            REFLECT_GENERATED_BODY()

        public:
            WorldViewWindow();
            WorldViewWindow(u32 minWidth, u32 minHeight);
            WorldViewWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight);
            ~WorldViewWindow();

            virtual void Initialise() override;
            virtual void OnDraw() override;
            EDITOR_WINDOW(WorldViewWindow, EditorWindowCategories::Windows);

        private:
            void ContentWindowDragTarget();
            
            void SetupRenderGraphPasses();
            void GBufferDepthPrepass();
            void GBufferPass();
            void TransparentGBufferPass();

            void BindCommonResources(Graphics::RHI_CommandList* cmd_list, RenderData& renderData);

            Graphics::BufferFrame GetBufferFrame() const;
            Graphics::BufferSamplers GetBufferSamplers() const;

        private:
            constexpr static const char* c_WorldName = "EditorWorldView";
            ECS::Entity* m_editorCameraEntity;
            ECS::CameraComponent* m_editorCameraComponent;

            bool m_enableDepthPrepass = false;
            RenderData m_renderingData;
        };
    }
}