#pragma once

#include "Reflect/Reflect.h"
#include "Reflect/Core/Options.h"
#include "Reflect/FileParser/FileParser.h"

namespace InsightReflectTool
{
    class GenerateComponentRegister
    {
    public:
        bool Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;

    private:
        void WriteGetTypeInfos(std::fstream& file, const std::vector<Reflect::Parser::ReflectContainerData>& componentClasses) const;
    };
}