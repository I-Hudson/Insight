#pragma once

#include "Editor/Build/BuildTarget.h"

#include "Core/ISysytem.h"
#include "Serialisation/ISerialisable.h"


namespace Insight
{
    namespace Editor
    {
        /// @brief Handle building the current open project.
        class BuildSystem : public Core::ISystem, public Serialisation::ISerialisable
        {
        public:
            BuildSystem();
            virtual ~BuildSystem() override;

            IS_SYSTEM(BuildSystem);
            IS_SERIALISABLE_H(BuildSystem);

            void SetBuildTarget(BuildTarget buildTarget);
            bool BuildProject();

        private:
            BuildTarget m_currentBuildTarget;
        };
    }

    namespace Serialisation
    {
        struct BuildSystem1 { };
        template<>
        struct ComplexSerialiser<BuildSystem1, void, Editor::BuildSystem>
        {
            void operator()(ISerialiser* serialiser, Editor::BuildSystem* buildSystem) const
            {
                if (serialiser->IsReadMode())
                {
                    serialiser->StartObject("BuildTarget");
                    std::string buildTarget;
                    serialiser->Read("BuildTarget", buildTarget);
                    serialiser->StopObject();

                    if (!buildTarget.empty())
                    {
                        buildSystem->m_currentBuildTarget.Name = buildTarget;
                    }
                }
                else
                {
                    serialiser->StartObject("BuildTarget");
                    serialiser->Write("BuildTarget", buildSystem->m_currentBuildTarget.Name);
                    serialiser->StopObject();
                }
            }
        };
    }

    OBJECT_SERIALISER(Editor::BuildSystem, 1,
        SERIALISE_COMPLEX_THIS(Serialisation::BuildSystem1, 1, 0)
    )
}