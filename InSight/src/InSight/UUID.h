#pragma once

#pragma comment(lib, "rpcrt4.lib")  // UuidCreate - Minimum supported OS Win 2000
#include <windows.h>
#include <random>
#include <sstream>

namespace Insight
{
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);


    class IS_API UUID
    {
    public:
        UUID() : m_uuid(GenUUID())
        { }
        const std::string& GetUUID() const { return m_uuid; }

    private:
        static std::string GenUUID()
        {
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i = 0; i < 8; i++) {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 4; i++) {
                ss << dis(gen);
            }
            ss << "-4";
            for (i = 0; i < 3; i++) {
                ss << dis(gen);
            }
            ss << "-";
            ss << dis2(gen);
            for (i = 0; i < 3; i++) {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 12; i++) {
                ss << dis(gen);
            };
            return ss.str();
        }

    private:
        std::string m_uuid;
    };
}