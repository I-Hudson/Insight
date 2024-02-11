#include "Core/Console.h"

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
    }
}