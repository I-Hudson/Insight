#pragma once

#include "Core/Defines.h"

#include <string_view>

#include <Simplygon.h>

namespace Insight
{
    enum SimplygonStages
    {
        Reduction = BIT(0),
    };

    class SimplygonWrapper
    {
    public:

        static bool Initialise();
        static void Shutdown();

        static std::string Run(std::string_view path, SimplygonStages stagesToRun);

    private:
        static void RunReduction(Simplygon::spScene& scene);

        static Simplygon::spScene LoadScene(std::string_view path);
        static std::string SaveScene(Simplygon::spScene& scene, std::string path);
        
        static std::string GetOptimisedFilePath(std::string_view path);
        static bool CheckForOptimisedFile(std::string_view path);
        static void CheckLog(Simplygon::ISimplygon* sg);

    private:
        static Simplygon::ISimplygon* s_instance;
    };
}