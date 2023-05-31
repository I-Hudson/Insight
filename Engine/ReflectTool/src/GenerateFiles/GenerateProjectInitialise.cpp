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

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT void ", "ProjectModuleInitialise", 
                { "Core::ImGuiSystem* imguiSystem" }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "SET_IMGUI_CURRENT_CONTEXT();" << NEW_LINE;
                    TAB_N(indent);
                    file << "Editor::RegisterAllEditorWindows();" << NEW_LINE;                    
                    TAB_N(indent);
                    file << "ECS::RegisterAllComponents();" << NEW_LINE;
                    TAB_N(indent);
                    file << "IS_INFO(\"Project DLL module initialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT void", "ProjectModuleUninitialise", { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "Editor::UnregisterAllEditorWindows();" << NEW_LINE;
                    TAB_N(indent);
                    file << "ECS::UnregisterAllComponents();" << NEW_LINE;
                    TAB_N(indent);
                    file << "IS_INFO(\"Project DLL module uninitialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT std::vector<std::string>", "ProjectModuleGetEditorWindowNames", { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "return Editor::GetAllEditorWindowNames();" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "extern \"C\" IS_PROJECT std::vector<std::string>", "ProjectModuleGetComponentNames", { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "return ECS::GetAllComponentNames();" << NEW_LINE;
                }, 1);


            file << "}\n";

            return true;
        }
        else
        {
            return false;
        }
    }
}