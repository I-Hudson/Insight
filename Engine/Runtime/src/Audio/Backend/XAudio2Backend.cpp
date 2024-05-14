#if defined(IS_PLATFORM_WINDOWS) && defined(IS_AUDIO_XAUDIO_ENABLE)
#include "Audio/Backend/XAudio2Backend.h"

#include "Core/Asserts.h"
#include "Asset/AssetRegistry.h"

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfreadwrite.lib")

namespace Insight
{
    namespace Runtime
    {
#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

        XAudio2Backend::XAudio2Backend()
        {
            ASSERT(SUCCEEDED(XAudio2Create(&m_audio2, 0, XAUDIO2_DEFAULT_PROCESSOR)));

            XAUDIO2_DEBUG_CONFIGURATION debug = {};
            debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
            debug.BreakMask = XAUDIO2_LOG_ERRORS;
            m_audio2->SetDebugConfiguration(&debug, 0);

            ASSERT(SUCCEEDED(m_audio2->CreateMasteringVoice(&m_masteringVoice)));

            ASSERT(SUCCEEDED(MFStartup(MF_VERSION)));

            m_audioThread = std::thread(AudioThreadUpdates, this);
        }

        XAudio2Backend::~XAudio2Backend()
        {
            m_stopAudioThread = true;
            m_audioThread.join();

            ASSERT(SUCCEEDED(MFShutdown()));

            m_masteringVoice->DestroyVoice();
            m_audio2->Release();
        }

        u32 XAudio2Backend::PlaySound(const char* fileName)
        {
            const std::string audioFilePath = AssetRegistry::Instance().ValidatePath(fileName);

            const AssetInfo* assetInfo = AssetRegistry::Instance().GetAssetInfo(audioFilePath);
            for (size_t i = 0; i < m_playingSounds.size(); ++i)
            {
                if (m_playingSounds[i].AssetInfo == assetInfo)
                {
                    return m_playingSounds[i].Id;
                }
            }

            std::vector<u8> fileBuffer = AssetRegistry::Instance().LoadAssetData(audioFilePath);
            if (fileBuffer.empty())
            {
                return 0;
            }

            WAVEFORMATEXTENSIBLE wfx = { 0 };
            XAUDIO2_BUFFER buffer = { 0 };

            DWORD dwChunkSize;
            DWORD dwChunkPosition;
            //check the file type, should be fourccWAVE or 'XWMA'
            FindChunk(fileBuffer, fourccRIFF, dwChunkSize, dwChunkPosition);
            DWORD filetype;
            ReadChunkData(fileBuffer, &filetype, sizeof(DWORD), dwChunkPosition);
            if (filetype != fourccWAVE)
            {
                return false;
            }

            FindChunk(fileBuffer, fourccFMT, dwChunkSize, dwChunkPosition);
            ReadChunkData(fileBuffer, &wfx, dwChunkSize, dwChunkPosition);

            //fill out the audio data buffer with the contents of the fourccDATA chunk
            FindChunk(fileBuffer, fourccDATA, dwChunkSize, dwChunkPosition);
            BYTE* pDataBuffer = new BYTE[dwChunkSize];
            ReadChunkData(fileBuffer, pDataBuffer, dwChunkSize, dwChunkPosition);

            buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
            buffer.pAudioData = pDataBuffer;  //buffer containing audio data
            buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

            u32 soundId = 0;
            IXAudio2SourceVoice* sourceVoice = nullptr;
            for (size_t i = 0; i < m_playingSounds.size(); ++i)
            {
                XAudioSound& xAudioSound = m_playingSounds[i];
                if (xAudioSound.IsFree)
                {
                    ASSERT(xAudioSound.SourceVoice != nullptr);
                    ASSERT(xAudioSound.Buffer == nullptr);
                    ASSERT(xAudioSound.BufferSize > 0);
                    ASSERT(xAudioSound.Id > 0);
                    ASSERT(xAudioSound.IsFree == true);

                    xAudioSound.Buffer = pDataBuffer;
                    xAudioSound.BufferSize = dwChunkSize;
                    sourceVoice = xAudioSound.SourceVoice;
                    soundId = xAudioSound.Id;
                    xAudioSound.IsFree = false;
                }
            }

            if (sourceVoice == nullptr)
            {
                ASSERT(SUCCEEDED(m_audio2->CreateSourceVoice(&sourceVoice, (WAVEFORMATEX*)&wfx)));

                XAudioSound xAudioSound
                {
                    sourceVoice,
                    pDataBuffer,
                    dwChunkSize,
                    m_playingSounds.size() + 1,
                    assetInfo,
                    false
                };
                m_playingSounds.push_back(xAudioSound);
                soundId = m_playingSounds.back().Id;
            }

            // The audio sample data to which buffer points is still 'owned' by the app and must remain 
            // allocated and accessible until the sound stops playing.
            ASSERT(SUCCEEDED(sourceVoice->SubmitSourceBuffer(&buffer)));
            ASSERT(SUCCEEDED(sourceVoice->Start()));

            return soundId;
        }

