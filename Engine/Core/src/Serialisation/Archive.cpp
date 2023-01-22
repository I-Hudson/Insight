#include "Serialisation/Archive.h"
#include "FileSystem/FileSystem.h"

#include "Platforms/Platform.h"

#include <fstream>

namespace Insight
{
    Archive::Archive(std::string_view filePath, ArchiveModes mode)
    {
        if (filePath.empty())
        {
            return;
        }

        std::string absFilePath = FileSystem::FileSystem::GetAbsolutePath(filePath);
        if (mode == ArchiveModes::Read)
        {

        }
        else
        {
            m_filePath = absFilePath;
            CreateEmpty();
        }
    }

    Archive::~Archive()
    {
        Close();
    }

    void Archive::Close()
    {
        if (!IsRead() && !m_filePath.empty())
        {
            // Write to file.
            std::fstream stream{};
            stream.open(m_filePath, std::ios::out | std::ios::trunc);
            if (stream.is_open())
            {
                stream.write((const char*)m_data.data(), static_cast<std::streamsize>(m_streamPos));
                stream.close();
            }
        }
        m_data.clear();
        m_dataPtr = nullptr;
        m_streamPos = 0;
    }

    bool Archive::IsOpen() const
    {
        return m_dataPtr != nullptr;
    }

    bool Archive::IsRead() const
    {
        return m_mode == ArchiveModes::Read;
    }

    u64 Archive::GetSize() const
    {
        return m_streamPos;
    }

    void Archive::CreateEmpty()
    {
        m_data.resize(128);
        m_dataPtr = m_data.data();
    }

    void Archive::Write(const char* data, u64 size)
    {
        Write((const Byte*)(data), size);
    }

    void Archive::Write(const void* data, u64 size)
    {
        Write((const Byte*)(data), size);
    }

    void Archive::Write(const Byte* data, u64 size)
    {
        ASSERT(!IsRead());
        ASSERT(IsOpen());
        const u64 newStreamPosition = m_streamPos + size;
        if (newStreamPosition > m_data.size())
        {
            m_data.resize(newStreamPosition * 2);
            m_dataPtr = m_data.data();
        }
        Platform::MemCopy(m_dataPtr + m_streamPos, data, size);
        m_streamPos = newStreamPosition;
    }
}