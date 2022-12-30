#include "Graphics/DX12RenderPasses.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderContext.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

namespace Insight
{
    namespace Runtime
    {
        Graphics::RHI_Buffer* VertexBuffer = nullptr;
        Graphics::RHI_Buffer* IndexBuffer = nullptr;

        struct Vertex
        {
            glm::vec4 Position;
            glm::vec4 Colour;
        };

        struct alignas(16) UBO
        {
            glm::vec4 Transform;
            glm::vec4 OverrideColour;
            int Override;
        };

        void DX12RenderPasses::Render()
        {
            using namespace Graphics;

            if (!VertexBuffer)
            {
                Vertex vertices[] =
                {
                    Vertex { { 0.0f,   0.25f, 0.25f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f} },
                    Vertex { { 0.25f, -0.25f, 0.25f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f} },
                    Vertex { {-0.25f, -0.25f, 0.25f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f} },
                };
                VertexBuffer = Renderer::CreateVertexBuffer(sizeof(vertices), sizeof(Vertex));
                VertexBuffer->QueueUpload(&vertices, sizeof(vertices));
                VertexBuffer->SetName("TestVertexBuffer");
            }

            if (!IndexBuffer)
            {
                u32 indices[] =
                {
                    0, 1, 2
                };
                IndexBuffer = Renderer::CreateIndexBuffer(sizeof(indices));
                IndexBuffer->QueueUpload(&indices, sizeof(indices));
                IndexBuffer->SetName("TestIndexBuffer");
            }

            struct PassData
            { };
            RenderGraph::Instance().AddPass<PassData>("TestPass",
                [&](PassData& data, RenderGraphBuilder& builder)
                {
                    builder.WriteTexture(-1);
            
                    ShaderDesc shaders = {};
                    shaders.VertexFilePath = "./Resources/Shaders/hlsl/TestPassDX12.hlsl";
                    shaders.PixelFilePath = "./Resources/Shaders/hlsl/TestPassDX12.hlsl";
                    builder.SetShader(shaders);

                    PipelineStateObject pso = { };
                    pso.Name = "TestPass_PSO";
                    pso.ShaderDescription = shaders;
                    pso.DepthWrite = false;
                    pso.DepthTest = false;
                    pso.Swapchain = true;
                    builder.SetPipeline(pso);

                    builder.SetAsRenderToSwapchain();

                    RenderpassDescription renderPassDescriptor = {};
                    renderPassDescriptor.AddAttachment(AttachmentDescription::Default(PixelFormat::Unknown, ImageLayout::ColourAttachment));
                    builder.SetRenderpass(renderPassDescriptor);

                    builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                    builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                },
                [&](PassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
                {
                    if (VertexBuffer->GetUploadStatus() != DeviceUploadStatus::Completed
                    || IndexBuffer->GetUploadStatus() != DeviceUploadStatus::Completed)
                    {
                        return;
                    }

                    cmdList->BindPipeline(renderGraph.GetPipelineStateObject("TestPass"), nullptr);
                    cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("TestPass"));

                    cmdList->SetVertexBuffer(VertexBuffer);
                    cmdList->SetIndexBuffer(IndexBuffer, IndexType::Uint32);

                    UBO uniform
                    {
                        glm::vec4(0, 0, 0, 0),
                        glm::vec4(1, 1, 1, 1),
                        Input::InputSystem::Instance().GetKeyboardMouseDevice()->WasHeld(Input::KeyboardButtons::Key_Space) ? 1 : 0
                    };

                    cmdList->SetUniform(0, 0, uniform);
                    cmdList->DrawIndexed(3, 1, 0, 0, 0);

                    uniform.Transform = glm::vec4(-0.75f, 0, 0, 0);
                    uniform.OverrideColour = glm::vec4(1.0f, 0.5f, 0.16f, 1);
                    cmdList->SetUniform(0, 0, uniform);
                    cmdList->DrawIndexed(3, 1, 0, 0, 0);

                    uniform.Transform = glm::vec4(0.75f, 0, 0, 0);
                    uniform.OverrideColour = glm::vec4(0.5f, 0.23f, 0.85f, 1);
                    cmdList->SetUniform(0, 0, uniform);
                    cmdList->DrawIndexed(3, 1, 0, 0, 0);

                    cmdList->EndRenderpass();
                });
        }
    }
}