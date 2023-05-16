#pragma once

#include <Reflect.h>


Reflect::FileParser ParseFilesInDirectory(std::string_view directory, const Reflect::ReflectAddtionalOptions& options)
{
    Reflect::FileParser parser;
    parser.SetIgnoreStrings(
        {
            "IS_CORE",
            "IS_MATHS",
            "IS_GRAPHICS",
            "IS_RUNTIME",
            "IS_EDITOR",
        });
    parser.ParseDirectory(directory.data(), &options);
    Reflect::CodeGenerate codeGenerate;
    codeGenerate.Reflect(parser, &options);
    return parser;
}