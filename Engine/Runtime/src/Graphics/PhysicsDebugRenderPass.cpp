#include "Graphics/PhysicsDebugRenderPass.h"
#include "Graphics/Vertex.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderGraph/RenderGraphBuilder.h"

#include "Asset/AssetRegistry.h"
#include "Core/EnginePaths.h"

#include "Physics/PhysicsWorld.h"

namespace Insight
{
    namespace Graphics
    {
        void PhysicsDebugRenderPass::Create()
        {
            RHI_Buffer_Overrides buffer_overrides = {};
            buffer_overrides.Force_Host_Writeable = true;
            if (m_vertex_buffer.Size() == 0)
            {
                m_vertex_buffer.Setup();
                m_vertex_buffer.ForEach([buffer_overrides](RHI_Buffer*& buffer)
                    {
                        buffer = Renderer::CreateVertexBuffer(sizeof(Physics::DebugRendererData::Vertex) * 1024, sizeof(Physics::DebugRendererData::Vertex), buffer_overrides);
                    });
            }
            if (m_index_buffer.Size() == 0)
            {
                m_index_buffer.Setup();
                m_index_buffer.ForEach([buffer_overrides](RHI_Buffer*& buffer)
                    {
                        buffer = Renderer::CreateIndexBuffer(sizeof(u16) * 1024, buffer_overrides);
                    });
            }

            ShaderDesc shaderDesc("PhysicsDebugPass_LineShader", EnginePaths::GetResourcePath() + "/Shaders/hlsl/PhysicsDebugLine.hlsl"
                , ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
            shaderDesc.InputLayout = 
            {
                ShaderInputLayout(0, PixelFormat::R32G32B32A32_Float, 0, 0, "POSITION"),
                ShaderInputLayout(1, PixelFormat::R32G32B32A32_Float, 0, 16, "COLOR"),
                ShaderInputLayout(2, PixelFormat::R32G32_Float, 0, 32, "TEXCOORD0")
            };
            RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);

            ShaderDesc triangleShaderDesc("PhysicsDebugPass_TriangleShader", EnginePaths::GetResourcePath() + "/Shaders/hlsl/PhysicsDebugTriangle.hlsl"
                , ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
            triangleShaderDesc.InputLayout =
            {
                ShaderInputLayout(0, PixelFormat::R32G32B32A32_Float, 0, 0, "POSITION"),
                ShaderInputLayout(1, PixelFormat::R32G32B32A32_Float, 0, 16, "COLOR"),
                ShaderInputLayout(2, PixelFormat::R32G32_Float, 0, 32, "TEXCOORD0")
            };
            RenderContext::Instance().GetShaderManager().GetOrCreateShader(triangleShaderDesc);
        }

        void PhysicsDebugRenderPass::Render(ConstantBuffer constantBuffer, std::string_view colourTextureName, std::string_view depthTextureName)
        {
            struct PassData
            {
                Physics::DebugRendererData RenderData;
                std::string ColourTextureName;
                std::string DepthTextureName;
                ConstantBuffer ConstantBuffer;
            };
            PassData passData
            {
                Physics::PhysicsWorld::GetDebugRenderData(),
                colourTextureName.data(),
                depthTextureName.data(),
                std::move(constantBuffer)
            };

            RenderGraph::Instance().AddPass<PassData>("PhysicsDebugPass", [this](PassData& data, RenderGraphBuilder& builder)
            {
                ShaderDesc lineShader("PhysicsDebugPass_LineShader", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
                lineShader.InputLayout =
                {
                    ShaderInputLayout(0, PixelFormat::R32G32B32A32_Float, 0, 0, "POSITION"),
                    ShaderInputLayout(1, PixelFormat::R32G32B32A32_Float, 0, 16, "COLOR"),
                    ShaderInputLayout(2, PixelFormat::R32G32_Float, 0, 32, "TEXCOORD0")
                };
                builder.SetShader(lineShader);

                PipelineStateObject linePSO = { };
                linePSO.Name = "PhysicsDebugPass_LinePSO";
                linePSO.PrimitiveTopologyType = PrimitiveTopologyType::LineList;
                linePSO.CullMode = CullMode::Front;
                linePSO.FrontFace = FrontFace::CounterClockwise;
                linePSO.DepthTest = true;
                linePSO.DepthWrite = true;
                linePSO.DepthCompareOp = CompareOp::Less;

                linePSO.BlendEnable = true;
                linePSO.SrcColourBlendFactor = BlendFactor::SrcAlpha;
                linePSO.DstColourBlendFactor = BlendFactor::OneMinusSrcAlpha;
                linePSO.ColourBlendOp = BlendOp::Add;
                linePSO.SrcAplhaBlendFactor = BlendFactor::Zero;
                linePSO.DstAplhaBlendFactor = BlendFactor::Zero;
                linePSO.AplhaBlendOp = BlendOp::Add;

                linePSO.Dynamic_States.push_back(DynamicState::LineWidth);
                linePSO.ShaderDescription = lineShader;

                builder.SetPipeline(linePSO);

                Graphics::RGTextureHandle colourRT = builder.GetTexture(data.ColourTextureName);
                builder.WriteTexture(colourRT);
                Graphics::RGTextureHandle depthRT = builder.GetTexture(data.DepthTextureName);
                builder.WriteDepthStencil(depthRT);

                RenderpassDescription renderPassDesc = {};
                renderPassDesc.AddAttachment(Graphics::AttachmentDescription::Load(builder.GetRHITexture(colourRT)->GetFormat(), Graphics::ImageLayout::ColourAttachment));
                renderPassDesc.Attachments.back().InitalLayout = Graphics::ImageLayout::ColourAttachment;

                renderPassDesc.DepthStencilAttachment.InitalLayout = Graphics::ImageLayout::DepthStencilAttachment;
                renderPassDesc.DepthStencilAttachment = AttachmentDescription::Load(builder.GetRHITexture(depthRT)->GetFormat(), Graphics::ImageLayout::DepthStencilAttachment);
                builder.SetRenderpass(renderPassDesc);

                const u32 renderResolutionX = builder.GetRenderResolution().x;
                const u32 renderResolutionY = builder.GetRenderResolution().y;
                builder.SetViewport(renderResolutionX, renderResolutionY);
                builder.SetScissor(renderResolutionX, renderResolutionY);
            },
            [&](PassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
            {
                {
                    IS_PROFILE_SCOPE("PhysicsDebug_Line Pass");
                    
                    PipelineStateObject pso = renderGraph.GetPipelineStateObject("PhysicsDebugPass");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("PhysicsDebugPass"));

                    RHI_BufferView constantBuffer = cmdList->UploadUniform(data.ConstantBuffer);
                    cmdList->SetUniform(0, 0, constantBuffer);

                    RHI_Buffer* vertexBuffer = m_vertex_buffer.Get();
                    RHI_Buffer* indexBuffer = m_index_buffer.Get();

                    const Physics::DebugRendererData& renderData = data.RenderData;
                    const u64 requiredVertexBufferSize = renderData.Vertices.size() * sizeof(Physics::DebugRendererData::Vertex);
                    const u64 requiredIndexBufferSize = renderData.Indices.size() * sizeof(u16);
                    
                    if (vertexBuffer->GetSize() < requiredVertexBufferSize)
                    {
                        vertexBuffer->Resize(requiredVertexBufferSize);
                    }
                    if (indexBuffer->GetSize() < requiredIndexBufferSize)
                    {
                        indexBuffer->Resize(requiredIndexBufferSize);
                    }

                    vertexBuffer->Upload(renderData.Vertices.data(), requiredVertexBufferSize);
                    indexBuffer->Upload(renderData.Indices.data(), requiredIndexBufferSize);

                    cmdList->SetVertexBuffer(vertexBuffer);
                    cmdList->SetIndexBuffer(indexBuffer, IndexType::Uint16);

                    // Line rendering
                    for (size_t lineDrawIdx = 0; lineDrawIdx < renderData.Lines.size(); ++lineDrawIdx)
                    {
                        const Physics::DebugRendererData::Line& line = renderData.Lines[lineDrawIdx];
                        cmdList->Draw(line.Size, 1, line.StartIndex, 0);
                    }
                    
                    ShaderDesc triangleShader("PhysicsDebugPass_TriangleShader", {}, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
                    triangleShader.InputLayout =
                    {
                        ShaderInputLayout(0, PixelFormat::R32G32B32A32_Float, 0, 0, "POSITION"),
                        ShaderInputLayout(1, PixelFormat::R32G32B32A32_Float, 0, 16, "COLOR"),
                        ShaderInputLayout(2, PixelFormat::R32G32_Float, 0, 32, "TEXCOORD0")
                    };
                    PipelineStateObject trianglePSO = pso;
                    trianglePSO.Name = "PhysicsDebugPass_TrianglePSO";
                    trianglePSO.PrimitiveTopologyType = PrimitiveTopologyType::TriangleList;
                    trianglePSO.BlendEnable = false;
                    trianglePSO.ShaderDescription = triangleShader;

                    // Triangle rendering
                    cmdList->BindPipeline(trianglePSO, nullptr);

                    cmdList->SetUniform(0, 0, constantBuffer);

                    for (size_t triangleIdx = 0; triangleIdx < renderData.Triangles.size(); ++triangleIdx)
                    {
                        const Physics::DebugRendererData::Triangle& triangle = renderData.Triangles[triangleIdx];
                        cmdList->DrawIndexed(triangle.IndexCount, 1, triangle.IndexStartIndex, triangle.VertexStartIndex, 0);
                    }

                    cmdList->EndRenderpass();
                }
            }, std::move(passData));
        }

        void PhysicsDebugRenderPass::Destroy()
        {
            m_vertex_buffer.ForEach([](RHI_Buffer*& buffer)
                {
                Renderer::FreeVertexBuffer(buffer);
                buffer = nullptr;
                });
            m_index_buffer.ForEach([](RHI_Buffer*& buffer)
                {
                    Renderer::FreeVertexBuffer(buffer);
                    buffer = nullptr;
                });
        }
    }
}