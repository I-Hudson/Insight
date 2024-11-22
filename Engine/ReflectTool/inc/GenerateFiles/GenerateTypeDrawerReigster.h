#pragma once

#include "Reflect/Reflect.h"
#include "Reflect/Core/Options.h"
#include "Reflect/FileParser/FileParser.h"

namespace InsightReflectTool
{
    class GenerateTypeDrawerReigster
    {
    public:
        bool Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
    };
}