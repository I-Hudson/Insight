#include "GenerateFiles/GenerateProjectInitialise.h"
#include "Utils.h"

#include "Editor/HotReload/HotReloadExportFunctions.h"

#include <filesystem>

namespace InsightReflectTool
{
    bool GenerateProjectInitialise::GenerateHeader(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

            file << "#ifdef IS_EDITOR" << NEW_LINE;
            Utils::WriteIncludeFile(file, "Editor/HotReload/HotReloadMetaData.h");
            file << "#endif" << NEW_LINE;

            Utils::WriteIncludeLibraryFile(file, "vector");
            Utils::WriteIncludeLibraryFile(file, "string");

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

            file << "namespace Core" << NEW_LINE;
            file << "{" << NEW_LINE;
            file << "class ImGuiSystem;" << NEW_LINE;
            file << "}" << NEW_LINE;

            file << "#ifdef IS_PACKAGE_BUILD" << NEW_LINE;

            file << "IS_PROJECT void " << ProjectModule::c_Initialise << "(Core::ImGuiSystem* imguiSystem);" << NEW_LINE;
            file << "IS_PROJECT void " << ProjectModule::c_Uninitialise << "();" << NEW_LINE;
            file << "IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetEditorWindowNames << "();" << NEW_LINE;
            file << "IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetComponentNames << "();" << NEW_LINE;
            file << "IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetTypeDrawerNames << "();" << NEW_LINE;

            file << "#else" << NEW_LINE;

            file << "extern \"C\" IS_PROJECT void " << ProjectModule::c_Initialise << "(Core::ImGuiSystem* imguiSystem);" << NEW_LINE;
            file << "extern \"C\" IS_PROJECT void " << ProjectModule::c_Uninitialise << "();" << NEW_LINE;
            file << "extern \"C\" IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetEditorWindowNames << "();" << NEW_LINE;
            file << "extern \"C\" IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetComponentNames << "();" << NEW_LINE;
            file << "extern \"C\" IS_PROJECT std::vector<std::string> " << ProjectModule::c_GetTypeDrawerNames << "();" << NEW_LINE;

            file << "#endif" << NEW_LINE;

            file << "#ifdef IS_EDITOR" << NEW_LINE;
            file << "extern \"C\" IS_PROJECT ::Insight::Editor::HotReloadMetaData " << ProjectModule::c_GetMetaData << "();" << NEW_LINE;
            file << "#endif" << NEW_LINE;

            file << "}\n";

            return true;
        }
        else
        {
            return false;
        }
    }

    bool GenerateProjectInitialise::GenerateSource(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const
    {
        std::fstream file;
        std::string absPath = std::filesystem::absolute(outFilePath).string();
        Utils::ValidateOutputPath(absPath);

        file.open(absPath, std::ios::out | std::ios::trunc);
        if (file.is_open())
        {
            Utils::WriteGeneratedFileHeader(file);

            Utils::WriteIncludeFile(file, "ProjectInitialise.gen.h");
            Utils::WriteIncludeFile(file, "Core/ImGuiSystem.h");
            Utils::WriteIncludeFile(file, "EditorWindows.gen.h");
            Utils::WriteIncludeFile(file, "RegisterComponents.gen.h");
            Utils::WriteIncludeFile(file, "TypeDrawers.gen.h");
            Utils::WriteIncludeFile(file, "Core/Logger.h");

            file << "namespace Insight\n";
            file << "{\n";

            Utils::WriteSourceFunctionDefinition(file, "void ", ProjectModule::c_Initialise,
                { "Core::ImGuiSystem* imguiSystem" }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "SET_IMGUI_CURRENT_CONTEXT();" << NEW_LINE;
                    TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                    TAB_N(indent); file << "Editor::" << EditorWindowRegister::c_RegisterAllEditorWindows << "();" << NEW_LINE;
                    TAB_N(indent); file << "Editor::" << TypeDrawerRegister::c_RegisterAllTypeDrawers << "(); " << NEW_LINE;
                    TAB_N(indent); file << "#endif" << NEW_LINE;

                    TAB_N(indent); file << "ECS::Project::"<< ComponentRegister::c_RegisterAllComponents << "(); " << NEW_LINE;
                    TAB_N(indent);  file << "IS_LOG_INFO(\"Project DLL module initialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "void", ProjectModule::c_Uninitialise, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    
                    TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                    TAB_N(indent); file << "Editor::" << EditorWindowRegister::c_UnregisterAllEditorWindows <<"();" << NEW_LINE;
                    TAB_N(indent); file << "Editor::" << TypeDrawerRegister::c_UnregisterAllTypeDrawers << "();" << NEW_LINE;
                    TAB_N(indent); file << "#endif" << NEW_LINE;

                    TAB_N(indent); file << "ECS::Project::" << ComponentRegister::c_UnregisterAllComponents << "();" << NEW_LINE;
                    TAB_N(indent); file << "IS_LOG_INFO(\"Project DLL module uninitialised\");" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", ProjectModule::c_GetEditorWindowNames, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                    TAB_N(indent); file << "return Editor::" << EditorWindowRegister::c_GetAllEditorWindowNames << "();" << NEW_LINE;
                    TAB_N(indent); file << "#else" << NEW_LINE;
                    TAB_N(indent); file << "return std::vector<std::string>();" << NEW_LINE;
                    TAB_N(indent); file << "#endif" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", ProjectModule::c_GetComponentNames, { }, [&](std::fstream& file)
                {
                    const int indent = 2;
                    TAB_N(indent);
                    file << "return ECS::Project::" << ComponentRegister::c_GetAllComponentNames << "();" << NEW_LINE;
                }, 1);

            Utils::WriteSourceFunctionDefinition(file, "std::vector<std::string>", ProjectModule::c_GetTypeDrawerNames, { }, [&](std::fstream& file)
            {
                const int indent = 2;
                TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                TAB_N(indent); file << "return Editor::" << TypeDrawerRegister::c_GetAllTypeDrawerNames << "();" << NEW_LINE;
                TAB_N(indent); file << "#else" << NEW_LINE;
                TAB_N(indent); file << "return std::vector<std::string>();" << NEW_LINE;
                TAB_N(indent); file << "#endif" << NEW_LINE;
            }, 1);

            file << "#ifdef IS_EDITOR" << NEW_LINE;
            Utils::WriteSourceFunctionDefinition(file, "::Insight::Editor::HotReloadMetaData", ProjectModule::c_GetMetaData, { }, [&](std::fstream& file)
            {
                const int indent = 2;
                TAB_N(indent); file << "::Insight::Editor::HotReloadMetaData metaData;" << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                TAB_N(indent); file << "metaData.EditorWindowNames = std::move(Editor::" << EditorWindowRegister::c_GetAllEditorWindowNames << "()); " << NEW_LINE;
                TAB_N(indent); file << "#endif" << NEW_LINE;

                TAB_N(indent); file << "metaData.ComponentNames = std::move(ECS::" << StateInfo::Instance().GetTypeString() << "::" << ComponentRegister::c_GetAllComponentNames << "()); " << NEW_LINE;
                TAB_N(indent); file << "metaData.TypeDrawerNames = std::move(Editor::" << TypeDrawerRegister::c_GetAllTypeDrawerNames << "()); " << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "#ifdef IS_EDITOR" << NEW_LINE;
                TAB_N(indent); file << "metaData.EditorWindowTypeInfos = std::move(Editor::" << EditorWindowRegister::c_GetAllEditorWindowsTypeInfos << "());" << NEW_LINE;
                TAB_N(indent); file << "#endif" << NEW_LINE;

                TAB_N(indent); file << "metaData.ComponentTypeInfos = std::move(ECS::" << StateInfo::Instance().GetTypeString() << "::" << ComponentRegister::c_GetAllComponentTypeInfos << "());" << NEW_LINE;
                file << NEW_LINE;

                TAB_N(indent); file << "return metaData;" << NEW_LINE;
            }, 1);
            file << "#endif" << NEW_LINE;

            file << "}\n";

            return true;
        }
        else
        {
            return false;
        }
    }
}