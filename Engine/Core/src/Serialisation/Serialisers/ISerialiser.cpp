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
            for (size_t i = 0; i < m_childSerialisers.size(); ++i)
            {
                Delete(m_childSerialisers.at(i));
            }
            m_childSerialisers.clear();
            m_childSerialisers.shrink_to_fit();
        }


        ISerialiser* ISerialiser::Create(SerialisationTypes type)
        {
            switch (type)
            {
            case Insight::Serialisation::SerialisationTypes::Json: return New<JsonSerialiser>(false);
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
            ASSERT(!m_name.empty());
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

        ISerialiser* ISerialiser::AddChildSerialiser()
        {
            ISerialiser* newSerialiser = ISerialiser::Create(m_type);
            AddChildSerialiser(newSerialiser);
            return newSerialiser;
        }

        void ISerialiser::AddChildSerialiser(ISerialiser* serialiser)
        {
            if (serialiser == nullptr)
            {
                return;
            }

            ASSERT(m_type == serialiser->GetType())
            m_childSerialisers.push_back(serialiser);
        }
    }
}