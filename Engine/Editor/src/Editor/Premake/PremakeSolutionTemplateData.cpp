#include "Editor/Premake/PremakeSolutionTemplateData.h"

#include "Core/EnginePaths.h"
#include "Runtime/ProjectSystem.h"

namespace Insight::Editor
{
    PremakeSolutionTemplateData PremakeSolutionTemplateData::CreateFromProjectInfo(const char* solutonName)
    {
        PremakeSolutionTemplateData data;
        data.SolutionName = solutonName;
        data.ProjectName = Runtime::ProjectSystem::Instance().GetProjectInfo().ProjectName;
        data.InsightRootPath = EnginePaths::GetInstallDirectory();

        return data;
    }
}