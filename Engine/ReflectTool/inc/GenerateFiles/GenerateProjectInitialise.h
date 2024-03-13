#pragma once

#include "Reflect.h"

namespace InsightReflectTool
{
    class GenerateProjectInitialise
    {
    public:
        bool GenerateHeader(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
        bool GenerateSource(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
    };
}