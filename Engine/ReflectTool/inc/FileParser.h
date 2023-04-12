#pragma once

#include <Reflect.h>


Reflect::FileParser ParseFilesInDirectory(std::string_view directory)
{
    Reflect::FileParser parser;
    Reflect::ReflectAddtionalOptions options = { };
    parser.ParseDirectory(directory.data(), &options);
    Reflect::CodeGenerate codeGenerate;
    codeGenerate.Reflect(parser, &options);
    return parser;
}