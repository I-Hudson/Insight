#include "Editor/EditorWindows/PhysicsWindow.h"

#include "World/WorldSystem.h"
#include "ECS/Components/PhysicsComponent.h"

#include "Physics/PhysicsWorld.h"

#include <imgui.h>

namespace Insight
{
    namespace Editor
    {
        PhysicsWindow::PhysicsWindow()
            : IEditorWindow()
        { }
        PhysicsWindow::PhysicsWindow(u32 minWidth, u32 minHeight)
            : IEditorWindow(minWidth, minHeight)
        { }
        PhysicsWindow::PhysicsWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
            : IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
        { }
        PhysicsWindow::~PhysicsWindow()
        { }

        void PhysicsWindow::OnDraw()
        {
            TObjectPtr<Runtime::World> activeWorld = Runtime::WorldSystem::Instance().GetActiveWorld();
            if (!activeWorld)
            {
                return;
            }

            if (ImGui::TreeNodeEx("World", ImGuiTreeNodeFlags_CollapsingHeader))
            {
                if (Physics::PhysicsWorld::IsRecording())
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                    ImGui::Text("Status: Recording");
                    ImGui::PopStyleColor();
                }
                else
                {
                    ImGui::Text("Status: Not Recording");
                }

                if (ImGui::Button("Start Recording"))
                {
                    Physics::PhysicsWorld::StartRecord();
                }
                else if (ImGui::Button("End Recording"))
                {
                    Physics::PhysicsWorld::EndRecord();
                }

                //ImGui::TreePop();
            }

            const std::vector<Ptr<ECS::Entity>> physicsEntities = activeWorld->GetAllEntitiesWithComponent<ECS::PhysicsComponent>();
            if (ImGui::TreeNodeEx("Entities", 
                ImGuiTreeNodeFlags_CollapsingHeader
                | ImGuiTreeNodeFlags_SpanFullWidth))
            {
                for (size_t i = 0; i < physicsEntities.size(); ++i)
                {
                    const Ptr<ECS::Entity>& entity = physicsEntities[i];
                    std::string treeTitle = entity->GetName();
                    if (treeTitle.empty())
                    {
                        treeTitle = "##" + entity->GetGUID().ToString();
                    }

                    if (ImGui::TreeNodeEx(treeTitle.c_str(),
                        ImGuiTreeNodeFlags_CollapsingHeader 
                        | ImGuiTreeNodeFlags_SpanFullWidth))
                    {
                        const Ptr<ECS::PhysicsComponent> physicsComponent = entity->GetComponent<ECS::PhysicsComponent>();
                        if (!physicsComponent)
                        {
                            continue;
                        }

                        ImGui::Text("%s", physicsComponent->IsEnabled() ? "Enabled" : "Disabled");
                        //ImGui::TreePop();
                    }
                }

                //ImGui::TreePop();
            }
        }
    }
}