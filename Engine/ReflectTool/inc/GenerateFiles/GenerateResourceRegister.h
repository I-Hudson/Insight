#pragma once

#include "Reflect/Reflect.h"

namespace InsightReflectTool
{
    class GenerateResourceRegister
    {
    public:
        bool Generate(const Reflect::Parser::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
    };
}