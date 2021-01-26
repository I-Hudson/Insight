#include "ispch.h"
#include "ProfilerMemory.h"
#include "Engine/Platform/Platform.h"

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

	ASSERT(!ContainsAllocation(ptr, m_currentAllocations) && !ContainsAllocation(ptr, m_allAllocations));

	m_currentAllocations[ptr] = ProfilerMemoryAdditionalInfo { "", size };
	m_allAllocations[ptr] = ProfilerMemoryAdditionalInfo { "", size };

	m_locker.Unlock();
}

void ProfilerMemory::UnTrackAllocation(void* ptr)
{
	m_locker.Lock();

	ASSERT(ContainsAllocation(ptr, m_currentAllocations));
	auto currentIt = GetIterator(ptr, m_currentAllocations);
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
		IS_INFO("Allocation: \n\t Ptr: {0} \n\t Name:{1} \n\t Size: {2}", alloc.first, alloc.second.Name, alloc.second.Size);
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
		char* c1 = (char*)ptr;
		char* c2 = (char*)alloc.first;

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
}
