#pragma once

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<ResourceDatabase1, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            if (serialiser->IsReadMode())
            {
                u64 resourcesToSave = 0;
                serialiser->StartArray("Resources", resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);

                    TObjectPtr<Runtime::IResource> resource = Runtime::ResourceManager::Create(resouceId);
                    if (resource)
                    {
                        resource->Deserialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        const_cast<Runtime::ResourceId&>(pair.first).Serialise(serialiser);
                        pair.second->Serialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
        }

        void ComplexSerialiser<ResourceDatabase2, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (ISerialiser* serialiser, Runtime::ResourceDatabase::ResourceOwningMap& map, Runtime::ResourceDatabase* resourceDatabase) const
        {
            ASSERT(serialiser);
            if (serialiser->IsReadMode())
            {
                u64 resourcesToSave = 0;
                serialiser->StartArray("Resources", resourcesToSave);
                for (u64 i = 0; i < resourcesToSave; ++i)
                {
                    Runtime::ResourceId resouceId;
                    resouceId.Deserialise(serialiser);
                    Runtime::ResourceManager::Create(resouceId);
                }
                serialiser->StopArray();
            }
            else
            {
                u64 resourcesToSave = 0;
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        ++resourcesToSave;
                    }
                }

                serialiser->StartArray("Resources", resourcesToSave);
                for (auto const& pair : map)
                {
                    if (!pair.second->IsDependentOnAnotherResource())
                    {
                        const_cast<Runtime::ResourceId&>(pair.first).Serialise(serialiser);
                    }
                }
                serialiser->StopArray();
            }
        }
    }
}