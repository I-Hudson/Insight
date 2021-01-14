/*-----------------------------------------------------------------------------

	Copyright 2001 Digital anvil

	CRC32.h

	JAN.24 2001 Written by Yuichi Ito

=============================================================================*/
#ifndef __CRC32_H
#define __CRC32_H
#include "Core.h"

//These functions convert capital letters to lower case (not good for raw byte streams)
u32 GetCRC32(const char* beginStr, const char* endStr);
u32 GetCRC32(const char* string);
u32 GetContinuedCRC32(u32 crc, const char* string);
u32 GetContinuedCRC32(u32 crc, const char c);
u32 GetContinuedCRC32(u32 crc, const char* beginStr, const char* endStr);

u32 GetContinuedCaseSensitiveCRC32(u32 crc, const char* string);

u32 GetCaseSensitiveCRC32(const char* string);
u32 GetFileCRC32(const char* data, size_t size);

//Standard CRC32 (ok for raw byte streams / without the use of tolower)
u32 GetRawCRC32(const void* data, u32 dataSize);
u32 GetContinuedRawCRC32(u32 crc, const void* data, u32 dataSize);

#endif // __CRC32_H
