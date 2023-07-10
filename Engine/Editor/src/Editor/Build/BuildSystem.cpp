#include "Editor/Build/BuildSystem.h"

#include "Resource/ResourceManager.h"

namespace Insight::Editor
{
    BuildSystem::BuildSystem()
    {
    }

    BuildSystem::~BuildSystem()
    {
    }

    IS_SERIALISABLE_CPP(BuildSystem);

    void BuildSystem::SetBuildTarget(BuildTarget buildTarget)
    {
        m_currentBuildTarget = buildTarget;
    }

    bool BuildSystem::BuildProject()
    {
        return false;
    }

    bool BuildSystem::BuildLooseResourcePack()
    {
        return false;
    }

}