#include "Editor/EditorWindows/WorldViewWindow.h"
#include "Editor/EditorWindows/ContentWindow.h"
#include "Editor/EditorWindows/EditorWindowManager.h"
#include "Editor/EditorWindows/WorldEntitiesWindow.h"
#include "Editor/EditorGUI.h"

#include "Runtime/Engine.h"
//#include "Resource/Model.h"
#include "Asset/Assets/Model.h"

#include "Graphics/GraphicsSystem.h"
#include "Graphics/RenderGraph/RenderGraph.h"
#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/FreeCameraControllerComponent.h"

#include "Core/Logger.h"

#include <imgui.h>
#include <ImGuizmo.h>

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
            IS_PROFILE_FUNCTION();

            ImGuizmo::BeginFrame();
            ImGuizmo::SetDrawlist();

            SetupRenderGraphPasses();

            const char* editorOutputItems[] =
            {
                "EditorWorldLightRT",
                "EditorWorldColourRT",
            };
            static int editorOutput = 0;
            ImGui::Combo("Editor Output", &editorOutput, editorOutputItems, ARRAY_COUNT(editorOutputItems));

            Graphics::RHI_Texture* worldViewTexture = Graphics::RenderGraph::Instance().GetRenderCompletedRHITexture(editorOutputItems[editorOutput]);
            if (worldViewTexture == nullptr)
            {
                return;
            }

            ImVec2 windowSize = ImVec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            ImGui::Image(worldViewTexture, windowSize);

            ContentWindowDragTarget();

            TObjectPtr<Runtime::World> world = Runtime::WorldSystem::Instance().FindWorldByName(c_WorldName);
            if (world)
            {
                ImGui::IsWindowFocused() ?
                    world->SetWorldState(Runtime::WorldStates::Running) : world->SetWorldState(Runtime::WorldStates::Paused);
            }

            IEditorWindow* worldEntitiesWindowBase = EditorWindowManager::Instance().GetActiveWindow(WorldEntitiesWindow::WINDOW_NAME);

            if (worldEntitiesWindowBase != nullptr)
            {
                WorldEntitiesWindow* worldEntitiesWindow = static_cast<WorldEntitiesWindow*>(worldEntitiesWindowBase);
                std::unordered_set<Core::GUID> selectedEntites = worldEntitiesWindow->GetSelectedEntities();
                if (!selectedEntites.empty())
                {
                    Maths::Matrix4 viewMatrix = m_editorCameraComponent->GetCamera().GetInvertedViewMatrix();
                    Maths::Matrix4 projectionMatrix = m_editorCameraComponent->GetProjectionMatrix();

                    ECS::Entity* entity = Runtime::WorldSystem::Instance().GetEntityByGUID(*selectedEntites.begin());
                    ECS::TransformComponent* transformComponent = entity->GetComponent<ECS::TransformComponent>();
                    glm::mat4 entityMatrix = transformComponent->GetTransform();

                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
                    ImGuizmo::Manipulate(&viewMatrix[0][0], &projectionMatrix[0][0], ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, &entityMatrix[0][0]);
                    if (ImGuizmo::IsUsing())
                    {
                        transformComponent->SetTransform(entityMatrix);
                    }
                }
            }
        }

        void WorldViewWindow::ContentWindowDragTarget()
        {
            IS_PROFILE_FUNCTION();

            std::string resourceGuidString;
            if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowResourceDragSource, resourceGuidString, Runtime::Asset::GetStaticTypeInfo().GetType()))
            {
                Core::GUID resourceGuid;
                resourceGuid.StringToGuid(resourceGuidString);

                //Runtime::IResource* resource = Runtime::ResourceManager::Instance().GetResourceFromGuid(resourceGuid);
                //if (resource)
                //{
                //    if (resource->GetTypeInfo().GetType() == Runtime::Model::GetStaticTypeInfo().GetType())
                //    {
                //        Runtime::Model* model = static_cast<Runtime::Model*>(resource);
                //        model->CreateEntityHierarchy();
                //    }
                //}

                Ref<Runtime::Asset> asset = Runtime::AssetRegistry::Instance().LoadAsset(resourceGuid);
                if (Ref<Runtime::ModelAsset> modelAsset = asset.As<Runtime::ModelAsset>())
                {
                    modelAsset->CreateEntityHierarchy();
                }
            }
        }

        void WorldViewWindow::SetupRenderGraphPasses()
        {
            IS_PROFILE_FUNCTION();

            const ECS::Camera camera = m_editorCameraComponent->GetCamera();
            const Maths::Matrix4 cameraTransform = m_editorCameraComponent->GetViewMatrix();

            RenderFrame renderFrame;
            {
                IS_PROFILE_SCOPE("Copy render frame");
                renderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();
            }
            renderFrame.SetCameraForAllWorlds(camera, cameraTransform);
            renderFrame.Sort();

            RenderData renderData =
            {
                std::move(renderFrame),
                GetBufferFrame(),
                GetBufferSamplers()
            };

            Graphics::RenderGraph::Instance().AddSyncPoint([this, renderData]()
                {
                    m_renderingData = renderData;
                });

            Graphics::RenderGraph::Instance().AddPreRender([this](Graphics::RenderGraph& renderGraph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("Upload WorldViewWindow Buffer Frame");
                    m_renderingData.FrameView = cmdList->UploadUniform(m_renderingData.BufferFrame);
                });

            LightShadowPass();
            if (m_enableDepthPrepass)
            {
                GBufferDepthPrepass();
            }
            GBufferPass();
            TransparentGBufferPass();
            LightPass();
        }

        void WorldViewWindow::LightShadowPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldTransparentGBufferData
            {
                glm::ivec2 RenderResolution;
                RenderFrame RenderFrame;
            };

            WorldTransparentGBufferData passData;
            passData.RenderResolution = glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            passData.RenderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();

            Graphics::RenderGraph::Instance().AddPass<WorldTransparentGBufferData>("EditorWorldLightShadowPass",
                [](WorldTransparentGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("LightShadowPass pass setup");

                    const u32 renderResolutionX = builder.GetRenderResolution().x;
                    const u32 renderResolutionY = builder.GetRenderResolution().y;

                    Graphics::ShaderDesc shaderDesc("LightShadowPass", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex);
                    shaderDesc.InputLayout = Graphics::ShaderDesc::GetDefaultShaderInputLayout();
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject pso = { };
                    {
                        pso.Name = "EditorWorldLightShadowPass";
                        pso.CullMode = Graphics::CullMode::Front;
                        pso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        pso.DepthTest = true;
                        pso.DepthWrite = true;
                        pso.DepthClampEnabled = false;
                        pso.DepthBaisEnabled = true;
                        pso.DepthConstantBaisValue = Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ) ? -4.0f : 4.0f;
                        pso.DepthSlopeBaisValue = Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ) ? -1.5f : 1.5f;
                        pso.ShaderDescription = shaderDesc;
                        pso.DepthStencilFormat = PixelFormat::D32_Float;
                        if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                        {
                            pso.DepthCompareOp = Graphics::CompareOp::GreaterOrEqual;
                        }
                        else
                        {
                            pso.DepthCompareOp = Graphics::CompareOp::LessOrEqual;
                        }
                    }
                    builder.SetPipeline(pso);

                    Graphics::RenderpassDescription renderpassDescription = { };
                    renderpassDescription.AddAttachment(Graphics::AttachmentDescription::Default(PixelFormat::D32_Float, Graphics::ImageLayout::ShaderReadOnly));
                    renderpassDescription.Attachments.back().InitalLayout = Graphics::ImageLayout::DepthStencilAttachment;
                    builder.SetRenderpass(renderpassDescription);

                    builder.SetViewport(1024, 1024);
                    builder.SetScissor(1024, 1024);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldLightShadowPass pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldLightShadowPass");
                    cmdList->BindPipeline(pso, nullptr);

                    for (size_t worldIdx = 0; worldIdx < data.RenderFrame.RenderWorlds.size(); ++worldIdx)
                    {
                        const RenderWorld& renderWorld = data.RenderFrame.RenderWorlds[worldIdx];
                        for (size_t pointLightIdx = 0; pointLightIdx < renderWorld.PointLights.size(); ++pointLightIdx)
                        {
                            const RenderPointLight& pointLight = renderWorld.PointLights[pointLightIdx];
                            for (size_t arrayIdx = 0; arrayIdx < 6; ++arrayIdx)
                            {
                                Graphics::RenderpassDescription renderpassDescription = render_graph.GetRenderpassDescription("EditorWorldLightShadowPass");
                                renderpassDescription.DepthStencil = pointLight.DepthTexture;
                                renderpassDescription.DepthStencilAttachment.Layer_Array_Index = static_cast<u32>(arrayIdx);
                                cmdList->BeginRenderpass(renderpassDescription);

                                struct alignas(16) LightBuffer
                                {
                                    Maths::Matrix4 ProjectionView;
                                };
                                LightBuffer lightBuffer =
                                {
                                    pointLight.Projection * pointLight.View[arrayIdx]
                                };
                                cmdList->SetUniform(0, 0, lightBuffer);

                                for (const u64 meshIndex : renderWorld.OpaqueMeshIndexs)
                                {
                                    const RenderMesh& mesh = renderWorld.Meshes.at(meshIndex);
                                    Graphics::Frustum pointLightFrustum(pointLight.View[arrayIdx], pointLight.Projection, pointLight.FarPlane);

                                    const bool isVisable = pointLightFrustum.IsVisible(Maths::Vector3(mesh.Transform[3].xyz), mesh.BoudingBox.GetRadius());
                                    if (!isVisable)
                                    {
                                        //continue;
                                    }

                                    struct alignas(16) Object
                                    {
                                        glm::mat4 Transform;
                                    };
                                    Object object =
                                    {
                                        mesh.Transform,
                                    };
                                    cmdList->SetUniform(1, 0, object);

                                    const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(0);
                                    cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                                    cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                                    cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                                    ++Graphics::RenderStats::Instance().MeshCount;
                                }

                                cmdList->EndRenderpass();
                            }
                        }
                    }
                }, std::move(passData));
        }

        void WorldViewWindow::GBufferDepthPrepass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldDepthPrepassData
            {
                glm::ivec2 RenderResolution;
            };

            WorldDepthPrepassData passData =
            {
                glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y)
            };

            Graphics::RenderGraph::Instance().AddPass<WorldDepthPrepassData>("EditorWorldDepthPrepass",
            [this](WorldDepthPrepassData& data, Graphics::RenderGraphBuilder& builder)
            {
                IS_PROFILE_SCOPE("Depth Prepass setup");

                const u32 renderResolutionX = builder.GetRenderResolution().x;
                const u32 renderResolutionY = builder.GetRenderResolution().y;

                Graphics::RHI_TextureInfo textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                      renderResolutionX
                    , renderResolutionY
                    , PixelFormat::D32_Float
                    , Graphics::ImageUsageFlagsBits::DepthStencilAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                Graphics::RGTextureHandle depthStencil = builder.CreateTexture("EditorWorldDepthStencilRT_Prepass", textureCreateInfo);
                builder.WriteDepthStencil(depthStencil);

                Graphics::ShaderDesc shaderDesc("GBuffer", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex | Graphics::ShaderStageFlagBits::ShaderStage_Pixel);
                shaderDesc.InputLayout = Graphics::ShaderDesc::GetDefaultShaderInputLayout();
                builder.SetShader(shaderDesc);

                Graphics::PipelineStateObject gbufferPso = { };
                {
                    gbufferPso.Name = "EditorWorldPrepass_PSO";
                    gbufferPso.CullMode = Graphics::CullMode::Front;
                    gbufferPso.FrontFace = Graphics::FrontFace::CounterClockwise;
                    gbufferPso.ShaderDescription = shaderDesc;
                    gbufferPso.DepthCompareOp = Graphics::CompareOp::LessOrEqual;

                    if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                    {
                        gbufferPso.DepthCompareOp = Graphics::CompareOp::GreaterOrEqual;
                    }
                }
                builder.SetPipeline(gbufferPso);

                builder.SetViewport(renderResolutionX, renderResolutionY);
                builder.SetScissor(renderResolutionX, renderResolutionY);
            },
            [this](WorldDepthPrepassData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
            {
                IS_PROFILE_SCOPE("EditorWorldDepthPrepass pass execute");

                Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldDepthPrepass");
                cmdList->BindPipeline(pso, nullptr);
                cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldDepthPrepass"));

                {
                    IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                    BindCommonResources(cmdList, m_renderingData);
                }

                for (const RenderWorld& world : m_renderingData.RenderFrame.RenderWorlds)
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
                            Graphics::RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
                            if (diffuseTexture)
                            {
                                cmdList->SetTexture(3, 0, diffuseTexture);
                                object.Textures_Set[0] = 1;
                            }
                        }

                        cmdList->SetUniform(2, 0, object);

                        const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(0);
                        cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                        cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                        cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                        ++Graphics::RenderStats::Instance().MeshCount;
                    }
                }
                cmdList->EndRenderpass();
            }, std::move(passData));
        }

        void WorldViewWindow::GBufferPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldGBufferData
            {
                glm::ivec2 RenderResolution;
                bool DepthPrepassEnabled;
            };

            WorldGBufferData passData =
            {
                glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y),
                m_enableDepthPrepass
            };

            Graphics::RenderGraph::Instance().AddPass<WorldGBufferData>("EditorWorldGBuffer",
                [this](WorldGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("GBuffer pass setup");

                    const u32 renderResolutionX = builder.GetRenderResolution().x;
                    const u32 renderResolutionY = builder.GetRenderResolution().y;

                    Graphics::RHI_TextureInfo textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          renderResolutionX
                        , renderResolutionY
                        , PixelFormat::R8G8B8A8_UNorm
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle colourRT = builder.CreateTexture("EditorWorldColourRT", textureCreateInfo);
                    builder.WriteTexture(colourRT);

                    textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          renderResolutionX
                        , renderResolutionY
                        , PixelFormat::R16G16B16A16_Float
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle normal_rt = builder.CreateTexture("EditorWorldNormalRT", textureCreateInfo);
                    builder.WriteTexture(normal_rt);

                    textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          renderResolutionX
                        , renderResolutionY
                        , PixelFormat::R16G16_Float
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle velocity_rt = builder.CreateTexture("EditorWorldVelocityRT", textureCreateInfo);
                    builder.WriteTexture(velocity_rt);

                    textureCreateInfo.Format = PixelFormat::D32_Float;
                    textureCreateInfo.ImageUsage = Graphics::ImageUsageFlagsBits::DepthStencilAttachment | Graphics::ImageUsageFlagsBits::Sampled;

                    Graphics::RGTextureHandle depthStencil = builder.CreateTexture("EditorWorldDepthStencilRT", textureCreateInfo);
                    builder.WriteDepthStencil(depthStencil);

                    Graphics::ShaderDesc shaderDesc("GBuffer", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex | Graphics::ShaderStageFlagBits::ShaderStage_Pixel);
                    shaderDesc.InputLayout = Graphics::ShaderDesc::GetDefaultShaderInputLayout();
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject gbufferPso = { };
                    {
                        gbufferPso.Name = "EditorWorldGBuffer_PSO";
                        gbufferPso.CullMode = Graphics::CullMode::Front;
                        gbufferPso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        gbufferPso.ShaderDescription = shaderDesc;
                        gbufferPso.DepthCompareOp = Graphics::CompareOp::LessOrEqual;

                        if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
                        {
                            gbufferPso.DepthCompareOp = Graphics::CompareOp::GreaterOrEqual;
                        }

                        if (data.DepthPrepassEnabled)
                        {
                            gbufferPso.DepthWrite = false;
                            gbufferPso.DepthCompareOp = Graphics::CompareOp::Equal;
                        }
                    }
                    builder.SetPipeline(gbufferPso);

                    builder.SetViewport(renderResolutionX, renderResolutionY);
                    builder.SetScissor(renderResolutionX, renderResolutionY);
                },
                [this](WorldGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldGBuffer pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldGBuffer");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldGBuffer"));

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, m_renderingData);
                    }

                    for (const RenderWorld& world : m_renderingData.RenderFrame.RenderWorlds)
                    {
                        const RenderCamera& mainCamera = world.MainCamera;
                        for (const u64 meshIndex : world.OpaqueMeshIndexs)
                        {
                            IS_PROFILE_SCOPE("Draw Entity");
                            const RenderMesh& mesh = world.Meshes.at(meshIndex);

                            const Graphics::Frustum mainCameraFrustm(
                                mainCamera.Camera.GetViewMatrix(),
                                mainCamera.Camera.GetProjectionMatrix(),
                                mainCamera.Camera.GetFarPlane());
                            const bool isVisable = mainCameraFrustm.IsVisible(Maths::Vector3(mesh.Transform[3].xyz), mesh.BoudingBox.GetRadius());
                            if (!isVisable)
                            {
                                continue;
                            }

                            Graphics::BufferPerObject object = {};
                            object.Transform = mesh.Transform;
                            object.Previous_Transform = mesh.Transform;

                            {
                                IS_PROFILE_SCOPE("Set textures");

                                const RenderMaterial& renderMaterial = mesh.Material;
                                // Theses sets and bindings shouldn't chagne.
                                Graphics::RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
                                if (diffuseTexture)
                                {
                                    cmdList->SetTexture(3, 0, diffuseTexture);
                                    object.Textures_Set[0] = 1;
                                }
                            }

                            cmdList->SetUniform(2, 0, object);

                            const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(0);
                            cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                            cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                            cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                            ++Graphics::RenderStats::Instance().MeshCount;
                        }
                    }
                    cmdList->EndRenderpass();
                }, std::move(passData));
        }

        void WorldViewWindow::TransparentGBufferPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldTransparentGBufferData
            {
                glm::ivec2 RenderResolution;
            };

            WorldTransparentGBufferData passData;
            passData.RenderResolution = glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

            Graphics::RenderGraph::Instance().AddPass<WorldTransparentGBufferData>("EditorWorldTransparentGBuffer",
                [](WorldTransparentGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("TransparentGBuffer pass setup");

                    const u32 renderResolutionX = builder.GetRenderResolution().x;
                    const u32 renderResolutionY = builder.GetRenderResolution().y;

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

                    Graphics::ShaderDesc shaderDesc("GBuffer", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex | Graphics::ShaderStageFlagBits::ShaderStage_Pixel);
                    shaderDesc.InputLayout = Graphics::ShaderDesc::GetDefaultShaderInputLayout();
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject pso = { };
                    {
                        IS_PROFILE_SCOPE("SetPipelineStateObject");
                        pso.ShaderDescription = shaderDesc;
                        pso.Name = "EditorTransparent_GBuffer";
                        pso.CullMode = Graphics::CullMode::Front;
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

                    builder.SetViewport(renderResolutionX, renderResolutionY);
                    builder.SetScissor(renderResolutionX, renderResolutionY);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldGBuffer pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldTransparentGBuffer");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldTransparentGBuffer"));

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, m_renderingData);
                    }

                    for (const RenderWorld& world : m_renderingData.RenderFrame.RenderWorlds)
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
                                Graphics::RHI_Texture* diffuseTexture = renderMaterial.Textures[(u64)Runtime::TextureAssetTypes::Diffuse];
                                if (diffuseTexture)
                                {
                                    cmdList->SetTexture(3, 0, diffuseTexture);
                                    object.Textures_Set[0] = 1;
                                }
                            }

                            cmdList->SetUniform(2, 0, object);

                            const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(0);
                            cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                            cmdList->SetIndexBuffer(renderMeshLod.Index_buffer, Graphics::IndexType::Uint32);
                            cmdList->DrawIndexed(renderMeshLod.Index_count, 1, renderMeshLod.First_index, renderMeshLod.Vertex_offset, 0);
                            ++Graphics::RenderStats::Instance().MeshCount;
                        }

                    }
                    cmdList->EndRenderpass();
                }, std::move(passData));
        }

        void WorldViewWindow::LightPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldTransparentGBufferData
            {
                glm::ivec2 RenderResolution;
                RenderFrame RenderFrame;
            };

            WorldTransparentGBufferData passData;
            passData.RenderResolution = glm::ivec2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
            passData.RenderFrame = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>()->GetRenderFrame();

            Graphics::RenderGraph::Instance().AddPass<WorldTransparentGBufferData>("EditorWorldLightPass",
                [](WorldTransparentGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("EditorWorldLightPass pass setup");

                    const u32 renderResolutionX = builder.GetRenderResolution().x;
                    const u32 renderResolutionY = builder.GetRenderResolution().y;

                    Graphics::RHI_TextureInfo textureCreateInfo = Graphics::RHI_TextureInfo::Tex2D(
                          renderResolutionX
                        , renderResolutionY
                        , PixelFormat::R32G32B32A32_Float
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled);
                    Graphics::RGTextureHandle lightRT = builder.CreateTexture("EditorWorldLightRT", textureCreateInfo);
                    builder.WriteTexture(lightRT);

                    Graphics::RGTextureHandle colourRT = builder.GetTexture("EditorWorldColourRT");
                    builder.ReadTexture(colourRT);
                    Graphics::RGTextureHandle depth = builder.GetTexture("EditorWorldDepthStencilRT");
                    builder.ReadTexture(depth);

                    Graphics::ShaderDesc shaderDesc("LightPass", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex | Graphics::ShaderStageFlagBits::ShaderStage_Pixel);
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject pso = { };
                    {
                        IS_PROFILE_SCOPE("SetPipelineStateObject");
                        pso.Name = "EditorLightPass";
                        pso.CullMode = Graphics::CullMode::Front;
                        pso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        pso.DepthTest = false;
                        pso.BlendEnable = false;
                        pso.ShaderDescription = shaderDesc;
                    }
                    builder.SetPipeline(pso);

                    builder.SetViewport(renderResolutionX, renderResolutionY);
                    builder.SetScissor(renderResolutionX, renderResolutionY);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorLightPass pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldLightPass");
                    cmdList->BindPipeline(pso, nullptr);
                    cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldLightPass"));

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, m_renderingData);
                    }

                    cmdList->SetTexture(6, 0, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldColourRT")));
                    cmdList->SetTexture(6, 1, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldDepthStencilRT")));
                    //cmdList->SetTexture(1, 7, render_graph.GetRHITexture(""));

                    for (const RenderWorld& world : data.RenderFrame.RenderWorlds)
                    {
                        const u32 c_MaxPointLights = 32;

                        struct PointLightBuffer
                        {
                            RenderPointLight PointLights[c_MaxPointLights];
                            int PointLightSize;
                            float CameraFarPlane;
                        };

                        PointLightBuffer pointLightBuffer;
                        for (size_t i = 0; i < world.PointLights.size(); ++i)
                        {
                            if (i >= 32)
                            {
                                FAIL_ASSERT_MSG("Only 32 point lights are supported.");
                                break;
                            }

                            pointLightBuffer.PointLights[i] = world.PointLights[i];
                            cmdList->SetTexture(7, 0 + i, world.PointLights[i].DepthTexture);
                        }
                        pointLightBuffer.PointLightSize = world.PointLights.size();
                        pointLightBuffer.CameraFarPlane = world.MainCamera.Camera.GetFarPlane();

                        Graphics::RHI_BufferView spotLightRHIBuffer = cmdList->UploadUniform(pointLightBuffer);
                        cmdList->SetUniform(6, 0, spotLightRHIBuffer);

                        cmdList->Draw(3, 1, 0, 0);

                        break;
                    }

                    cmdList->EndRenderpass();
                }, std::move(passData));
        }

        void WorldViewWindow::BindCommonResources(Graphics::RHI_CommandList* cmd_list, RenderData& renderData)
        {
            IS_PROFILE_FUNCTION();

            cmd_list->SetUniform(0, 0, renderData.FrameView);

            cmd_list->SetSampler(4, 0, renderData.BufferSamplers.Shadow_Sampler);
            cmd_list->SetSampler(4, 1, renderData.BufferSamplers.Repeat_Sampler);
            cmd_list->SetSampler(4, 2, renderData.BufferSamplers.Clamp_Sampler);
            cmd_list->SetSampler(4, 3, renderData.BufferSamplers.MirroredRepeat_Sampler);
        }

        Graphics::BufferFrame WorldViewWindow::GetBufferFrame() const
        {
            IS_PROFILE_FUNCTION();

            Graphics::BufferFrame bufferFrame;
            
            if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
            {
                m_editorCameraComponent->CreatePerspective(
                    m_editorCameraComponent->GetFovY()
                    , m_editorCameraComponent->GetAspect()
                    , m_editorCameraComponent->GetFarPlane()
                    , m_editorCameraComponent->GetNearPlane());
            }

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
            IS_PROFILE_FUNCTION();

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