#include "Graphics/DX12RenderPasses.h"

#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/RenderContext.h"

#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"

#include "Resource/ResourceManager.h"
#include "Resource/Texture2D.h"
#include "Resource/Material.h"

#include "World/World.h"
#include "World/WorldSystem.h"

#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/CameraComponent.h"

#include "Renderpass.h"

#include "Core/Profiler.h"

namespace Insight
{
    namespace Runtime
    {
        Graphics::RHI_Buffer* VertexBuffer = nullptr;
        Graphics::RHI_Buffer* IndexBuffer = nullptr;
        TObjectPtr<Texture2D> Texture = nullptr;
        Graphics::RHI_Sampler* Sampler = nullptr;

        struct alignas(16) UBO
        {
            glm::vec4 Transform;
            glm::vec4 OverrideColour;
            int Override;
        };

        auto SetTextureName(IResource* texture) -> void
        {
            texture->OnLoaded.Unbind<&SetTextureName>();
            static_cast<Texture2D*>(texture)->GetRHITexture()->SetName("Christmas_Cute_Roadhog");
        };

        void DX12RenderPasses::Render()
        {
            using namespace Graphics;

            if (!VertexBuffer)
            {
                const float size = 2.5f;
                Graphics::Vertex vertices[] =
                {
                    Graphics::Vertex(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
                    Graphics::Vertex(glm::vec4(0.0f, size, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
                    Graphics::Vertex(glm::vec4(size, size, 0.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
                    Graphics::Vertex(glm::vec4(size, 0.0f, 0.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
                };
                for (size_t i = 0; i < ARRAY_COUNT(vertices); ++i)
                {
                    vertices[i].UV.y = 1.0f - vertices[i].UV.y;
                }

                VertexBuffer = Renderer::CreateVertexBuffer(sizeof(vertices), sizeof(Graphics::Vertex));
                VertexBuffer->Upload(&vertices, sizeof(vertices));
                VertexBuffer->SetName("TestVertexBuffer");
            }

            if (!IndexBuffer)
            {
                u32 indices[] =
                {
                    0, 1, 2,
                    0, 2, 3
                };
                IndexBuffer = Renderer::CreateIndexBuffer(sizeof(indices));
                IndexBuffer->Upload(&indices, sizeof(indices));
                IndexBuffer->SetName("TestIndexBuffer");
            }

            if (!Texture)
            {
                Texture = ResourceManagerExt::Load(ResourceId("./Resources/Christmas_Cute_Roadhog.png", Texture2D::GetStaticResourceTypeId()));
                Texture->OnLoaded.Bind<&SetTextureName>();
            }

            if (!Sampler)
            {
                RHI_SamplerCreateInfo desc = {};
                desc.AddressMode = SamplerAddressMode::ClampToEdge;
                desc.CompareOp = CompareOp::Never;
                desc.MaxLod = FLT_MAX;
                desc.BoarderColour = BorderColour::FloatTransparentBlack;
                Sampler = RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(desc);
            }

            std::vector<Ptr<ECS::Entity>> entitiesToRender;
            TObjectPtr<Runtime::World> world = Runtime::WorldSystem::Instance().GetActiveWorld();
            if (world)
            {
                std::vector<Ptr<ECS::Entity>> entities = world->GetAllEntitiesWithComponentByName(ECS::MeshComponent::Type_Name);
                for (Ptr<ECS::Entity> entity : entities)
                {
                    ECS::MeshComponent* meshComponent = static_cast<ECS::MeshComponent*>(entity->GetComponentByName(ECS::MeshComponent::Type_Name));
                    Runtime::Material* material = meshComponent->GetMaterial();
                    if (material)
                    {
                    }
                        entitiesToRender.push_back(entity);
                }
            }

            auto editorWorld = Runtime::WorldSystem::Instance().FindWorldByName("EditorWorld");
            auto editorCamera = editorWorld->GetEntityByName("EditorCamera");
            auto cameraComponent = static_cast<ECS::CameraComponent*>(editorCamera->GetComponentByName(ECS::CameraComponent::Type_Name));

            struct PassData
            { 
                BufferFrame BFrame;
                std::vector<Ptr<ECS::Entity>> EntitiesToRender;
            };

            BufferFrame bFrame = {};
            bFrame.Proj_View = cameraComponent->GetProjectionViewMatrix();
            bFrame.Projection = cameraComponent->GetProjectionMatrix();
            bFrame.View = cameraComponent->GetViewMatrix();

            bFrame.View_Inverted = cameraComponent->GetInvertedViewMatrix();
            bFrame.Projection_View_Inverted = cameraComponent->GetInvertedProjectionViewMatrix();

            bFrame.Render_Resolution = RenderGraph::Instance().GetRenderResolution();
            bFrame.Ouput_Resolution = RenderGraph::Instance().GetOutputResolution();

            PassData passData
            {
                std::move(bFrame),
                std::move(entitiesToRender)
            };

            RenderGraph::Instance().AddPass<PassData>("TestPass",
                [&](PassData& data, RenderGraphBuilder& builder)
                {
                    RHI_TextureInfo textureCreateInfo = RHI_TextureInfo::Tex2D(
                        builder.GetRenderResolution().x
                        , builder.GetRenderResolution().y
                        , PixelFormat::D32_Float
                        , ImageUsageFlagsBits::DepthStencilAttachment | ImageUsageFlagsBits::Sampled);
                    RGTextureHandle depthTexture = builder.CreateTexture("DepthStencil", textureCreateInfo);

                    builder.WriteTexture(-1);
                    builder.WriteDepthStencil(depthTexture);

                    ShaderDesc shaders = {};
                    shaders.VertexFilePath = "./Resources/Shaders/hlsl/TestPassDX12.hlsl";
                    shaders.PixelFilePath = "./Resources/Shaders/hlsl/TestPassDX12.hlsl";
                    builder.SetShader(shaders);

                    PipelineStateObject pso = { };
                    pso.Name = "TestPass_PSO";
                    pso.ShaderDescription = shaders;
                    pso.DepthWrite = true;
                    pso.DepthTest = true;
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
                    || IndexBuffer->GetUploadStatus() != DeviceUploadStatus::Completed
                    || Texture->GetResourceState() != Runtime::EResoruceStates::Loaded
                    || (Texture->GetRHITexture() && Texture->GetRHITexture()->GetUploadStatus() != DeviceUploadStatus::Completed))
                    {
                        return;
                    }

                    IS_PROFILE_SCOPE("DX12 Test Pass");

                    cmdList->BindPipeline(renderGraph.GetPipelineStateObject("TestPass"), nullptr);
                    cmdList->BeginRenderpass(renderGraph.GetRenderpassDescription("TestPass"));

                    cmdList->SetVertexBuffer(VertexBuffer);
                    cmdList->SetIndexBuffer(IndexBuffer, IndexType::Uint32);

                    cmdList->SetUniform(0, 0, data.BFrame);

                    cmdList->SetUniform(1, 0, glm::mat4(1.0f));
                    cmdList->DrawIndexed(6, 1, 0, 0, 0);

                    for (const Ptr<ECS::Entity>& e : data.EntitiesToRender)
                    {
                        ECS::TransformComponent* transform_component = static_cast<ECS::TransformComponent*>(e->GetComponentByName(ECS::TransformComponent::Type_Name));
                        glm::mat4 transform = transform_component->GetTransform();

                        ECS::MeshComponent* mesh_component = static_cast<ECS::MeshComponent*>(e->GetComponentByName(ECS::MeshComponent::Type_Name));
                        if (!mesh_component
                            || !mesh_component->GetMesh())
                        {
                            continue;
                        }
                        Runtime::Mesh* mesh = mesh_component->GetMesh();

                        cmdList->SetUniform(1, 0, transform);

                        mesh->Draw(cmdList);
                    }

                    cmdList->EndRenderpass();
                }, std::move(passData));
        }
    }
}