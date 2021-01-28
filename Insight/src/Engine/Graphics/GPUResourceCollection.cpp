#include "ispch.h"
#include "GPUResourceCollection.h"
#include "GPUResource.h"

U64 GPUResourceCollection::GetMemoryUsage() const
{
    U64 result = 0;
    m_lock.Lock();

    for (I32 i = 0; i < m_collection.size(); ++i)
        result += m_collection[i]->GetMemoryUsage();

    m_lock.Unlock();
    return result;
}

void GPUResourceCollection::OnDeviceDestroy()
{
    m_lock.Lock();

    for (I32 i = (I32)m_collection.size() - 1; i >= 0 && i < (I32)m_collection.size(); i--)
    {
        if (m_collection[i])
        {
            m_collection[i]->ReleaseGPU();
        }
    }

    m_lock.Unlock();
}

void GPUResourceCollection::DumpToLog() const
{
    std::string output;

    m_lock.Lock();

    output.append(StringFormat("GPU Resources dump. Count: {0}, total GPU memory used: {1}", m_collection.size(), BytesToText(GetMemoryUsage())));
    output.append("\n");
    output.append("\n");

    for (I32 typeIndex = 0; typeIndex < GPUResource::ResourceType_Count; ++typeIndex)
    {
        const auto type = static_cast<GPUResource::ResourceType>(typeIndex);

        output.append(StringFormat("Group: {0}s", GPUResource::ToString(type)));
        output.append("\n");

        I32 count = 0;
        U64 memUsage = 0;
        for (I32 i = 0; i < m_collection.size(); ++i)
        {
            const auto resource = m_collection[i];
            if (resource->GetResourceType() == type)
            {
                count++;
                memUsage += resource->GetMemoryUsage();
                auto str = resource->GetType().GetTypeName();
                if (str.empty())
                {
                    output.append("\t");
                    output.append(str);
                    output.append("\n");
                }
            }
        }

        output.append(StringFormat("Total count: {0}, memory usage: {1}", count, BytesToText(memUsage)));
        output.append("\n");
        output.append("\n");
    }

    m_lock.Unlock();
}

void GPUResourceCollection::Add(GPUResource* resource)
{
    m_lock.Lock();

    auto it = std::find(m_collection.begin(), m_collection.end(), resource);
    ASSERT(resource && it == m_collection.end());
    bool foundEmptyPlace = false;
    for (auto& ptr : m_collection)
    {
        if (ptr == nullptr)
        {
            ptr = resource;
            foundEmptyPlace = true;
            break;
        }
    }
    if (!foundEmptyPlace)
    {
        m_collection.push_back(resource);
    }

    m_lock.Unlock();
}

void GPUResourceCollection::Remove(GPUResource* resource)
{
    m_lock.Lock();

    auto it = std::find_if(m_collection.begin(), m_collection.end(), [resource](GPUResource* gpuResource)
        {
            return resource == gpuResource;
        });
    ASSERT(resource && it != m_collection.end());
    m_collection.erase(it);

    m_lock.Unlock();
}
