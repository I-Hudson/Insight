#include "Editor/Premake/PremakeProjectTemplateData.h"

#include "Core/EnginePaths.h"
#include "Runtime/ProjectSystem.h"

namespace Insight::Editor
{
    PremakeProjectTemplateData PremakeProjectTemplateData::CreateFromProjectInfo()
    {
        PremakeProjectTemplateData data;
        data.ProjectName = Runtime::ProjectSystem::Instance().GetProjectInfo().ProjectName;
        data.InsightRootPath = EnginePaths::GetRootPath();
        return data;
    }
}