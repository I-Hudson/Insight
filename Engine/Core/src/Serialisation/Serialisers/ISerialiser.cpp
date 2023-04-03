#include "Serialisation/Serialisers/ISerialiser.h"
#include "Serialisation/JsonSerialiser.h"

#include "Core/Memory.h"
#include "Platforms/Platform.h"

namespace Insight
{
    namespace Serialisation
    {
        ISerialiser::ISerialiser(SerialisationTypes type)
            : m_type(type)
        { }

        ISerialiser::ISerialiser(SerialisationTypes type, bool isReadMode)
            : m_type(type)
            , m_isReadMode(isReadMode)
        { }

        ISerialiser::~ISerialiser()
        {
        }

        ISerialiser* ISerialiser::Create(SerialisationTypes type, bool isReadMode)
        {
            switch (type)
            {
            case Insight::Serialisation::SerialisationTypes::Json: return New<JsonSerialiser, Core::MemoryAllocCategory::Serialiser>(isReadMode);
            case Insight::Serialisation::SerialisationTypes::Binary:
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
    }
}