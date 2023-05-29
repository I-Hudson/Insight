#pragma once

#ifdef IS_PLATFORM_WINDOWS

#include "Core/TypeAlias.h"
#include "Core/Defines.h"
#include "Core/GUID.h"

#include "Core/CPUInformation.h"

#include <string>
#include <vector>

namespace Insight
{
	namespace Windows
	{
		class IS_CORE PlatformWindows
		{
		public:

			static void Initialise();
			static void Shutdown();

			static void MemCopy(void* dst, void const* src, u64 size);
			static void MemSet(void* dst, int value, u64 size);
			static void MemClear(void* dst, u64 size);
			static bool MemCompare(const void* buf1, const void* buf2, u64 size);

			static bool StrCompare(const char* str1, const char* str2);
			static bool WStrCompare(const wchar_t* wstr1, const wchar_t* wstr2);

			static std::vector<std::string> GetCallStack(int frameCount = 32);

			static std::string StringFromWString(const std::wstring& wstring);
			static std::wstring WStringFromString(const std::string& string);
			static std::wstring WStringFromStringView(std::string_view string);
			
			static Core::GUID CreateGUID();
			static void AssignGUID(Core::GUID& guid);

			static bool IsMainThread();
			static bool IsDebuggerAttached();

			static Core::CPUInformation GetCPUInformation();
			static Core::MemoryInformation GetMemoryInformation();

			static void* LoadDynamicLibrary(std::string_view path);
			static void FreeDynamicLibrary(void*& library);

			/// @brief Load an exported function from a dynamic shared library and reinterpret cast to the given template arguments.
			/// @tparam ReturnT 
			/// @tparam ...Args 
			/// @param library 
			/// @param functionName 
			/// @return 
			template<typename ReturnT, typename... Args>
			static ReturnT(*GetDynamicFunction(void* library, std::string_view functionName))(Args...)
			{
				if (library == nullptr || functionName.empty())
				{
					return nullptr;
				}

				using ReturnFuncSig = ReturnT(*)(Args...);
				void* procAddress = GetProcAddress((HMODULE)library, functionName.data());
				ReturnFuncSig func = reinterpret_cast<ReturnFuncSig>(procAddress);
				if (func == nullptr)
				{
					return nullptr;
				}
				return func;
			}

		private:
			static Core::MemoryInformation s_memoryInformation;
			static Core::CPUInformation s_cpuInformation;

			static u32 s_mainThreadId;
			static unsigned long s_processId;
			static void* s_memoryReadProcessHandle;
		};

	}
}
#endif /// IS_PLATFORM_WINDOWS