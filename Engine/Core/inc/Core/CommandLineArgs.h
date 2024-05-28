#pragma once

#include "Core/Defines.h"
#include "Core/Memory.h"

#include <unordered_map>
#include <string>

namespace Insight
{
	namespace Core
	{
		/// @brief Define a single command line value. This is stored as a stirng, then parsed to other value types.
		struct IS_CORE CommandLineValue
		{
			CommandLineValue() = default;
			CommandLineValue(std::string key, std::string value);

			/// @brief Return the command line value as a string.
			/// @return std::string
			std::string GetString() const;
			/// @brief Return the command line value as a i32.
			/// @return i32
			i32 GetI32() const;
			/// @brief Return the command line value as a u32.
			/// @return u32 
			u32 GetU32() const;
			/// @brief Return the command line value as a bool.
			/// @return bool
			bool GetBool() const;

			std::string Key = "";
			std::string Value = "";
		};

		/// @brief Helper class for parsing the command line args.
		class IS_CORE CommandLineArgs
		{
		public:
			static void ParseCommandLine(int argc, char** argv);
			static void ParseCommandLine(const char* file_path);
			static void AddCommandLine(const char* key, const char* value);

			static std::string GetAllCommandLine();
			static Ptr<CommandLineValue> GetCommandLineValue(std::string key);
			static bool CommandListExists(std::string const& key);

		private:
			static std::unordered_map<std::string, CommandLineValue> m_args;
			static CommandLineValue s_default_command_line_value;
		};
	}
}