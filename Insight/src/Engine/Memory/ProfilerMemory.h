#pragma once

#include "Engine/Templates/TSingleton.h"
#include "Engine/Platform/CriticalSection.h"

struct ProfilerMemoryAdditionalInfo
{
	std::string Name;
	U64 Size;
};

using ProProfilerMemoryCollection = std::unordered_map<void*, ProfilerMemoryAdditionalInfo>;

class ProfilerMemory : public TSingleton<ProfilerMemory>
{
public:
	~ProfilerMemory();

	void TrackAllocation(void* ptr, U64 size);
	void UnTrackAllocation(void* ptr);

	U64 GetCurrentAllocatedSize();
	void DumpAllocations();

	void AddName(void* ptr, const std::string& name);

private:
	bool ContainsAllocation(void* ptr, std::unordered_map<void*, ProfilerMemoryAdditionalInfo>& collection);
	ProProfilerMemoryCollection::iterator GetIterator(void* ptr, ProProfilerMemoryCollection& collection);

private:
	CriticalSection m_locker;
	ProProfilerMemoryCollection m_currentAllocations;
	ProProfilerMemoryCollection m_allAllocations;
};

