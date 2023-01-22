#pragma once

#include <string>
#include <sstream>

namespace Insight
{
    namespace Serialisation
    {
        template<typename T>
        struct PropertySerialiser
        {
            std::string operator()(T const& object)
            {
                std::stringstream ss;
                ss << object;
                if (ss.fail())
                {
                    return "";
                }
                return ss.str();
            }
        };

        template<typename T>
        struct PropertyDeserialiser
        {
            T operator()(std::string const& data)
            {
                return {};
            }
        };
    }
}