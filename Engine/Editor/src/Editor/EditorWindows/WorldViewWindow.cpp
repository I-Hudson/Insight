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
#include "Graphics/RenderGraph/RenderGraphBuilder.h"
#ifdef RENDERGRAPH_V2_ENABLED
#include "Graphics/RenderGraphV2/RenderGraphV2.h"
#endif

#include "Graphics/Enums.h"
#include "Graphics/ShaderDesc.h"

#include "ECS/Components/TransformComponent.h"
#include "ECS/Components/FreeCameraControllerComponent.h"

#include "Maths/Utils.h"

#include "Core/Logger.h"
#include "Core/Profiler.h"

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
            m_physicsDebugRenderPass.Destroy();
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
            m_editorCameraComponent->CreatePerspective(Maths::DegreesToRadians(90.0f), aspect, 0.1f, 1024.0f);
            m_editorCameraEntity->AddComponentByName(ECS::FreeCameraControllerComponent::Type_Name);

            m_physicsDebugRenderPass.Create();
        }

        void WorldViewWindow::OnDraw()
        {
            IS_PROFILE_FUNCTION();

            ImGuizmo::BeginFrame();
            ImGuizmo::SetDrawlist();

            SetupRenderGraphPasses();

            const char* editorOutputItems[] =
            {
                "EditorWorldColourRT",
                "EditorWorldLightRT",
                "EditorFSR_Output",
            };
            static int editorOutput = 0;
            ImGui::Combo("Editor Output", &editorOutput, editorOutputItems, ARRAY_COUNT(editorOutputItems));

            bool gpuSkinningEnabled = Runtime::AnimationSystem::Instance().IsGPUSkinningEnabled();
            ImGui::Checkbox("GPU Skinning", &gpuSkinningEnabled);
            Runtime::AnimationSystem::Instance().SetGPUSkinningEnabled(gpuSkinningEnabled);

            Graphics::RHI_Texture* worldViewTexture = Graphics::RenderGraph::Instance().GetRenderCompletedRHITexture(editorOutputItems[editorOutput]);
            if (worldViewTexture == nullptr)
            {
                return;
            }

            const ImVec2 imageCursorPos = ImGui::GetCursorScreenPos();
            const ImVec2 windowSize = ImGui::GetContentRegionAvail();
            ImGui::Image(worldViewTexture, windowSize);

            DrawViewButtons(imageCursorPos, ImVec2(windowSize.x, 0.0f));

            {
                // Render in game console window on top of the game viewport.
                const ImVec2 cursorPos = ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y);
                const u32 consoleWindowHeight = static_cast<u32>(windowSize.y * 0.3f);
                const u32 consoleWindowHalfHeight = static_cast<u32>(consoleWindowHeight * 0.5f);
                Core::Console::Instance().Render(static_cast<u32>(cursorPos.x), static_cast<u32>(cursorPos.y) - 30, static_cast<u32>(windowSize.x), 120);
            }

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
                // Set the transform gizmo for selected entities.
                WorldEntitiesWindow* worldEntitiesWindow = static_cast<WorldEntitiesWindow*>(worldEntitiesWindowBase);
                std::unordered_set<Core::GUID> selectedEntites = worldEntitiesWindow->GetSelectedEntities();
                if (!selectedEntites.empty())
                {
                    Maths::Matrix4 viewMatrix = m_editorCameraComponent->GetCamera().GetInvertedViewMatrix();
                    Maths::Matrix4 projectionMatrix = m_editorCameraComponent->GetProjectionMatrix();

                    ECS::Entity* entity = Runtime::WorldSystem::Instance().GetEntityByGUID(*selectedEntites.begin());
                    ECS::TransformComponent* transformComponent = entity->GetComponent<ECS::TransformComponent>();
                    Maths::Matrix4 entityMatrix = transformComponent->GetTransform();

                    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);
                    ImGuizmo::Manipulate(&viewMatrix[0][0], &projectionMatrix[0][0], m_imGuizmoOperation, ImGuizmo::LOCAL, &entityMatrix[0][0]);
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
            if (EditorGUI::ObjectFieldTarget(ContentWindow::c_ContentWindowAssetDragSource, resourceGuidString, Runtime::Asset::GetStaticTypeInfo().GetType()))
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

        void WorldViewWindow::DrawViewButtons(const ImVec2& cursorPos, const ImVec2& size)
        {
            const ImVec2 windowPos = ImVec2(cursorPos.x, cursorPos.y);

            ImGui::SetNextWindowPos(windowPos);
            ImGui::SetNextWindowSize(size);

            const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration;

            const ImVec4 consoleBackgroundColour(65.0f / 255.0f, 65.0f / 255.0f, 170.0f / 255.0f, 1.0);
            ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

            ImGui::PushStyleColor(ImGuiCol_WindowBg, consoleBackgroundColour);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

            if (ImGui::Begin("WorldViewWindowControls", nullptr, windowFlags))
            {
                if (ImGui::Button("Translate"))
                {
                    m_imGuizmoOperation = ImGuizmo::TRANSLATE;
                }
                ImGui::SameLine();

                if (ImGui::Button("Rotate"))
                {
                    m_imGuizmoOperation = ImGuizmo::ROTATE;
                }
                ImGui::SameLine();

                if (ImGui::Button("Scale"))
                {
                    m_imGuizmoOperation = ImGuizmo::SCALE;
                }
            }
            ImGui::End();

            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        void WorldViewWindow::SetupRenderGraphPasses()
        {
            IS_PROFILE_FUNCTION();

            const ECS::Camera camera = m_editorCameraComponent->GetCamera();
            const Maths::Matrix4 cameraTransform = m_editorCameraComponent->GetViewMatrix();
            Runtime::GraphicsSystem* graphicsSystem = App::Engine::Instance().GetSystemRegistry().GetSystem<Runtime::GraphicsSystem>();

            RenderFrame renderFrame;
            {
                IS_PROFILE_SCOPE("Copy render frame");
                renderFrame = graphicsSystem->GetRenderFrame();
            }
            renderFrame.SetCameraForAllWorlds(camera, cameraTransform);
            renderFrame.Sort();

            {
                IS_PROFILE_SCOPE("Set RenderData");
                m_renderingData.GetPending().RenderFrame = std::move(renderFrame);
                m_renderingData.GetPending().BufferFrame = GetBufferFrame();
                m_renderingData.GetPending().BufferSamplers = GetBufferSamplers();
            }

            {
                IS_PROFILE_SCOPE("RenderGraph AssSyncPoint");
                Graphics::RenderGraph::Instance().AddSyncPoint([this]()
                    {
                        m_renderingData.Swap();
                    });
            }

            {
                IS_PROFILE_SCOPE("RenderGraph AddPreRender");
                Graphics::RenderGraph::Instance().AddPreRender([this](Graphics::RenderGraph& renderGraph, Graphics::RHI_CommandList* cmdList)
                    {
                            IS_PROFILE_SCOPE("Upload WorldViewWindow Buffer Frame");
                            m_renderingData.GetCurrent().FrameView = cmdList->UploadUniform(m_renderingData.GetCurrent().BufferFrame);
                    });
            }

            {
                IS_PROFILE_SCOPE("RenderGraph passes");

                Graphics::PhysicsDebugRenderPass::ConstantBuffer constantBuffer
                {
                    m_editorCameraComponent->GetProjectionViewMatrix(),
                    m_editorCameraComponent->GetProjectionMatrix(),
                    m_editorCameraComponent->GetViewMatrix(),
                    m_editorCameraComponent->GetViewMatrix(),
                    m_editorCameraComponent->GetProjectionMatrix(),
                };

                LightShadowPass();
                if (m_enableDepthPrepass)
                {
                    GBufferDepthPrepass();
                }
                GBufferPass();
                TransparentGBufferPass();
                m_physicsDebugRenderPass.Render(constantBuffer, "EditorWorldColourRT", "EditorWorldDepthStencilRT");
                LightPass();
                FSR2Pass();
            }
        }

        void WorldViewWindow::LightShadowPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldTransparentGBufferData
            { };
            WorldTransparentGBufferData passData;

            Graphics::RenderGraph::Instance().AddPass<WorldTransparentGBufferData>("EditorWorldLightShadowPass",
                [](WorldTransparentGBufferData& data, Graphics::RenderGraphBuilder& builder)
                {
                    IS_PROFILE_SCOPE("LightShadowPass pass setup");

                    Graphics::ShaderDesc shaderDesc("LightShadowPass", {}, Graphics::ShaderStageFlagBits::ShaderStage_Vertex);
                    shaderDesc.InputLayout = Graphics::ShaderDesc::GetDefaultShaderInputLayout();
                    builder.SetShader(shaderDesc);

                    Graphics::PipelineStateObject pso = { };
                    {
                        pso.Name = "EditorWorldLightShadowPass";
                        pso.CullMode = Graphics::CullMode::Back;
                        pso.FrontFace = Graphics::FrontFace::CounterClockwise;
                        pso.DepthTest = true;
                        pso.DepthWrite = true;
                        pso.DepthClampEnabled = false;
                        pso.DepthBaisEnabled = true;

                        const float depthConstantBias = 0.05f;
                        const float depthSlopetBias = 1.0f;

                        pso.DepthConstantBaisValue = Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ) ? -depthConstantBias : depthConstantBias;
                        pso.DepthSlopeBaisValue = Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ) ? -depthSlopetBias : depthSlopetBias;

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
                    renderpassDescription.AddAttachment(Graphics::AttachmentDescription::Default(pso.DepthStencilFormat, Graphics::ImageLayout::ShaderReadOnly));
                    renderpassDescription.Attachments.back().InitalLayout = Graphics::ImageLayout::DepthStencilAttachment;
                    builder.SetRenderpass(renderpassDescription);

                    const int renderResolution = 1024;
                    builder.SetViewport(renderResolution, renderResolution);
                    builder.SetScissor(renderResolution, renderResolution);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorWorldLightShadowPass pass execute");

                    Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldLightShadowPass");
                    cmdList->BindPipeline(pso, nullptr);

                    {
                        IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                        BindCommonResources(cmdList, m_renderingData.GetCurrent());
                    }

                    const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                    for (size_t worldIdx = 0; worldIdx < renderFrame.RenderWorlds.size(); ++worldIdx)
                    {
                        const RenderWorld& renderWorld = renderFrame.RenderWorlds[worldIdx];
                        
                        {
                            IS_PROFILE_SCOPE("Shader Directional Lights");
                            for (size_t dirLightIdx = 0; dirLightIdx < renderWorld.DirectionalLights.size(); ++dirLightIdx)
                            {
                                const RenderDirectionalLight& directionalLight = renderWorld.DirectionalLights[dirLightIdx];
                                for (size_t cascadeIdx = 0; cascadeIdx < ECS::DirectionalLightComponent::c_cascadeCount; ++cascadeIdx)
                                {
                                    IS_PROFILE_SCOPE("DirectionalLight Side");

                                    Graphics::RenderpassDescription renderpassDescription = render_graph.GetRenderpassDescription("EditorWorldLightShadowPass");
                                    renderpassDescription.DepthStencil = directionalLight.DepthTexture;
                                    renderpassDescription.DepthStencilAttachment.Layer_Array_Index = static_cast<u32>(cascadeIdx);
                                    cmdList->BeginRenderpass(renderpassDescription);

                                    struct alignas(16) LightBuffer
                                    {
                                        Maths::Matrix4 ProjectionView;
                                    };
                                    LightBuffer lightBuffer =
                                    {
                                        directionalLight.ProjectionView[cascadeIdx]
                                    };
                                    {
                                        IS_PROFILE_SCOPE("SetUniform");
                                        cmdList->SetUniform(0, 1, lightBuffer);
                                    }

                                    for (const u64 meshIndex : renderWorld.OpaqueMeshIndexs)
                                    {
                                        const RenderMesh& mesh = renderWorld.Meshes[meshIndex];
                                        /*
                                        bool isVisable = false;
                                        {
                                            IS_PROFILE_SCOPE("Frustum Culling");
                                            Graphics::Frustum pointLightFrustum(pointLight.View[arrayIdx], pointLight.Projection, pointLight.Radius);
                                            isVisable = pointLightFrustum.IsVisible(Maths::Vector3(mesh.Transform[3]), mesh.BoudingBox.GetRadius());
                                        }

                                        if (!isVisable)
                                        {
                                            //continue;
                                        }
                                        */

                                        struct alignas(16) Object
                                        {
                                            Maths::Matrix4 Transform = Maths::Matrix4::Identity;
                                            int SkinnedMesh = 0;
                                        };
                                        Object object =
                                        {
                                            mesh.Transform,
                                            mesh.SkinnedMesh,
                                        };

                                        {
                                            IS_PROFILE_SCOPE("SetUniform");
                                            cmdList->SetUniform(1, 0, object);
                                        }

                                        if (mesh.SkinnedMesh)
                                        {
                                            RenderSetSkinnedMeshesBonesUniform(mesh, cmdList);
                                        }

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
                        
                        {
                            IS_PROFILE_SCOPE("Shader Point Lights");
                            for (size_t pointLightIdx = 0; pointLightIdx < renderWorld.PointLights.size(); ++pointLightIdx)
                            {
                                const RenderPointLight& pointLight = renderWorld.PointLights[pointLightIdx];
                                for (size_t arrayIdx = 0; arrayIdx < 6; ++arrayIdx)
                                {
                                    IS_PROFILE_SCOPE("PointLight Side");

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
                                    {
                                        IS_PROFILE_SCOPE("SetUniform");
                                        cmdList->SetUniform(0, 1, lightBuffer);
                                    }

                                    for (const u64 meshIndex : renderWorld.OpaqueMeshIndexs)
                                    {
                                        bool isVisable = false;
                                        const RenderMesh& mesh = renderWorld.Meshes[meshIndex];
                                        {
                                            IS_PROFILE_SCOPE("Frustum Culling");
                                            Graphics::Frustum pointLightFrustum(pointLight.View[arrayIdx], pointLight.Projection, pointLight.Radius);
                                            isVisable = pointLightFrustum.IsVisible(Maths::Vector3(mesh.Transform[3]), mesh.BoudingBox.GetRadius());
                                        }
                                        if (!isVisable)
                                        {
                                            //continue;
                                        }

                                        struct alignas(16) Object
                                        {
                                            Maths::Matrix4 Transform = Maths::Matrix4::Identity;
                                            int SkinnedMesh = 0;
                                        };
                                        Object object =
                                        {
                                            mesh.Transform,
                                            mesh.SkinnedMesh,
                                        };

                                        {
                                            IS_PROFILE_SCOPE("SetUniform");
                                            cmdList->SetUniform(1, 0, object);
                                        }

                                        if (mesh.SkinnedMesh)
                                        {
                                            RenderSetSkinnedMeshesBonesUniform(mesh, cmdList);
                                        }

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
                    }
                }, std::move(passData));
        }

        void WorldViewWindow::GBufferDepthPrepass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldDepthPrepassData
            {
                Maths::Vector2 RenderResolution;
            };

            WorldDepthPrepassData passData =
            {
                Maths::Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y)
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
                    BindCommonResources(cmdList, m_renderingData.GetCurrent());
                }

                const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                for (const RenderWorld& world : renderFrame.RenderWorlds)
                {

                    for (const u64 meshIndex : world.OpaqueMeshIndexs)
                    {
                        IS_PROFILE_SCOPE("Draw Entity");
                        const RenderMesh& mesh = world.Meshes[meshIndex];

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
                Maths::Vector2 RenderResolution;
                bool DepthPrepassEnabled;
            };

            WorldGBufferData passData =
            {
                Maths::Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y),
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
                        BindCommonResources(cmdList, m_renderingData.GetCurrent());
                    }

                    const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                    for (const RenderWorld& world : renderFrame.RenderWorlds)
                    {
                        const RenderCamera& mainCamera = world.MainCamera;
                        for (const u64 meshIndex : world.OpaqueMeshIndexs)
                        {
                            IS_PROFILE_SCOPE("Draw Entity");
                            const RenderMesh& mesh = world.Meshes[meshIndex];

                            const Graphics::Frustum mainCameraFrustm(
                                mainCamera.Camera.GetViewMatrix(),
                                mainCamera.Camera.GetProjectionMatrix(),
                                mainCamera.Camera.GetFarPlane());
                            const bool isVisable = mainCameraFrustm.IsVisible(Maths::Vector3(mesh.Transform[3]), mesh.BoudingBox.GetRadius());
                            if (!isVisable)
                            {
                                //continue;
                            }

                            Graphics::BufferPerObject object = {};
                            object.Transform = mesh.Transform;
                            object.Previous_Transform = mesh.PreviousTransform;

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

                            object.SkinnedMesh = mesh.SkinnedMesh;
                            if (mesh.SkinnedMesh)
                            {
                                RenderSetSkinnedMeshesBonesUniform(mesh, cmdList);
                            }

                            cmdList->SetUniform(2, 0, object);

                            const Runtime::MeshLOD& renderMeshLod = mesh.GetLOD(0);
                            if (renderMeshLod.VertexBufferView.IsValid())
                            {
                                cmdList->SetVertexBuffer(renderMeshLod.VertexBufferView.GetBuffer());
                            }
                            else
                            {
                                cmdList->SetVertexBuffer(renderMeshLod.Vertex_buffer);
                            }

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
                Maths::Vector2 RenderResolution;
            };

            WorldTransparentGBufferData passData;
            passData.RenderResolution = Maths::Vector2(ImGui::GetWindowSize().x, ImGui::GetWindowSize().y);

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
                        BindCommonResources(cmdList, m_renderingData.GetCurrent());
                    }

                    const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                    for (const RenderWorld& world : renderFrame.RenderWorlds)
                    {
                        for (const u64 meshIndex : world.TransparentMeshIndexs)
                        {
                            IS_PROFILE_SCOPE("Draw Entity");
                            const RenderMesh& mesh = world.Meshes[meshIndex];

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

                            object.SkinnedMesh = mesh.SkinnedMesh;
                            if (object.SkinnedMesh)
                            {
                                struct SkinnedBonesMatrices
                                {
                                    Maths::Matrix4 BoneMatrices[Runtime::Skeleton::c_MaxBoneCount];
                                };
                                ASSERT(mesh.BoneTransforms.size() <= Runtime::Skeleton::c_MaxBoneCount);

                                SkinnedBonesMatrices skinnedBonesMatrices;
                                if (mesh.BoneTransforms.size() > 0)
                                {
                                    Platform::MemCopy(
                                        &skinnedBonesMatrices.BoneMatrices[0],
                                        mesh.BoneTransforms.data(),
                                        mesh.BoneTransforms.size() * sizeof(mesh.BoneTransforms[0]));
                                }
                                cmdList->SetUniform(2, 2, skinnedBonesMatrices);
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

        bool ComputePassForLight = false;
        void WorldViewWindow::LightPass()
        {
            IS_PROFILE_FUNCTION();

            struct WorldTransparentGBufferData
            {
                bool ComputePass = false;
            };
            WorldTransparentGBufferData passData
            {
                ComputePassForLight
            };
            ImGui::Checkbox("LightPassCompute", &ComputePassForLight);

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
                        , Graphics::ImageUsageFlagsBits::ColourAttachment | Graphics::ImageUsageFlagsBits::Sampled | Graphics::ImageUsageFlagsBits::Storage);
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

                    Graphics::ShaderDesc shaderDescCompute("LightPassCompute", {}, Graphics::ShaderStageFlagBits::ShaderStage_Compute);
                    Graphics::ComputePipelineStateObject computePso = {};
                    computePso.Name = "ComputeEditorLightPass";
                    computePso.ShaderDescription = shaderDescCompute;
                    builder.SetComputePipeline(computePso);
                    if (data.ComputePass)
                    {
                        //builder.SetShader(shaderDescCompute);
                    }

                    builder.SetViewport(renderResolutionX, renderResolutionY);
                    builder.SetScissor(renderResolutionX, renderResolutionY);
                },
                [this](WorldTransparentGBufferData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmdList)
                {
                    IS_PROFILE_SCOPE("EditorLightPass pass execute");

                    if (!data.ComputePass)
                    {
                        Graphics::PipelineStateObject pso = render_graph.GetPipelineStateObject("EditorWorldLightPass");
                        cmdList->BindPipeline(pso, nullptr);
                        cmdList->BeginRenderpass(render_graph.GetRenderpassDescription("EditorWorldLightPass"));

                        {
                            IS_PROFILE_SCOPE("Set Buffer Frame Uniform");
                            BindCommonResources(cmdList, m_renderingData.GetCurrent());
                        }

                        cmdList->SetTexture(6, 0, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldDepthStencilRT")));
                        cmdList->SetTexture(6, 1, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldColourRT")));
                        cmdList->SetTexture(6, 2, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldNormalRT")));
                        //cmdList->SetTexture(1, 7, render_graph.GetRHITexture(""));

                        const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                        for (const RenderWorld& world : renderFrame.RenderWorlds)
                        {
                            const u32 c_MaxDirectionalLights = 8;
                            const u32 c_MaxPointLights = 32;

                            struct DirectionalLightBuffer
                            {
                                RenderDirectionalLight DirectionalLights[c_MaxDirectionalLights];
                                u32 DirectionalLightSize;
                            };
                            DirectionalLightBuffer directionalLightBuffer;
                            {
                                IS_PROFILE_SCOPE("Set directional light data");
                                for (u32 i = 0; i < world.DirectionalLights.size(); ++i)
                                {
                                    if (i >= c_MaxDirectionalLights)
                                    {
                                        FAIL_ASSERT_MSG("Only '%d' directional lights are supported.", c_MaxDirectionalLights);
                                        break;
                                    }

                                    directionalLightBuffer.DirectionalLights[i] = world.DirectionalLights[i];
                                    cmdList->SetTexture(7, 0 + i, world.DirectionalLights[i].DepthTexture);
                                }
                                directionalLightBuffer.DirectionalLightSize = static_cast<u32>(world.DirectionalLights.size());
                            }
                            Graphics::RHI_BufferView directionalLightRHIBuffer = cmdList->UploadUniform(directionalLightBuffer);
                            {
                                IS_PROFILE_SCOPE("SetUniform");
                                cmdList->SetUniform(6, 0, directionalLightRHIBuffer);
                            }

                            struct PointLightBuffer
                            {
                                RenderPointLight PointLights[c_MaxPointLights];
                                int PointLightSize;
                            };

                            PointLightBuffer pointLightBuffer;
                            {
                                IS_PROFILE_SCOPE("Set point light data");
                                for (u32 i = 0; i < world.PointLights.size(); ++i)
                                {
                                    if (i >= 32)
                                    {
                                        FAIL_ASSERT_MSG("Only 32 point lights are supported.");
                                        break;
                                    }

                                    pointLightBuffer.PointLights[i] = world.PointLights[i];
                                    cmdList->SetTexture(7, 0 + i, world.PointLights[i].DepthTexture);
                                }
                                pointLightBuffer.PointLightSize = static_cast<u32>(world.PointLights.size());
                            }

                            Graphics::RHI_BufferView spotLightRHIBuffer = cmdList->UploadUniform(pointLightBuffer);
                            {
                                IS_PROFILE_SCOPE("SetUniform");
                                cmdList->SetUniform(6, 1, spotLightRHIBuffer);
                            }

                            cmdList->Draw(3, 1, 0, 0);

                            break;
                        }

                        cmdList->EndRenderpass();
                    }
                    else
                    {
                        Graphics::RHI_Texture* outputTexture = render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldLightRT"));

                        Graphics::PipelineBarrier beforeBarreir;
                        beforeBarreir.SrcStage = static_cast<u32>(Graphics::PipelineStageFlagBits::TopOfPipe);
                        beforeBarreir.DstStage = static_cast<u32>(Graphics::PipelineStageFlagBits::ComputeShader);

                        Graphics::ImageBarrier beforeImageBarrier;
                        beforeImageBarrier.SrcAccessFlags = Graphics::AccessFlagBits::None;
                        beforeImageBarrier.DstAccessFlags = Graphics::AccessFlagBits::ShaderWrite;
                        beforeImageBarrier.OldLayout = outputTexture->GetLayout();
                        beforeImageBarrier.NewLayout = Graphics::ImageLayout::ShaderWrite;
                        beforeImageBarrier.SubresourceRange = Graphics::ImageSubresourceRange::SingleMipAndLayer(Graphics::ImageAspectFlagBits::Colour);
                        beforeImageBarrier.Image = outputTexture;
                        beforeBarreir.ImageBarriers.push_back(beforeImageBarrier);

                        cmdList->PipelineBarrier(beforeBarreir);

                        Graphics::ComputePipelineStateObject pso = render_graph.GetComputePipelineStateObject("ComputeEditorLightPass");
                        cmdList->BindPipeline(pso);

                        BindCommonResources(cmdList, m_renderingData.GetCurrent());

                        cmdList->SetUnorderedAccess(0, 0, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldLightRT")));
                        cmdList->SetTexture(0, 0, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldDepthStencilRT")));
                        cmdList->SetTexture(0, 1, render_graph.GetRHITexture(render_graph.GetTexture("EditorWorldColourRT")));

                        const RenderFrame& renderFrame = m_renderingData.GetCurrent().RenderFrame;
                        for (const RenderWorld& world : renderFrame.RenderWorlds)
                        {
                            const u32 c_MaxPointLights = 32;

                            struct PointLightBuffer
                            {
                                RenderPointLight PointLights[c_MaxPointLights];
                                int PointLightSize;
                            };
                            PointLightBuffer pointLightBuffer;
                            {
                                IS_PROFILE_SCOPE("Set point light data");
                                for (u32 i = 0; i < world.PointLights.size(); ++i)
                                {
                                    if (i >= 32)
                                    {
                                        FAIL_ASSERT_MSG("Only 32 point lights are supported.");
                                        break;
                                    }

                                    pointLightBuffer.PointLights[i] = world.PointLights[i];
                                    cmdList->SetTexture(1, 0 + i, world.PointLights[i].DepthTexture);
                                }
                                pointLightBuffer.PointLightSize = static_cast<u32>(world.PointLights.size());
                            }

                            if (pointLightBuffer.PointLightSize > 0)
                            {
                                Graphics::RHI_BufferView spotLightRHIBuffer = cmdList->UploadUniform(pointLightBuffer);
                                {
                                    IS_PROFILE_SCOPE("Set Light Uniform");
                                    cmdList->SetUniform(1, 0, spotLightRHIBuffer);
                                }
                            }
                        }

                        // The descriptor allocator should "request" descriptor handles and such from resources as the resource can't just make every descriptor
                        // for itself. Or the resource should make descriptors depending on the 'ImageUsageFlagsBits' it has been given.

                        const float thread_group_count = 8.0f;
                        const uint32_t threadGroupCountX = static_cast<uint32_t>(std::ceil(static_cast<float>(outputTexture->GetWidth()) / thread_group_count));
                        const uint32_t threadGroupCountY = static_cast<uint32_t>(std::ceil(static_cast<float>(outputTexture->GetHeight()) / thread_group_count));

                        cmdList->Dispatch(threadGroupCountX, threadGroupCountY);


                        Graphics::PipelineBarrier afterBarreir;
                        afterBarreir.SrcStage = static_cast<u32>(Graphics::PipelineStageFlagBits::ComputeShader);
                        afterBarreir.DstStage = static_cast<u32>(Graphics::PipelineStageFlagBits::FragmentShader);

                        Graphics::ImageBarrier afterImageBarrier;
                        afterImageBarrier.SrcAccessFlags = beforeImageBarrier.DstAccessFlags;
                        afterImageBarrier.DstAccessFlags = Graphics::AccessFlagBits::ShaderRead;
                        afterImageBarrier.OldLayout = beforeImageBarrier.NewLayout;
                        afterImageBarrier.NewLayout = Graphics::ImageLayout::ShaderReadOnly;
                        afterImageBarrier.SubresourceRange = Graphics::ImageSubresourceRange::SingleMipAndLayer(Graphics::ImageAspectFlagBits::Colour);
                        afterImageBarrier.Image = outputTexture;
                        afterBarreir.ImageBarriers.push_back(afterImageBarrier);

                        cmdList->PipelineBarrier(afterBarreir);
                    }
                }, std::move(passData));
        }

        void WorldViewWindow::FSR2Pass()
        {
            IS_PROFILE_FUNCTION();

            static float fsrSharpness;
            bool reset = false;
            if (ImGui::Checkbox("Editor Enable FSR", &m_fsr2Enabled))
            {
                //reset = true;
            }
            ImGui::DragFloat("Editor FSR sharpness", &fsrSharpness, 0.05f, 0.0f, 1.0f);

            if (m_renderResolution == Maths::Vector2(0, 0))
            {
                m_renderResolution = Graphics::RenderGraph::Instance().GetRenderResolution();
            }

            int renderRes[2] = { m_renderResolution.x, m_renderResolution.y };
            ImGui::InputInt2("Render Resolution", renderRes);
            if (ImGui::Button("Apply Render Resolution"))
            {
                m_renderResolution = Maths::Vector2(renderRes[0], renderRes[1]);
                Graphics::RenderGraph::Instance().SetRenderResolution(m_renderResolution);
                return;
            }

            if (Graphics::RenderGraph::Instance().GetRenderResolution() == Graphics::RenderGraph::Instance().GetOutputResolution()
                || !m_fsr2Enabled)
            {
                return;
            }

            struct PassData
            {
                float NearPlane;
                float FarPlane;
                float FOVY;
            };
            PassData passData = {};

            passData.NearPlane = m_editorCameraComponent->GetCamera().GetNearPlane();
            passData.FarPlane = m_editorCameraComponent->GetCamera().GetFarPlane();
            passData.FOVY = m_editorCameraComponent->GetCamera().GetFovY();

            Graphics::RenderGraph::Instance().AddPass<PassData>("EditorFSR2",
                [](PassData& data, Graphics::RenderGraphBuilder& builder)
                {
                    Graphics::RHI_TextureInfo create_info = Graphics::RHI_TextureInfo::Tex2D(
                        builder.GetOutputResolution().x
                        , builder.GetOutputResolution().y
                        , PixelFormat::R8G8B8A8_UNorm
                        , Graphics::ImageUsageFlagsBits::Sampled | Graphics::ImageUsageFlagsBits::Storage);
                    Graphics::RGTextureHandle textureHandle = builder.CreateTexture("EditorFSR_Output", create_info);

                    builder.SetViewport(Graphics::RenderGraph::Instance().GetOutputResolution().x, Graphics::RenderGraph::Instance().GetOutputResolution().y);
                    builder.SetScissor(Graphics::RenderGraph::Instance().GetOutputResolution().x, Graphics::RenderGraph::Instance().GetOutputResolution().y);

                    builder.SkipTextureWriteBarriers();
                },
                [this, reset](PassData& data, Graphics::RenderGraph& render_graph, Graphics::RHI_CommandList* cmd_list)
                {
                    Graphics::RHI_FSR::Instance().Dispatch(cmd_list
                    , render_graph.GetRHITexture("EditorWorldLightRT")
                    , render_graph.GetRHITexture("EditorWorldDepthStencilRT")
                    , render_graph.GetRHITexture("EditorWorldVelocityRT")
                    , render_graph.GetRHITexture("EditorFSR_Output")
                    , data.NearPlane
                    , data.FarPlane
                    , data.FOVY
                    , App::Engine::s_FrameTimer.GetElapsedTimeMillFloat()
                    , fsrSharpness
                    , reset);
                }, std::move(passData));
        }

        void WorldViewWindow::RenderSetSkinnedMeshesBonesUniform(const RenderMesh& mesh, Graphics::RHI_CommandList* cmdList) const
        {
            if (mesh.SkinnedMesh)
            {
                struct SkinnedBonesMatrices
                {
                    Maths::Matrix4 BoneMatrices[Runtime::Skeleton::c_MaxBoneCount];
                };
                ASSERT(mesh.BoneTransforms.size() <= Runtime::Skeleton::c_MaxBoneCount);

                SkinnedBonesMatrices skinnedBonesMatrices;
                if (mesh.BoneTransforms.size() > 0)
                {
                    Platform::MemCopy(
                        &skinnedBonesMatrices.BoneMatrices[0],
                        mesh.BoneTransforms.data(),
                        mesh.BoneTransforms.size() * sizeof(mesh.BoneTransforms[0]));
                }
                cmdList->SetUniform(2, 2, skinnedBonesMatrices);
            }
            else
            {
                FAIL_ASSERT_MSG("[WorldViewWindow::RenderSetSkinnedMeshesBonesUniform] Trying to set the uniform for a skinned mesh, when mesh is not skinned.");
            }
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

        Graphics::BufferFrame WorldViewWindow::GetBufferFrame()
        {
            IS_PROFILE_FUNCTION();

            Graphics::BufferFrame bufferFrame;
            
            Graphics::RHI_FSR::Instance().GenerateJitterSample(&bufferFrame.TAA_Jitter_Current[0], &bufferFrame.TAA_Jitter_Current[1]);
            bufferFrame.TAA_Jitter_Previous[0] = m_taaJitterPrevious.x;
            bufferFrame.TAA_Jitter_Previous[1] = m_taaJitterPrevious.y;

            m_taaJitterPrevious = Maths::Vector2(bufferFrame.TAA_Jitter_Current[0], bufferFrame.TAA_Jitter_Current[1]);

            if (Graphics::RenderContext::Instance().IsRenderOptionsEnabled(Graphics::RenderOptions::ReverseZ))
            {
                m_editorCameraComponent->CreatePerspective(
                    m_editorCameraComponent->GetFovY()
                    , m_editorCameraComponent->GetAspect()
                    , m_editorCameraComponent->GetFarPlane()
                    , m_editorCameraComponent->GetNearPlane());
            }

            if (m_fsr2Enabled)
            {
                Maths::Matrix4 projection = m_editorCameraComponent->GetProjectionMatrix();
                Maths::Matrix4 translation = Maths::Matrix4::Identity;
                translation[3] = Maths::Vector4(bufferFrame.TAA_Jitter_Current[0], bufferFrame.TAA_Jitter_Current[1], 0.0f, 1.0f);
                projection *= translation;
                m_editorCameraComponent->GetCamera().SetProjectionMatrix(projection);
            }

            bufferFrame.Proj_View = m_editorCameraComponent->GetProjectionViewMatrix();
            bufferFrame.Projection = m_editorCameraComponent->GetProjectionMatrix();
            bufferFrame.View = m_editorCameraComponent->GetViewMatrix();

            bufferFrame.View_Inverted = m_editorCameraComponent->GetInvertedViewMatrix();
            bufferFrame.Projection_View_Inverted = m_editorCameraComponent->GetInvertedProjectionViewMatrix();

            bufferFrame.Render_Resolution[0] = Graphics::RenderGraph::Instance().GetRenderResolution().x;
            bufferFrame.Render_Resolution[1] = Graphics::RenderGraph::Instance().GetRenderResolution().y;

            bufferFrame.Ouput_Resolution[0] = Graphics::RenderGraph::Instance().GetOutputResolution().x;
            bufferFrame.Ouput_Resolution[1] = Graphics::RenderGraph::Instance().GetOutputResolution().y;

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