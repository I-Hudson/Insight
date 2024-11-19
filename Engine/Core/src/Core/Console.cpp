#include "Core/Console.h"

#include <imgui.h>

namespace Insight
{
    namespace Core
    {
        std::unordered_map<std::string, ConsoleValue> Console::m_values;

        Console::Console()
        { }

        Console::~Console()
        { }

        void Console::SetValue(const std::string& key, const std::string& value)
        {
            m_values[key] = ConsoleValue{ key, value };
        }

        const ConsoleValue& Console::GetValue(const std::string& key) const
        {
            if (auto iter = m_values.find(key);
                iter != m_values.end())
            {
                return iter->second;
            }
            return m_emptyConsoleValue;
        }

        void Console::Message(const std::string_view message)
        {
            ConsoleMessageColour colour;
            colour.r = 1;
            colour.g = 1;
            colour.b = 1;
            colour.a = 1;
            Message(message, colour);
        }

        void Console::Message(const std::string_view message, const ConsoleMessageColour colour)
        {
            ConsoleMessage consoleMessage;
            consoleMessage.Message = message;
            consoleMessage.Colour = colour;

            m_consoleMessages[m_consoleMessageLastIndex] = consoleMessage;

            // Wrap the start index back to 0 if required otherwise just increment by one.
            m_consoleMessageLastIndex = (m_consoleMessageLastIndex + 1) % c_ConsoleMessageSize;
        }

        void Console::Show(const bool shouldShow)
        {
            m_isShowing = shouldShow;
        }

        bool Console::IsShowing() const
        {
            return m_isShowing;
        }

        void Console::Render()
        {
#ifndef IMGUI_DISABLE
            if (!m_isShowing)
            {
                return;
            }


#endif
        }
    }
}