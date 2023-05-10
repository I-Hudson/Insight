#include "Editor/EditorWindows/WorldViewWindow.h"

#include "Runtime/Engine.h"

#include "Graphics/GraphicsSystem.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/Enums.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/FreeCameraControllerComponent.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        WorldViewWindow::WorldViewWindow()
            : IEditorWindow()
        { }

        WorldViewWindow::WorldViewWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }

        WorldViewWindow::WorldViewWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }

        WorldViewWindow::~WorldViewWindow()
        {
            Runtime::WorldSystem::Instance().RemoveWorld(Runtime::WorldSystem::Instance().FindWorldByName(c_WorldName));
        }

        void WorldViewWindow::Initialise()
        {
            m_imguiWindowFlags = 0
                | ImGuiWindowFlags_NoScrollbar
                | ImGuiWindowFlags_NoCollapse;

            const float aspect = (float)Graphics::Window::Instance().GetWidth() / (float)Graphics::Window::Instance().GetHeight();
            TObjectPtr<Runtime::World> editorWorld = Runtime::WorldSystem::Instance().CreatePersistentWorld(c_WorldName, Runtime::WorldTypes::Tools);
            editorWorld->SetOnlySearchable(true);
            m_editorCameraEntity = editorWorld->AddEntity("EditorCamera").Get();

            m_editorCameraEntity->AddComponentByName(ECS::TransformComponent::Type_Name);
            m_editorCameraComponent = static_cast<ECS::CameraComponent*>(m_editorCameraEntity->AddComponentByName(ECS::CameraComponent::Type_Name));
            m_editorCameraComponent->CreatePerspective(glm::radians(90.0f), aspect, 0.1f, 1024.0f);
            m_editorCameraEntity->AddComponentByName(ECS::FreeCameraControllerComponent::Type_Name);
        }

        void WorldViewWindow::OnDraw()
        {
            SetupRenderGraphPasses();

            Graphics::RHI_Texture* worldViewTexture = Graphics::RenderGraph::Instance().GetRenderCompletedRHITexture("EditorWorldColourRT");
            if (worldViewTexture == nullptr)
            {
                return;
            }

            ImVec2 windowSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            ImGui::Image(worldViewTexture, windowSize);

            TObjectPtr<Runtime::World> world = Runtime::WorldSystem::Instance().FindWorldByName(c_WorldName);
            if (world)
            {
                ImGui::IsWindowFocused() ? 
                    world->SetWorldState(Runtime::WorldStates::Running) : world->SetWorldState(Runtime::WorldStates::Paused);
            }
        }

        void WorldViewWindow::SetupRenderGraphPasses()
        {
            const ECS::Camera camera = m_editorCameraComponent->GetCamera();
            const glm::mat4 cameraTransform = m_editorCameraComponent->GetViewMatrix();

            RenderFrame renderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();
            renderFrame.SetCameraForAllWorlds(camera, cameraTransform);
            renderFrame.Sort();

            RenderData renderData =
            {
                renderFrame,
                GetBufferFrame(),
                GetBufferSamplers()
            };

            GBufferPass(renderData);
            TransparentGBufferPass(renderData);
        }

        void WorldViewWindow::GBufferPass(const RenderData& renderData)
        {
            struct WorldGBufferData
            {
                RenderData RenderData;
            };

            WorldGBufferData passData =
            {
                renderData
            };

            Graphics::RenderGraph::Instance().AddPass<WorldGBufferData>("EditorWorldGBuffer",
                [](WorldGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("GBuffer pass setup");

                    Graphics::RHI_TextureInfo textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          builder.GetRenderResolution().x
                        , builder.GetRenderResolution().y
                        , PixelFormat::R8G8B8A8_UNorm
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle colourRT = builder.CreateTexture("EditorWorldColourRT", textureCreateInfo);
                    builder.WriteTexture(colourRT);

                    textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          builder.GetRenderResolution().x
                        , builder.GetRenderResolution().y
                        , PixelFormat::R16G16B16A16_Float
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle normal_rt = builder.CreateTexture("EditorWorldNormalRT", textureCreateInfo);
                    builder.WriteTexture(normal_rt);

                    textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          builder.GetRenderResolution().x
                        , builder.GetRenderResolution().y
                        , PixelFormat::R16G16_Float
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle velocity_rt = builder.CreateTexture("EditorWorldVelocityRT", textureCreateInfo);
                    builder.WriteTexture(velocity_rt);

                    textureCreateInfo.Format = PixelFormat::D32_Float;
                    textureCreateInfo.ImageUsage = Graphics::ImageUsageFlagsBits::DepthStencilAttachment | Graphics::ImageUsageFlagsBits::Sampled;

                    Graphics::RGTextureHandle depthStencil = builder.CreateTexture("EditorWorldDepthStencilRT", textureCreateInfo);
                    builder.WriteDepthStencil(depthStencil);

                    Graphics::ShaderDesc shaderDesc;
                    {
                        shaderDesc.VertexFilePath = "Resources/Shaders/hlsl/GBuffer.hlsl";
                        shaderDesc.PixelFilePath = "Resources/Shaders/hlsl/GBuffer.hlsl";
                        shaderDesc.InputLayout = Graphics::GetDefaultShaderInputLayout();
                    }
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject gbufferPso = { };
                    {
                        gbufferPso.Name = "EditorWorldGBuffer_PSO";
                        gbufferPso.CullMode = Graphics::CullMode::None;
                        gbufferPso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        gbufferPso.ShaderDescription = shaderDesc;
                        gbufferPso.DepthCompareOp = Graphics::CompareOp::LessOrEqual;

                        if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                        {
                            gbufferPso.DepthCompareOp = Graphics::CompareOp::GreaterOrEqual;
                        }
                    }
                    builder.SetPipeline(gbufferPso);

                    builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                    builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                },
                [this](WorldGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldGBuffer pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldGBuffer");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldGBuffer"));

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, data.RenderData);
                    }

                    for (const RenderWorld& world : data.RenderData.RenderFrame.RenderWorlds)
                    {

                        for (const u64 meshIndex : world.OpaqueMeshIndexs)
                        {
                            IS_PROFILE_SCOPE("Draw Entity");
                            const RenderMesh& mesh = world.Meshes.at(meshIndex);

                            Graphics::BufferPerObject object = {};
                            object.Transform = mesh.Transform;
                            object.Previous_Transform = mesh.Transform;

                            {
                                IS_PROFILE_SCOPE("Set textures");

                                const RenderMaterial& renderMaterial = mesh.Material;
                                // Theses sets and bindings shouldn't chagne.
                                Graphics::RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureTypes::Diffuse];
                                if (diffuseTexture)
                                {
                                    cmdList->SetTexture(3, 0, diffuseTexture);
                                    object.Textures_Set[0] = 1;
                                }
                            }

                            cmdList->SetUniform(2, 0, object);

                            const Runtime::MeshLOD& renderMeshLod = mesh.MeshLods.at(0);
                            cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                            cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                            cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                            ++Graphics::RenderStats::Instance().MeshCount;
                        }
                    }
                    cmdList->EndRenderpass();
                }, std::move(passData));
        }

        void WorldViewWindow::TransparentGBufferPass(const RenderData& renderData)
        {
            struct WorldTransparentGBufferData
            {
                RenderData RenderData;
            };

            WorldTransparentGBufferData passData;
            passData.RenderData = renderData;


            Graphics::RenderGraph::Instance().AddPass<WorldTransparentGBufferData>("EditorWorldTransparentGBuffer",
                [](WorldTransparentGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("TransparentGBuffer pass setup");

                    Graphics::RGTextureHandle colourRT = builder.GetTexture("EditorWorldColourRT");
                    builder.WriteTexture(colourRT);
                    Graphics::RGTextureHandle normal_rt = builder.GetTexture("EditorWorldNormalRT");
                    builder.WriteTexture(normal_rt);
                    Graphics::RGTextureHandle velocity_rt = builder.GetTexture("EditorWorldVelocityRT");
                    builder.WriteTexture(velocity_rt);
                    Graphics::RGTextureHandle depth = builder.GetTexture("EditorWorldDepthStencilRT");
                    builder.WriteDepthStencil(depth);
                    
                    Graphics::RenderpassDescription renderpassDescription = {};
                    renderpassDescription.AddAttachment(Graphics::AttachmentDescription::Load(builder.GetRHITexture(colourRT)->GetFormat(), Graphics::ImageLayout::ColourAttachment));
                    renderpassDescription.AddAttachment(Graphics::AttachmentDescription::Load(builder.GetRHITexture(normal_rt)->GetFormat(), Graphics::ImageLayout::ColourAttachment));
                    renderpassDescription.AddAttachment(Graphics::AttachmentDescription::Load(builder.GetRHITexture(velocity_rt)->GetFormat(), Graphics::ImageLayout::ColourAttachment));
                    for (Graphics::AttachmentDescription& attachment : renderpassDescription.Attachments)
                    {
                        attachment.InitalLayout = Graphics::ImageLayout::ColourAttachment;
                    }

                    renderpassDescription.DepthStencilAttachment = Graphics::AttachmentDescription::Load(builder.GetRHITexture(depth)->GetFormat(), Graphics::ImageLayout::DepthStencilAttachment);
                    renderpassDescription.DepthStencilAttachment.InitalLayout = Graphics::ImageLayout::DepthStencilAttachment;
                    builder.SetRenderpass(renderpassDescription);

                    Graphics::ShaderDesc shaderDesc;
                    {
                        shaderDesc.VertexFilePath = "Resources/Shaders/hlsl/GBuffer.hlsl";
                        shaderDesc.PixelFilePath = "Resources/Shaders/hlsl/GBuffer.hlsl";
                        shaderDesc.InputLayout = Graphics::GetDefaultShaderInputLayout();
                    }
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject pso = { };
                    {
                        IS_PROFILE_SCOPE("SetPipelineStateObject");
                        pso.ShaderDescription = shaderDesc;
                        pso.Name = "Transparent_GBuffer";
                        pso.CullMode = Graphics::CullMode::None;
                        pso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        pso.BlendEnable = true;
                        pso.SrcColourBlendFactor = Graphics::BlendFactor::SrcAlpha;
                        pso.DstColourBlendFactor = Graphics::BlendFactor::OneMinusSrcAlpha;
                        pso.ColourBlendOp = Graphics::BlendOp::Add;
                        pso.SrcAplhaBlendFactor = Graphics::BlendFactor::One;
                        pso.DstAplhaBlendFactor = Graphics::BlendFactor::One;
                        pso.AplhaBlendOp = Graphics::BlendOp::Add;

                        if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                        {
                            pso.DepthCompareOp = Graphics::CompareOp::GreaterOrEqual;
                        }
                    }
                    builder.SetPipeline(pso);

                    builder.SetViewport(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                    builder.SetScissor(builder.GetRenderResolution().x, builder.GetRenderResolution().y);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldGBuffer pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldTransparentGBuffer");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldTransparentGBuffer"));

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, data.RenderData);
                    }

                    for (const RenderWorld& world : data.RenderData.RenderFrame.RenderWorlds)
                    {
                        for (const u64 meshIndex : world.TransparentMeshIndexs)
                        {
                            IS_PROFILE_SCOPE("Draw Entity");
                            const RenderMesh& mesh = world.Meshes.at(meshIndex);

                            Graphics::BufferPerObject object = {};
                            object.Transform = mesh.Transform;
                            object.Previous_Transform = mesh.Transform;

                            {
                                IS_PROFILE_SCOPE("Set textures");

                                const RenderMaterial& renderMaterial = mesh.Material;
                                // Theses sets and bindings shouldn't change.
                                Graphics::RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureTypes::Diffuse];
                                if (diffuseTexture)
                                {
                                    cmdList->SetTexture(3, 0, diffuseTexture);
                                    object.Textures_Set[0] = 1;
                                }
                            }

                            cmdList->SetUniform(2, 0, object);

                            const Runtime::MeshLOD& renderMeshLod = mesh.MeshLods.at(0);
                            cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                            cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                            cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                            ++Graphics::RenderStats::Instance().MeshCount;
                        }

                    }
                    cmdList->EndRenderpass();
                }, std::move(passData));
        }

        void WorldViewWindow::BindCommonResources(Graphics::RHI_CommandList* cmd_list, RenderData& renderData)
        {
            if (!renderData.FrameView.IsValid())
            {
                renderData.FrameView = cmd_list->UploadUniform(renderData.BufferFrame);
            }
            cmd_list->SetUniform(0, 0, renderData.FrameView);

            cmd_list->SetSampler(4, 0, renderData.BufferSamplers.Shadow_Sampler);
            cmd_list->SetSampler(4, 1, renderData.BufferSamplers.Repeat_Sampler);
            cmd_list->SetSampler(4, 2, renderData.BufferSamplers.Clamp_Sampler);
            cmd_list->SetSampler(4, 3, renderData.BufferSamplers.MirroredRepeat_Sampler);
        }

        Graphics::BufferFrame WorldViewWindow::GetBufferFrame() const
        {
            Graphics::BufferFrame bufferFrame;
            bufferFrame.Proj_View = m_editorCameraComponent->GetProjectionViewMatrix();
            bufferFrame.Projection = m_editorCameraComponent->GetProjectionMatrix();
            bufferFrame.View = m_editorCameraComponent->GetViewMatrix();

            bufferFrame.View_Inverted = m_editorCameraComponent->GetInvertedViewMatrix();
            bufferFrame.Projection_View_Inverted = m_editorCameraComponent->GetInvertedProjectionViewMatrix();

            bufferFrame.Render_Resolution = Graphics::RenderGraph::Instance().GetRenderResolution();
            bufferFrame.Ouput_Resolution = Graphics::RenderGraph::Instance().GetOutputResolution();
            return bufferFrame;
        }

        Graphics::BufferSamplers WorldViewWindow::GetBufferSamplers() const
        {
            Graphics::BufferSamplers bufferSamplers;
            Graphics::RHI_SamplerCreateInfo sampler_create_info = { };
            sampler_create_info.MagFilter = Graphics::Filter::Linear;
            sampler_create_info.MinFilter = Graphics::Filter::Linear;
            sampler_create_info.MipmapMode = Graphics::SamplerMipmapMode::Nearest;
            sampler_create_info.AddressMode = Graphics::SamplerAddressMode::ClampToEdge;
            sampler_create_info.CompareEnabled = true;
            if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
            {
                sampler_create_info.CompareOp = Graphics::CompareOp::Greater;
            }
            else
            {
                sampler_create_info.CompareOp = Graphics::CompareOp::Less;
            }
            bufferSamplers.Shadow_Sampler = Graphics::RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(sampler_create_info);
            sampler_create_info.AddressMode = Graphics::SamplerAddressMode::Repeat;
            bufferSamplers.Repeat_Sampler = Graphics::RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(sampler_create_info);
            sampler_create_info.AddressMode = Graphics::SamplerAddressMode::ClampToEdge;
            bufferSamplers.Clamp_Sampler = Graphics::RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(sampler_create_info);
            sampler_create_info.AddressMode = Graphics::SamplerAddressMode::MirroredRepeat;
            bufferSamplers.MirroredRepeat_Sampler = Graphics::RenderContext::Instance().GetSamplerManager().GetOrCreateSampler(sampler_create_info);
            return bufferSamplers;
        }
    }
}