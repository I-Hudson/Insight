#include "Core/CommandLineArgs.h"
#include "Core/Logger.h"

#include "Core/StringUtils.h"

namespace Insight
{
	namespace Core
	{
		std::unordered_map<std::string, CommandLineValue> CommandLineArgs::m_args;
		CommandLineValue CommandLineArgs::s_default_command_line_value;

		CommandLineValue::CommandLineValue(std::string key, std::string value)
			: Key(std::move(key))
			, Value(std::move(value))
		{ }

		std::string CommandLineValue::GetString() const
		{
			return Value;
		}

		i32 CommandLineValue::GetI32() const
		{
			return std::stoi(Value);
		}

		u32 CommandLineValue::GetU32() const
		{
			return std::stoul(Value);
		}

		bool CommandLineValue::GetBool() const
		{
			// Bool should be defined as 0/1 not true and false as it works better in code.
			return std::stoi(Value);
		}

		void CommandLineArgs::ParseCommandLine(int argc, char** argv)
		{
			// Start at one as the first command line args is the path to the .exe
			for (int i = 1; i < argc; ++i)
			{
				std::string arg = argv[i];
				const u64 equal_index = arg.find('=');

				std::string key = equal_index != std::string::npos ? arg.substr(0, equal_index) : "";
				std::string value = equal_index != std::string::npos ? arg.substr(arg.find('=') + 1) : "";

				ToLowwer(key);
				ToLowwer(value);

				if (key.empty() || value.empty())
				{
					IS_CORE_WARN("[CommandLineArgs::ParseCommandLine] Key '{}' and/or value '{}' is empty.", key, value);
					continue;
				}

				if (m_args.find(key) != m_args.end())
				{
					IS_CORE_WARN("[CommandLineArgs::ParseCommandLine] Key '{}' already defined with value '{}'. Second definition ignored.", key, value);
					continue;
				}

				if (value == "true")
				{
					value = "1";
				}
				else if (value == "false")
				{
					value = "0";
				}

				m_args[key] = CommandLineValue(key, value);
			}
		}

		Ptr<CommandLineValue> CommandLineArgs::GetCommandLineValue(std::string key)
		{
			ToLowwer(key);
			if (auto itr = m_args.find(key); itr != m_args.end())
			{
				return &const_cast<CommandLineValue&>(itr->second);
			}
			return &s_default_command_line_value;
		}
	}
}