#include "Core/MemoryTracker.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef IS_PLATFORM_WINDOWS
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable : 4091)
#include "DbgHelp.h"
#pragma comment(lib, "DbgHelp.lib")
#pragma warning(pop)
#endif

namespace Insight
{
	namespace Core
	{
		MemoryTracker::~MemoryTracker()
		{
			Destroy();
		}

		void MemoryTracker::Destroy()
		{
			if (m_allocations.size() > 0)
			{
				for (const auto& pair :m_allocations)
				{
					const MemoryTrackedAlloc& alloc = pair.second;
					std::cout << "Allocation leak: \n";
					std::cout << "\tPtr: " << alloc.Ptr << "\n";
					std::cout << "\tType: " << (int)alloc.Type << "\n";
					std::cout << "\tCallstack: \n";
					for (int i = c_CallStackCount - 1; i >= 0; --i)
					{
						const std::string& str = alloc.CallStack[i];
						if (str.empty())
						{
							continue;
						}

						std::cout << "\t\t" << str << "\n";
					}
				}
			}

			if (m_symInitialize)
			{
#ifdef IS_PLATFORM_WINDOWS
				SymCleanup(GetCurrentProcess());
#endif
			}
		}

		void MemoryTracker::Track(void* ptr, MemoryTrackAllocationType type)
		{
			auto itr = m_allocations.find(ptr);
			if (itr != m_allocations.end())
			{
				// Allocation already tracked.
				return;
			}
			m_allocations[ptr] = MemoryTrackedAlloc(ptr, type, GetCallStack());
		}

		void MemoryTracker::UnTrack(void* ptr)
		{
			auto itr = m_allocations.find(ptr);
			if (itr != m_allocations.end())
			{
				m_allocations.erase(itr);
			}
		}

		std::array<std::string, c_CallStackCount> MemoryTracker::GetCallStack()
		{
			std::array<std::string, c_CallStackCount> callStack;

#ifdef IS_PLATFORM_WINDOWS
			const ULONG framesToSkip = 0;
			const ULONG framesToCapture = c_CallStackCount;
			void* backTrace[framesToCapture]{};
			ULONG backTraceHash = 0;

			SYMBOL_INFO* symbol;
			HANDLE process;

			process = GetCurrentProcess();
			if (!m_symInitialize)
			{
				m_symInitialize = SymInitialize(process, NULL, TRUE);
			}

			const USHORT nFrame = CaptureStackBackTrace(
				framesToSkip
				, framesToCapture
				, backTrace
				, &backTraceHash
			);

			symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
			if (symbol)
			{
				symbol->MaxNameLen = 255;
				symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

				for (int i = 1; i < nFrame; ++i)
				{
					if (!SymFromAddr(process, (DWORD64)(backTrace[i]), 0, symbol))
						break;

					std::stringstream hexAddressStream;
					hexAddressStream << std::hex << symbol->Address;
					std::string hexAddress = hexAddressStream.str();
					std::transform(hexAddress.begin(), hexAddress.end(), hexAddress.begin(), [](const char c)
						{
							return std::toupper(c);
						});

					callStack[i - 1] = (std::to_string(nFrame - i - 1) +
						": " +
						std::string(symbol->Name) +
						" - 0x" +
						hexAddress);
				}
				free(symbol);
			}
#endif

			return callStack;
		}
	}
}