#include "Graphics/PhysicsDebugRenderPass.h"
#include "Graphics/Vertex.h"
#include "Graphics/RenderGraph/RenderGraph.h"

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
                        buffer = Renderer::CreateVertexBuffer(sizeof(Graphics::Vertex) * 1024, sizeof(Graphics::Vertex), buffer_overrides);
                    });
            }
            if (m_index_buffer.Size() == 0)
            {
                m_index_buffer.Setup();
                m_index_buffer.ForEach([buffer_overrides](RHI_Buffer*& buffer)
                    {
                        buffer = Renderer::CreateIndexBuffer(sizeof(u32) * 1024, buffer_overrides);
                    });
            }

            std::vector<Byte> shaderData = Runtime::AssetRegistry::Instance().LoadAssetData(EnginePaths::GetResourcePath() + "/Shaders/hlsl/PhysicsDebug.hlsl");
            ShaderDesc shaderDesc("PhysicsDebug", shaderData, ShaderStageFlagBits::ShaderStage_Vertex | ShaderStageFlagBits::ShaderStage_Pixel);
            shaderDesc.InputLayout = ShaderDesc::GetDefaultShaderInputLayout();
            RenderContext::Instance().GetShaderManager().GetOrCreateShader(shaderDesc);
        }

        void PhysicsDebugRenderPass::Render()
        {
            struct PassData
            {
                Physics::DebugRendererData RenderData;
            };
            PassData passData
            {
                Physics::PhysicsWorld::GetDebugRenderData()
            };

            RenderGraph::Instance().AddPass<PassData>("PhysicsDebugPass", [this](PassData& data, RenderGraphBuilder& builder)
            {
                
            },
            [&](PassData& data, RenderGraph& renderGraph, RHI_CommandList* cmdList)
            {
                IS_PROFILE_SCOPE("ImGui pass execute");
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