#pragma once

namespace Insight
{
    template<typename T>
    class TSingleton
    {
    public:
        static T& Instance()
        {
            static T instnace;
            return instnace;
        }
    };
}