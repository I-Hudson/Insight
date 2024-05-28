#include "Serialisation/Serialisers/ISerialiser.h"
#include "Serialisation/Serialisers/ISerialiserHeader.h"

#include "Serialisation/Serialisers/JsonSerialiser.h"
#include "Serialisation/Serialisers/BinarySerialiser.h"

#include "Core/Memory.h"
#include "Core/Asserts.h"

#include "Platforms/Platform.h"

namespace Insight
{
    namespace Serialisation
    {
        ISerialiser::ISerialiser(SerialisationTypes type, bool isReadMode)
            : m_type(type)
            , m_isReadMode(isReadMode)
        {
        }

        ISerialiser::~ISerialiser()
        {
        }

        ISerialiser* ISerialiser::Create(SerialisationTypes type, bool isReadMode)
        {
            switch (type)
            {
            case Insight::Serialisation::SerialisationTypes::Json: return New<JsonSerialiser, Core::MemoryAllocCategory::Serialiser>(isReadMode);
            case Insight::Serialisation::SerialisationTypes::Binary: return New<BinarySerialiser, Core::MemoryAllocCategory::Serialiser>(isReadMode);
                break;
            default:
                break;
            }
            FAIL_ASSERT();
            return nullptr;
        }

        void ISerialiser::SetName(std::string_view tag)
        {
            m_name = tag;
        }

        std::string_view ISerialiser::GetName() const
        {
            return m_name;
        }

        void ISerialiser::SetVersion(u32 currentVersion)
        {
            m_version = currentVersion;
        }

        u32 ISerialiser::GetVersion() const
        {
            return m_version;
        }

        SerialisationTypes ISerialiser::GetType() const
        {
            return m_type;
        }

        bool ISerialiser::IsReadMode() const
        {
            return m_isReadMode;
        }

        void ISerialiser::Write(std::string_view tag, const char* string)
        {
            Write(tag, std::string(string));
        }

        void ISerialiser::WriteHeader(std::vector<Byte>& data) const
        {
            BinarySerialiser serialiser(false);
            ISerialiserHeader header;
            header.Type = static_cast<u8>(m_type);

            header.Serialise(&serialiser);
            
            const u64 headerSize = serialiser.m_head.Size;
            const u64 dataSize = data.size();
            const u64 newDataSize = dataSize + headerSize;

            data.resize(newDataSize);

            // Move the data to the right by 'headerSize' to allow for 'headerSize' in the front of the data vector.
            Platform::MemCopy(data.data() + headerSize, data.data(), dataSize);
            // Copy the header data from the binary serialiser to the front of the data vector.
            Platform::MemCopy(data.data(), serialiser.m_head.Data, headerSize);
        }

        bool ISerialiser::ValidateHeader(std::vector<Byte>& data) const
        {
            if (data.empty())
            {
                return false;
            }

            /// Deserialise the data vector with no validation. With validation and this would be a 
            /// circle of the binary serialiser always going Deserialise->ValidateHeader->Deserialise->ValidateHeader...
            /// We don't care about any data after the header and as the header should always be in binary format this should
            /// be ok.
            BinarySerialiser serialiser(true);
            serialiser.DeserialiseNoValidate(data);

            ISerialiserHeader header;
            header.Deserialise(&serialiser);

            const u64 headerSize = serialiser.m_head.Size;
            data.erase(data.begin(), data.begin() + headerSize);

            return header.Type == static_cast<u8>(m_type);
        }
    }
}