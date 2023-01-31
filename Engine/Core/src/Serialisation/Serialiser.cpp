#include "Serialisation/Serialiser.h"

namespace Insight
{
    namespace Serialisation
    {
        ISerialiser::ISerialiser(bool isReadMode)
            : m_isReadMode(isReadMode)
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


        void ISerialiser::SetVersion(u32 currentVersion)
        {
            m_version = currentVersion;
        }

        u32 ISerialiser::GetVersion() const
        {
            m_version;
        }

        bool ISerialiser::IsReadMode() const
        {
            return m_isReadMode;
        }
    }
}