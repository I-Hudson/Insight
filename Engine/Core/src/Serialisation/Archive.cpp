#include "Serialisation/Archive.h"
#include "FileSystem/FileSystem.h"

#include "Platforms/Platform.h"

#include <fstream>
#include <istream>

namespace Insight
{
    Archive::Archive(std::string_view filePath, ArchiveModes mode)
        : Archive(filePath, mode, FileType::Binary)
    {

    }

    Archive::Archive(std::string_view filePath, ArchiveModes mode, FileType fileType)
        : m_filePath(filePath)
        , m_mode(mode)
    {
        if (filePath.empty())
        {
            return;
        }

        std::string absFilePath = FileSystem::GetAbsolutePath(filePath);
        if (mode == ArchiveModes::Read)
        {
            std::ifstream inStream;
            inStream.open(absFilePath, std::ios::in | c_FileTypeToStdIos[static_cast<int>(fileType)]);
            if (inStream.is_open())
            {
                inStream.seekg(0, std::ios::end);
                u64 dataSize = inStream.tellg();
                inStream.seekg(0, std::ios::beg);

                m_data.resize(dataSize / sizeof(char));

                inStream.read((char*)m_data.data(), m_data.size());
                inStream.close();
            }
            m_dataPtr = m_data.data();
            m_streamPos = 0;
            Close();
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
            FileSystem::SaveToFile(m_data, m_filePath, true);
            m_data.clear();
        }
        m_filePath = {};
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

    bool Archive::IsEmpty() const
    {
        return m_data.empty();
    }

    std::vector<Byte> Archive::GetData() const
    {
        return m_data;
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