#include "Core/MemoryTracker.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"

#include "Platforms/Platform.h"

#include "Core/Profiler.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <charconv>


#ifdef IS_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             /// Exclude rarely-used stuff from Windows headers.
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

        void MemoryTracker::Initialise()
        {
#ifdef IS_MEMORY_TRACKING
            std::lock_guard lock(m_lock);
            m_isReady = true;
            for (size_t i = 0; i < static_cast<u64>(MemoryAllocCategory::Size); ++i)
            {
                m_categoryAllocationSizeBytes[i] = 0;
                m_categoryAllocationCount[i] = 0;
            }
#endif
        }

        void MemoryTracker::Destroy()
        {
            IS_PROFILE_FUNCTION();

#ifdef IS_MEMORY_TRACKING
            std::lock_guard allocationLock(m_lock);
            std::lock_guard allocationToNameLock(m_allocationToNameLock);

            if (!m_isReady)
            {
                return;
            }
            m_isReady = false;

            if (m_allocations.size() > 0)
            {
                for (const auto& pair : m_allocations)
                {
                    const MemoryTrackedAlloc& alloc = pair.second;
                    IS_CORE_ERROR("Allocation leak:");
                    IS_CORE_ERROR("\tPtr: {}", alloc.Ptr);
                    IS_CORE_ERROR("\tName: {}", m_allocationToName[alloc.Ptr]);
                    IS_CORE_ERROR("\tSize: {}", alloc.Size);
                    IS_CORE_ERROR("\tType: {}", (int)alloc.Type);
                    IS_CORE_ERROR("\tCallstack: ");
                    for (int i = c_CallStackCount - 1; i >= 0; --i)
                    {
                        const std::string& str = alloc.CallStack[i];
                        if (str.empty() || str == "\0")
                        {
                            continue;
                        }

                        IS_CORE_ERROR("\t\t{}", str);
                    }
                }
            }
            m_allocations = {};

            const char* TotalAllocatedBytesText = "Total allocated bytes: '{}'";
            if (m_totalAllocatedInBytes > 0)
            {
                IS_CORE_ERROR(TotalAllocatedBytesText, m_totalAllocatedInBytes);
            }
            else
            {
                IS_CORE_INFO(TotalAllocatedBytesText, m_totalAllocatedInBytes);
            }

            m_totalAllocatedInBytes = 0;
            for (size_t i = 0; i < static_cast<u64>(MemoryAllocCategory::Size); ++i)
            {
                m_categoryAllocationSizeBytes[i] = 0;
                m_categoryAllocationCount[i] = 0;
            }

            if (m_symInitialize)
            {
#ifdef IS_PLATFORM_WINDOWS
                SymCleanup(GetCurrentProcess());
#endif
            }
#endif // IS_MEMORY_TRACKING
        }

        void MemoryTracker::Track(void* ptr, u64 size, MemoryTrackAllocationType type)
        {
            Track(ptr, size, MemoryAllocCategory::General, type);
        }

        void MemoryTracker::Track(void* ptr, u64 size, MemoryAllocCategory category, MemoryTrackAllocationType type)
        {
            IS_PROFILE_FUNCTION();

#ifdef IS_MEMORY_TRACKING
            std::unique_lock lock(m_lock);

            if (!m_isReady)
            {
                return;
            }

            auto itr = m_allocations.find(ptr);
            //ASSERT(itr == m_allocations.end());
            if (itr == m_allocations.end())
            {
                lock.unlock();
                MemoryTrackedAlloc memoryTrackedAlloc(ptr, size, category, type, GetCallStack());

                lock.lock();
                m_allocations[ptr] = memoryTrackedAlloc;
                
                m_totalAllocatedInBytes += size;
                m_categoryAllocationSizeBytes.at(static_cast<u64>(category)) += size;
                ++m_categoryAllocationCount.at(static_cast<u64>(category));

                lock.unlock();
#ifdef IS_PROFILE_TRACY
                TracyAlloc(ptr, size);
#endif
            }
            else
            {
                lock.unlock();
            }
#endif // IS_MEMORY_TRACKING
        }

        void MemoryTracker::UnTrack(void* ptr)
        {
            IS_PROFILE_FUNCTION();

#ifdef IS_MEMORY_TRACKING
            std::lock_guard lock(m_lock);

            if (!m_isReady)
            {
                return;
            }

            auto itr = m_allocations.find(ptr);
            //ASSERT(itr != m_allocations.end());
            if (itr != m_allocations.end())
            {
                m_categoryAllocationSizeBytes.at(static_cast<u64>(itr->second.Category)) -= itr->second.Size;
                --m_categoryAllocationCount.at(static_cast<u64>(itr->second.Category));
                m_totalAllocatedInBytes -= itr->second.Size;

                m_allocations.erase(itr);
#ifdef IS_PROFILE_TRACY
                TracyFree(ptr);
#endif
            }
#endif
        }

        void MemoryTracker::NameAllocation(void* ptr, const char* name)
        {
            std::lock_guard allocationLock(m_lock);
            std::lock_guard allocationToNameLock(m_allocationToNameLock);
            if (auto iter = m_allocations.find(ptr);
                iter != m_allocations.end())
            {
                m_allocationToName[ptr] = name;
            }
        }

        u64 MemoryTracker::GetUsage(MemoryAllocCategory category) const
        {
            if (category == MemoryAllocCategory::Size)
            {
                return 0;
            }

            std::lock_guard lock(m_lock);
            return m_categoryAllocationSizeBytes.at(static_cast<u64>(category));
        }

        u64 MemoryTracker::GetTotalNumberOfAllocationsForCategory(MemoryAllocCategory category) const
        {
            if (category == MemoryAllocCategory::Size)
            {
                return 0;
            }
            std::lock_guard lock(m_lock);
            return m_categoryAllocationCount.at(static_cast<u64>(category));
        }

        u64 MemoryTracker::GetTotalNumberOfAllocations() const
        {
            std::lock_guard lock(m_lock);
            return m_allocations.size();
        }

        u64 MemoryTracker::GetTotalAllocatedInBytes() const
        {
            std::lock_guard lock(m_lock);
            return m_totalAllocatedInBytes;
        }

