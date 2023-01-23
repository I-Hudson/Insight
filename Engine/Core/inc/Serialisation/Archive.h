#pragma once

#include "Core/TypeAlias.h"
#include "Core/Defines.h"

#include <string_view>
#include <vector>

namespace Insight
{
    enum class ArchiveModes
    {
        Read,
        Write
    };

    // This is a data container used for serialization purposes.
    //	It can be used to READ or WRITE data, but not both at the same time.
    //	An archive that was created in WRITE mode can be changed to read mode and vica-versa
    //	The data flow is always FIFO (first in, first out)
    class IS_CORE Archive
    {
    public:
        Archive() = default;
        Archive(const Archive& other) = default;
        Archive(Archive&& other) = default;
        Archive(std::string_view filePath, ArchiveModes mode);
        ~Archive();

        Archive& operator=(const Archive&) = default;
        Archive& operator=(Archive&&) = default;

        void Close();

        bool IsOpen() const;
        bool IsRead() const;
        /// @brief Return the data size this Archive has in bytes.
        /// @return u64
        u64 GetSize() const;

        std::vector<Byte> GetData() const;

        void Write(const char* data, u64 size);
        void Write(const void* data, u64 size);
        void Write(const Byte* data, u64 size);

    private:
        void CreateEmpty();

    private:
        std::string m_filePath;
        ArchiveModes m_mode;
        std::vector<Byte> m_data;
        Byte* m_dataPtr = nullptr;
        /// @brief The current position relative to the starting position in the memory stream.
        u64 m_streamPos = 0;
    };
}