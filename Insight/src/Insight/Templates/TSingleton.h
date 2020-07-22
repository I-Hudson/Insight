#pragma once

namespace Insight
{
    template<typename T>
    class TSingleton
    {
    public:
        virtual ~T() { }

        static T& Instance()
        {
            T instnace;
            return instnace;
        }
    };
}