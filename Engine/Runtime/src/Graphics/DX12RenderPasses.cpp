#include "Graphics/DX12RenderPasses.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderContext.h"

namespace Insight
{
    namespace Runtime
    {
        Graphics::RHI_Buffer* VertexBuffer = nullptr;
        Graphics::RHI_Buffer* IndexBuffer = nullptr;

        void DX12RenderPasses::Render()
        {
            using namespace Graphics;

            if (!VertexBuffer)
            {
                glm::vec4 vertices[] =
                {
                    glm::vec4(-50.0f, -50.0f, 0.0f, 1.0f),
                    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),

                    glm::vec4(0.0f, 50.0f, 0.0f, 1.0f),
                    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),

                    glm::vec4(50.0f, -50.0f, 0.0f, 1.0f),
                    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
                };
                VertexBuffer = Renderer::CreateVertexBuffer(sizeof(vertices), sizeof(glm::vec4));
                VertexBuffer->QueueUpload(&vertices, sizeof(vertices));
            }

            if (!IndexBuffer)
            {
                u32 indices[] =
                {
                    0, 1, 2
                };
                IndexBuffer = Renderer::CreateIndexBuffer(sizeof(indices));
                IndexBuffer->QueueUpload(&indices, sizeof(indices));
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
                    pso.Swapchain = true;
                    builder.SetPipeline(pso);

                    builder.SetAsRenderToSwapchain();

                    RenderpassDescription renderPassDescriptor = {};
                    renderPassDescriptor.AddAttachment(AttachmentDescription::Load(PixelFormat::Unknown, ImageLayout::ColourAttachment));
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

                    cmdList->DrawIndexed(3, 1, 0, 0, 0);

                    cmdList->EndRenderpass();
                });
        }
    }
}