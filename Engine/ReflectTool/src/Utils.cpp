#include "Utils.h"

#include <filesystem>
#include <Reflect/Reflect.h>
#include <Reflect/Core/Options.h>
#include <Reflect/FileParser/FileParser.h>
#include <Reflect/CodeGenerate/CodeGenerate.h>

namespace InsightReflectTool
{
    namespace Utils
    {
        StateInfo State;

        void WriteGeneratedFileHeader(std::fstream& file)
        {
            if (file.is_open())
            {
                file << "// THIS FILE IS AUTO GENERATED BY THE INSIGHT REFLECT TOOL. ANY MODIFICATIONS WILL BE OVERWRITTEN." << "\n";
                file << "#pragma once" << "\n" << "\n";
            }
        }

        void WriteIncludeFile(std::fstream& file, std::string_view includeFile)
        {
            file << "#include \"";
            file << includeFile;
            file << "\"\n";
        }

        void WriteIncludeFiles(std::fstream& file, std::string_view fileOutputAbsPath, const std::vector<std::string>& filesToInclude)
        {
            for (const std::string& filePath : filesToInclude)
            {
                std::string includePath = std::filesystem::canonical(filePath).generic_string();
                std::string relativeIncludePath = std::filesystem::relative(includePath, fileOutputAbsPath).generic_string();
                WriteIncludeFile(file, relativeIncludePath);
            }
        }

        void WriteIncludeLibraryFile(std::fstream& file, std::string_view includeFile)
        {
            file << "#include <";
            file << includeFile;
            file << ">\n";
        }

        void WriteHeaderFunctionDeclaration(std::fstream& file, std::string_view returnValue, std::string_view functionName, std::vector<std::string_view> arguments, int indent)
        {
        }

        void WriteSourceFunctionDefinition(std::fstream& file, std::string_view returnValue, std::string_view functionName, std::vector<std::string_view> arguments, WriteSourceFunctionBodyFunc func, int indent)
        {
            TAB_N(indent);
            file << returnValue << " " << functionName << "(";
            for (const std::string_view& view : arguments)
            {
                file << view;
                if (view != arguments.back())
                {
                    file << ", ";
                }
            }
            file << ")" << NEW_LINE;

            TAB_N(indent);
            file << "{" << NEW_LINE;

            func(file);

            TAB_N(indent);
            file << "}" << NEW_LINE << NEW_LINE;
        }

        void ValidateOutputPath(std::string_view path)
        {
            std::string folderPath = std::filesystem::path(path).parent_path().string();
            if (!std::filesystem::exists(folderPath))
            {
                std::filesystem::create_directories(folderPath);
            }
        }

        std::vector<std::string> GetAllFilesWithType(std::string_view type, const Reflect::Parser::FileParser& fileParser)
        {
            std::vector<std::string> files;
            for (const auto& fileParsed : fileParser.GetAllFileParsedData())
            {
                if (fileParsed.parserOptions.DoNotReflect)
                {
                    continue;
                }

                for (const auto& reflectData : fileParsed.ReflectData)
                {
                    if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [&type](const Reflect::Parser::ReflectInheritanceData& data)
                        {
                            return type == data.Name;
                        }) != reflectData.Inheritance.end())
                    {
                        files.push_back(fileParsed.FilePath + "/" + fileParsed.FileName + ".h");
                    }
                }
            }
            return files;
        }

        std::vector<std::string> GetAllFilesWithTypeRecersive(const std::string_view type, const Reflect::Parser::FileParser& fileParser)
        {
            std::vector<std::string> files;
            for (const auto& fileParsed : fileParser.GetAllFileParsedData())
            {
                if (fileParsed.parserOptions.DoNotReflect)
                {
                    continue;
                }

                for (const auto& reflectData : fileParsed.ReflectData)
                {
                    if (CheckFileInheritsType(type, reflectData.Inheritance))
                    {
                        files.push_back(fileParsed.FilePath + "/" + fileParsed.FileName + ".h");
                    }
                }
            }
            return files;
        }

        bool CheckFileInheritsType(const std::string_view type, const std::vector<Reflect::Parser::ReflectInheritanceData>& inheritanceData)
        {
            bool result = std::find_if(inheritanceData.begin(), inheritanceData.end(),
                [type](const Reflect::Parser::ReflectInheritanceData& a)
                {
                    return type == a.Name;
                }) != inheritanceData.end();

            if (result)
            {
                return true;
            }

            for (size_t i = 0; i < inheritanceData.size(); ++i)
            {
                if (CheckFileInheritsType(type, inheritanceData[i].Inheritances))
                {
                    return true;
                }
            }

            return false;
        }

        std::vector<Reflect::Parser::ReflectContainerData> GetAllDerivedTypesFromBaseType(std::string_view baseType, const Reflect::Parser::FileParser& fileParser)
        {
            std::vector<Reflect::Parser::ReflectContainerData> classes;
            for (const auto& fileParsed : fileParser.GetAllFileParsedData())
            {
                if (fileParsed.parserOptions.DoNotReflect)
                {
                    continue;
                }

                for (const auto& reflectData : fileParsed.ReflectData)
                {
                    if (std::find_if(reflectData.Inheritance.begin(), reflectData.Inheritance.end(), [&baseType, &fileParser](const Reflect::Parser::ReflectInheritanceData& data)
                        {
                            if (data.Name == baseType)
                            {
                                return true;
                            }

                            return data.InheritsFromType(baseType);
                        }) != reflectData.Inheritance.end())
                    {
                        classes.push_back(reflectData);
                    }
                }
            }
            return classes;
        }
    }
}