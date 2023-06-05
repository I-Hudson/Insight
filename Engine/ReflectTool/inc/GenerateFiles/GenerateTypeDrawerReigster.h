#pragma once

#include "Reflect.h"

namespace InsightReflectTool
{
    class GenerateTypeDrawerReigster
    {
    public:
        bool Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
    };
}