#define MEMORY_TRACK_CALLSTACK
        std::array<char[c_CallstackStringSize], c_CallStackCount> MemoryTracker::GetCallStack()
        {
            IS_PROFILE_FUNCTION();
            ///std::vector<std::string> callStackVector = Platform::GetCallStack(c_CallStackCount);
            std::array<char[c_CallstackStringSize], c_CallStackCount> callStack;
            for (size_t i = 0; i < c_CallStackCount; ++i)
            {
                callStack[i][0] = '\0';
            }

            std::unique_lock lock(m_lock);
            if (!m_isReady)
            {
                lock.unlock();
                return callStack;
            }
            lock.unlock();

#ifdef IS_MEMORY_TRACKING
            /// TOOD: Think of a better way to have this supported. Would be nice to have this. Maybe a call stack should only be gotten
            /// if there is a crash? Look at third party options for getting the callstack. Disabled for non debug due to performance.
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

                    //std::stringstream hexAddressStream;
                    //hexAddressStream << std::hex << symbol->Address;
                    //std::string hexAddress = hexAddressStream.str();
                    //std::transform(hexAddress.begin(), hexAddress.end(), hexAddress.begin(), [](const char c)
                    //    {
                    //        return std::toupper(c);
                    //    });

#define ADVANCE_CHAR(cPtr, lPtr) if (cPtr == lPtr) { break; } else { ++cPtr; } 

                    char* firstChar = callStack[i - 1];
                    char* lastChar = callStack[i - 1] + (c_CallstackStringSize - 1ull);
                    char* currentChar = callStack[i - 1];

                    currentChar = std::to_chars(currentChar, lastChar, nFrame - i - 1).ptr;
                    *currentChar = ':';
                    ADVANCE_CHAR(currentChar, lastChar);
                    *currentChar = ' ';
                    ADVANCE_CHAR(currentChar, lastChar);
                    
                    u64 strlen = strnlen_s(symbol->Name, lastChar - currentChar);
                    Platform::MemCopy(currentChar, symbol->Name, strlen);
                    currentChar += strlen;

                    *currentChar = ' ';
                    ADVANCE_CHAR(currentChar, lastChar);
                    *currentChar = '-';
                    ADVANCE_CHAR(currentChar, lastChar);
                    *currentChar = ' ';
                    ADVANCE_CHAR(currentChar, lastChar);

                    currentChar = std::to_chars(currentChar, lastChar, symbol->Address).ptr;

                    if (currentChar < lastChar)
                    {
                        *currentChar = '\0';
                    }
                    else
                    {
                        lastChar = '\0';
                    }

                    //callStack[i - 1] = (std::to_string(nFrame - i - 1) +
                    //    ": " +
                    //    std::string(symbol->Name) +
                    //    " - 0x" +
                    //    hexAddress);
#undef OUT_OF_BOUNDS_CHECK
                }
                free(symbol);
            }
#endif
#endif // IS_MEMORY_TRACKING
            return callStack;
        }
    }
}