#pragma once

#include "Resource/ResourceDatabase.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<ResourceDatabase1, Runtime::ResourceDatabase::ResourceOwningMap, Runtime::ResourceDatabase>::operator()
            (Runtime::ResourceDatabase::ResourceOwningMap const& map, Runtime::ResourceDatabase* resourceDatabase, ISerialiser* serialiser) const
            {
                ASSERT(serialiser);
                if (serialiser->IsReadMode())
                {
                    u64 resoucesToSave = 0;
                    serialiser->StartArray("Resources", resoucesToSave);
                    for (u64 i = 0; i < resoucesToSave; ++i)
                    {
                        Runtime::ResourceId resouceId;
                        resouceId.Deserialise(serialiser);
                        Runtime::ResourceManager::Load(resouceId);
                    }
                    serialiser->StopArray();
                }
                else
                {
                    u64 resoucesToSave = 0;
                    for (auto const& pair : map)
                    {
                        if (!pair.second->IsDependentOnAnotherResource())
                        {
                            ++resoucesToSave;
                        }
                    }

                    serialiser->StartArray("Resources", resoucesToSave);
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