        void XAudio2Backend::StopSound(const u32 soundId)
        {
            for (size_t i = 0; i < m_playingSounds.size(); ++i)
            {
                XAudioSound& xAudioSound = m_playingSounds[i];
                if (xAudioSound.Id == soundId)
                {
                    ASSERT(xAudioSound.SourceVoice->Stop() == S_OK);
                    xAudioSound.SourceVoice->FlushSourceBuffers();

                    delete[] xAudioSound.Buffer;
                    xAudioSound.Buffer = nullptr;
                    xAudioSound.BufferSize = 0;
                    xAudioSound.AssetInfo = nullptr;
                    xAudioSound.IsFree = true;
                }
            }
        }

        bool XAudio2Backend::FindChunk(const std::vector<u8>& fileBuffer, const DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition) const
        {
            HRESULT hr = S_OK;
            DWORD dwChunkType = 0;
            DWORD dwChunkDataSize = 0;
            DWORD dwRIFFDataSize = 0;
            DWORD dwFileType = 0;
            DWORD dwOffset = 0;

            if (fileBuffer.empty())
            {
                return false;
            }

            u64 fileDataOffset = 0;
            while (hr == S_OK)
            {
                if (memcpy_s(&dwChunkType, sizeof(dwChunkType), fileBuffer.data() + fileDataOffset, sizeof(dwChunkType)) != 0)
                {
                    return false;
                }
                fileDataOffset += sizeof(dwChunkType);

                if (memcpy_s(&dwChunkDataSize, sizeof(dwChunkDataSize), fileBuffer.data() + fileDataOffset, sizeof(dwChunkDataSize)) != 0)
                {
                    return false;
                }
                fileDataOffset += sizeof(dwChunkDataSize);

                switch (dwChunkType)
                {
                case fourccRIFF:
                    dwRIFFDataSize = dwChunkDataSize;
                    dwChunkDataSize = 4;
                    if (memcpy_s(&dwFileType, sizeof(dwFileType), fileBuffer.data() + fileDataOffset, sizeof(dwFileType)) != 0)
                    {
                        return false;
                    }
                    fileDataOffset += sizeof(dwFileType);
                    break;

                default:
                    fileDataOffset = dwChunkDataSize;
                }

                dwOffset += sizeof(DWORD) * 2;

                if (dwChunkType == fourcc)
                {
                    chunkSize = dwChunkDataSize;
                    chunkDataPosition = dwOffset;
                    return true;
                }

                dwOffset += dwChunkDataSize;

                if (fileDataOffset >= dwRIFFDataSize) return false;
            }
            
            return true;
        }

        bool XAudio2Backend::ReadChunkData(const std::vector<u8>& fileBuffer, void* buffer, const DWORD bufferSize, const DWORD bufferOffset) const
        {
            if (memcpy_s(buffer, bufferSize, fileBuffer.data() + bufferOffset, bufferSize) != 0)
            {
                return false;
            }
            return true;
        }

        void XAudio2Backend::AudioThreadUpdates(XAudio2Backend* audioBackend)
        {
            while (!m_stopAudioThread)
            {

            }
        }
    }
}

#undef fourccRIFF
#undef fourccDATA
#undef fourccFMT 
#undef fourccWAVE
#undef fourccXWMA
#undef fourccDPDS

#endif