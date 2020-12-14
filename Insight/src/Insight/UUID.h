#pragma once

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <rpc.h>
#include <windows.h>
#include <random>
#include <sstream>

#include "Insight/Core.h"

namespace Insight
{
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    namespace Serialization
    {
        class Serializable;
    }

    class UUID
    {
    public:
        UUID() : m_uuid(GenUUID())
        { }
        const std::string& GetUUID() const { return m_uuid; }

        static std::string GenUUID()
        {
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i = 0; i < 8; ++i)
            {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 4; ++i)
            {
                ss << dis(gen);
            }
            ss << "-4";
            for (i = 0; i < 3; ++i)
            {
                ss << dis(gen);
            }
            ss << "-";
            ss << dis2(gen);
            for (i = 0; i < 3; ++i)
            {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 12; ++i)
            {
                ss << dis(gen);
            };
            return ss.str();
        }

        static U128 GenUUID_U128()
        {
            U128 uuid;
            UuidCreate(&uuid);
            return uuid;
        }

    protected:
        void SetUUID(const std::string uuid) { m_uuid = uuid; }

    private:
        std::string m_uuid;

        friend class Insight::Serialization::Serializable;
    };
}