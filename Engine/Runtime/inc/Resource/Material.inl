#include "Resource/Material.h"
#include "Resource/ResourceManager.h"

namespace Insight
{
    namespace Serialisation
    {
        void ComplexSerialiser<MaterialTextureSerialise
            , std::array<Runtime::ResourceLFHandle<Runtime::Texture2D>, static_cast<u32>(Runtime::TextureTypes::Count)>
            , Runtime::Material>::operator()
            (ISerialiser* serialiser
                , std::array<Runtime::ResourceLFHandle<Runtime::Texture2D>, static_cast<u32>(Runtime::TextureTypes::Count)>& textures
                , Runtime::Material* material) const
        {
            ASSERT(serialiser);
            constexpr const char* c_Textures = "Textures";
            if (serialiser->IsReadMode())
            {
                Serialisation::PropertyDeserialiser<Core::GUID> guidDeserialiser;
                u64 arraySize = 0;
                serialiser->StartArray(c_Textures, arraySize);
                for (size_t i = 0; i < arraySize; ++i)
                {
                    std::string serialisedGuid;
                    serialiser->Read("", serialisedGuid);
                    if (serialisedGuid.empty())
                    {
                        continue;
                    }

                    Core::GUID textureGuid = guidDeserialiser(serialisedGuid);
                    TObjectPtr<Runtime::IResource> texture = Runtime::ResourceManager::GetResourceFromGuid(textureGuid);
                    if (texture)
                    {
                        textures[i] = static_cast<Runtime::Texture2D*>(texture.Get());
                    }
                }
                serialiser->StopArray();
            }
            else
            {
                Serialisation::PropertySerialiser<Core::GUID> guidSerialiser;
                u64 arraySize = textures.size();
                serialiser->StartArray(c_Textures, arraySize);
                for (Runtime::ResourceLFHandle<Runtime::Texture2D> const& texture : textures)
                {
                    if (texture)
                    {
                        serialiser->Write("", guidSerialiser(texture->GetGuid()));
                    }
                    else
                    {
                        serialiser->Write("", "");
                    }
                }
                serialiser->StopArray();
            }
        }
	}
}