#pragma once

#include "Core/Defines.h"
#include "Core/TypeAlias.h"
#include "Core/Singleton.h"

#include <unordered_map>
#include <string>
#include <array>
#include <mutex>

namespace Insight
{
    namespace Core
    {
        class ConsoleSink;
        class ConsoleWindow;

        struct ConsoleValue
        {
            std::string Key;
            std::string Value;
        };

        template<typename T>
        constexpr bool ConsoleValuesSupported =
            std::is_same_v<bool, T>
            || std::is_same_v<u8, T>
            || std::is_same_v<u16, T>
            || std::is_same_v<u32, T>
            || std::is_same_v<u64, T>
            || std::is_same_v<i8, T>
            || std::is_same_v<i16, T>
            || std::is_same_v<i32, T>
            || std::is_same_v<i64, T>;

        template<typename T>
        class ConsoleValueAuto
        {
        public:
            static_assert(ConsoleValuesSupported<T>);

            ConsoleValueAuto(std::string key, T initalValue)
            {
                m_key = key;
                Console::SetValue(m_key, std::to_string(initalValue));
            }

            T GetValue() const
            {
                std::string value = Console::Instance().GetValue(m_key).Value;
                if (value.empty())
                {
                    return {};
                }

                if constexpr (std::is_integral_v<T> && std::is_signed_v<T>)
                {
                    const i64 typeValue = std::stoll(value);
                    return static_cast<T>(typeValue);
                }
                else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>)
                {
                    const u64 typeValue = std::stoull(value);
                    return static_cast<T>(typeValue);
                }
            }

        private:
            std::string m_key;
        };

        class IS_CORE Console : public Singleton<Console>
        {
            struct ConsoleMessageColour
            {
                union
                {
                    float Colour[4];
                    struct
                    {
                        float r; 
                        float g;
                        float b;
                        float a;
                    };
                };
            };
            struct ConsoleMessage
            {
                std::string Message;
                ConsoleMessageColour Colour;
            };

        public:
            Console();
            ~Console();

            static void SetValue(const std::string& key, const std::string& value);
            const ConsoleValue& GetValue(const std::string& key) const;

            void Show(const bool shouldShow);
            bool IsShowing() const;

            void Render(const u32 x, const u32 y, const u32 width, const u32 height);

        private:
            void Message(const std::string_view message);
            void Message(const std::string_view message, const ConsoleMessageColour colour);

        private:
            static std::unordered_map<std::string, ConsoleValue> m_values;
            ConsoleValue m_emptyConsoleValue;

            const static u8 c_ConsoleMessageSize = 64;
            // Keep the last 64 console messages.
            std::array<ConsoleMessage, c_ConsoleMessageSize> m_consoleMessages;
            std::mutex m_messagesMutex;
            u8 m_consoleMessageLastIndex = 0;

            bool m_isShowing = false;


            friend ConsoleSink;
            friend ConsoleWindow;
        };
    }
}