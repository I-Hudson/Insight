#pragma once

#include <Simplygon.h>

namespace Insight
{
    class SimplygonWrapper
    {
    public:

        static bool Initialise();
        static void Shutdown();

    private:
        static Simplygon::ISimplygon* s_instance;
    };
}