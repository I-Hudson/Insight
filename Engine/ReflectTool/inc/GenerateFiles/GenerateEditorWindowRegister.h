#pragma once

#include "Reflect.h"

namespace InsightReflectTool
{
    class GenerateEditorWindowRegister
    {
    public:
        bool Generate(const Reflect::FileParser& fileParser, std::string_view outFilePath, const Reflect::ReflectAddtionalOptions& options) const;
    };
}