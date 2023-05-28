#include "GenerateFiles/GenerateProjectInitialise.h"
#include "Utils.h"

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateProjectInitialise::Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

            file << "#include \"Core/ImGuiSystem.h\"\n";

            file << "#include \"EditorWindows.gen.h\"\n";
            file << "#include \"RegisterComponents.gen.h\"\n";
            file << "#include \"Core/Logger.h\"\n";

            file << "\n";

            file << "#ifndef IS_MONOLITH\n";
            file << "#ifdef IS_EXPORT_PROJECT_DLL\n";
            file << "#define IS_PROJECT __declspec(dllexport)\n";
            file << "#else\n";
            file << "#define IS_PROJECT __declspec(dllimport)\n";
            file << "#endif\n";
            file << "#else\n";
            file << "#define IS_PROJECT\n";
            file << "#endif\n\n";

            file << "namespace Insight\n";
            file << "{\n";

            file << "\textern \"C\" IS_PROJECT void ProjectModuleInitialise(Core::ImGuiSystem* imguiSystem)\n";
            file << "\t{\n";
            file << "\t\tSET_IMGUI_CURRENT_CONTEXT();\n";
            file << "\t\tEditor::RegisterAllEditorWindows();\n";
            file << "\t\tECS::RegisterAllComponents();\n";
            file << "\t\tIS_INFO(\"Project DLL module initialised\");\n";
            file << "\t}\n";

            file << "}\n";

            return true;
        }
        else
        {
            return false;
        }
    }
}