#include "ispch.h"
#include "Engine/Memory/ProfilerMemory.h"
#include "Engine/Platform/Platform.h"

#ifdef IS_DEBUG

#ifdef IS_PLATFORM_WINDOWS
#pragma warning(push)
#pragma warning(disable : 4091)
#include "DbgHelp.h"
#pragma comment(lib, "DbgHelp.lib")
#pragma warning(pop)
#endif

ProfilerMemory::~ProfilerMemory()
{
	m_locker.Lock();
	if (m_currentAllocations.size() != 0)
	{
		m_locker.Unlock();
		DumpAllocations();
		ASSERT(false);
	}
	m_locker.Unlock();
}

void ProfilerMemory::TrackAllocation(void* ptr, U64 size)
{
	m_locker.Lock();

	//ASSERT(!ContainsAllocation(ptr, m_currentAllocations) && !ContainsAllocation(ptr, m_allAllocations));

	ProfilerMemoryAdditionalInfo info =
	{
		"",
		size,
		GetCallStack()
	};

	m_currentAllocations[ptr] = info;
	m_allAllocations[ptr] = info;

	m_locker.Unlock();
}

void ProfilerMemory::UnTrackAllocation(void* ptr)
{
	m_locker.Lock();

	//ASSERT(ContainsAllocation(ptr, m_currentAllocations));
	auto currentIt = GetIterator(ptr, m_currentAllocations);
	if (currentIt != m_currentAllocations.end())
		m_currentAllocations.erase(currentIt);

	m_locker.Unlock();
}

U64 ProfilerMemory::GetCurrentAllocatedSize()
{
	m_locker.Lock();
	U64 currentAllocationSize = 0;
	for (auto&  alloc : m_currentAllocations)
	{
		currentAllocationSize += alloc.second.Size;
	}
	m_locker.Unlock();

	return currentAllocationSize;
}

void ProfilerMemory::DumpAllocations()
{
	m_locker.Lock();
	for (auto& alloc : m_currentAllocations)
	{
		std::string callStack;
		for (auto& callLine : alloc.second.CallStack)
		{
			callStack += "\n\t\t";
			callStack += callLine;
		}

		IS_INFO("Allocation: \n\t Ptr: {0} \n\t Name:{1} \n\t Size: {2} \n\t CallStack: {3}", alloc.first, alloc.second.Name, alloc.second.Size, callStack);
	}
	m_locker.Unlock();
}

void ProfilerMemory::AddName(void* ptr, const std::string& name)
{
	auto it = GetIterator(ptr, m_currentAllocations);
	if (it != m_currentAllocations.end())
	{
		(*it).second.Name = name;
	}
}

bool ProfilerMemory::ContainsAllocation(void* ptr, ProProfilerMemoryCollection& collection)
{
	for (auto& alloc : collection)
	{
		U64* c1 = (U64*)ptr;
		U64* c2 = (U64*)alloc.first;

		if (c1 == c2)
		{
			return true;
		}
	}
	return false;
}

ProProfilerMemoryCollection::iterator ProfilerMemory::GetIterator(void* ptr, ProProfilerMemoryCollection& collection)
{
	for (auto it = collection.begin(); it != collection.end(); ++it)
	{
		if (it->first == ptr)
		{
			return it;
		}
	}
	return collection.end();
}

std::vector<std::string> ProfilerMemory::GetCallStack()
{
#ifdef IS_PLATFORM_WINDOWS
	const ULONG framesToSkip = 0;
	const ULONG framesToCapture = 64;
	void* backTrace[framesToCapture]{};
	ULONG backTraceHash = 0;

	SYMBOL_INFO* symbol;
	HANDLE process;

	process = GetCurrentProcess();
	SymInitialize(process, NULL, TRUE);

	const USHORT nFrame = CaptureStackBackTrace(
		framesToSkip
		, framesToCapture
		, backTrace
		, &backTraceHash
	);

	symbol = (SYMBOL_INFO*)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
	symbol->MaxNameLen = 255;
	symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

	std::vector<std::string> callStack;
	for (int i = 1; i < nFrame; ++i)
	{
		SymFromAddr(process, (DWORD64)(backTrace[i]), 0, symbol);

		std::stringstream hexAddressStream;
		hexAddressStream << std::hex << symbol->Address;
		std::string hexAddress = hexAddressStream.str();
		std::transform(hexAddress.begin(), hexAddress.end(), hexAddress.begin(), [](const char c)
			{
				return std::toupper(c);
			});

		callStack.push_back(std::to_string(nFrame - i - 1) +
			": " +
			std::string(symbol->Name) +
			" - 0x" +
			hexAddress);
	}
	free(symbol);
#endif

	return callStack;
}
#endif