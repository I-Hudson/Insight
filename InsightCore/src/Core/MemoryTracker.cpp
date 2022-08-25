#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "Platform/Platform.h"

#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef IS_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
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
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_lock);

			if (m_allocations.size() > 0)
			{
				for (const auto& pair :m_allocations)
				{
					const MemoryTrackedAlloc& alloc = pair.second;
					IS_CORE_ERROR("Allocation leak:");
					IS_CORE_ERROR("\tPtr: {}", alloc.Ptr);
					IS_CORE_ERROR("\tType: {}", (int)alloc.Type);
					IS_CORE_ERROR("\tCallstack: ");
					for (int i = c_CallStackCount - 1; i >= 0; --i)
					{
						const std::string& str = alloc.CallStack[i];
						if (str.empty())
						{
							continue;
						}

						IS_CORE_ERROR("\t\t{}", str);
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
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_lock);

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
			IS_PROFILE_FUNCTION();
			std::lock_guard lock(m_lock);

			auto itr = m_allocations.find(ptr);
			if (itr != m_allocations.end())
			{
				m_allocations.erase(itr);
			}
		}

#define MEMORY_TRACK_CALLSTACK
		std::array<std::string, c_CallStackCount> MemoryTracker::GetCallStack()
		{
			IS_PROFILE_FUNCTION();
			//std::vector<std::string> callStackVector = Platform::GetCallStack(c_CallStackCount);
			std::array<std::string, c_CallStackCount> callStack;

			//for (size_t i = 0; i < c_CallStackCount; ++i)
			//{
			//	if (i < callStackVector.size())
			//	{
			//		callStack[i] = std::move(callStackVector[i]);
			//	}
			//}

			//return callStack;

			// TOOD: Think of a better way to have this supported. Would be nice to have this. Maybe a call stack should only be gotten
			// if there is a crash? Look at third party options for getting the callstack. Disabled for non debug due to performance.
#if defined(IS_PLATFORM_WINDOWS) && defined(MEMORY_TRACK_CALLSTACK) && defined(_DEBUG)
			
			const ULONG framesToSkip = 0;
			const ULONG framesToCapture = c_CallStackCount;
			void* backTrace[framesToCapture]{};
			ULONG backTraceHash = 0;

			SYMBOL_INFO* symbol = nullptr;
			static HANDLE process = nullptr;
			if (!process)
			{
				process = GetCurrentProcess();
			}

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