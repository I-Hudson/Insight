#pragma once

#include "Engine/Platform/Platform.h"
#include "Engine/Platform/CriticalSection.h"

class GPUResource;

class GPUResourceCollection
{
public:
	GPUResourceCollection(const U32& size)
		: m_collection(size)
	{ }

	~GPUResourceCollection() 
	{ }

	U64 GetMemoryUsage() const;

	void OnDeviceDestroy();

	void DumpToLog() const;

	void Add(GPUResource* resource);

	void Remove(GPUResource* resource);

private:

	CriticalSection m_lock;
	std::vector<GPUResource*> m_collection;
};

