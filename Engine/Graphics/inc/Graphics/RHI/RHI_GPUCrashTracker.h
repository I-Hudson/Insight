#pragma once

#include <iomanip>
#include <string>
#include <sstream>
#include <mutex>

#ifdef IS_PLATFORM_WINDOWS
#include <Windows.h>
#endif

#ifdef IS_NVIDIA_AFTERMATH_ENABLED
#ifdef IS_DX12_ENABLED
#include <d3dx12.h>
#endif
#include "GFSDK_Aftermath.h"
#include "GFSDK_Aftermath_GpuCrashDump.h"
#include "GFSDK_Aftermath_GpuCrashDumpDecoding.h"
#endif

namespace Insight
{
	namespace Graphics
	{
		class RHI_GPUCrashTracker
		{
		public:
			static RHI_GPUCrashTracker* Create();

			virtual void Init() = 0;
			virtual void Destroy() = 0;

			virtual void DeviceLost() = 0;

		protected:
			bool m_initiaised = false;
		};

#ifdef IS_NVIDIA_AFTERMATH_ENABLED
		class RHI_GPUCrashTrackerNvidiaAftermath : public RHI_GPUCrashTracker
		{
		public:
			virtual void Init() override;
			virtual void Destroy() override;

			virtual void DeviceLost() override;

		private:
			//*********************************************************
			// Callback handlers for GPU crash dumps and related data.
			//

			// Handler for GPU crash dump callbacks.
			void OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

			// Handler for shader debug information callbacks.
			void OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);

			// Handler for GPU crash dump description callbacks.
			void OnDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);

			// Handler for app-managed marker resolve callback
			void OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

			//*********************************************************
			// Helpers for writing a GPU crash dump and debug information
			// data to files.
			//

			// Helper for writing a GPU crash dump to a file.
			void WriteGpuCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);

			// Helper for writing shader debug information to a file
			void WriteShaderDebugInformationToFile(
				GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier,
				const void* pShaderDebugInfo,
				const uint32_t shaderDebugInfoSize);

			//*********************************************************
			// Helpers for decoding GPU crash dump to JSON.
			//

			// Handler for shader debug info lookup callbacks.
			void OnShaderDebugInfoLookup(
				const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier,
				PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const;

			// Handler for shader lookup callbacks.
			void OnShaderLookup(
				const GFSDK_Aftermath_ShaderBinaryHash& shaderHash,
				PFN_GFSDK_Aftermath_SetData setShaderBinary) const;

			// Handler for shader source debug info lookup callbacks.
			void OnShaderSourceDebugInfoLookup(
				const GFSDK_Aftermath_ShaderDebugName& shaderDebugName,
				PFN_GFSDK_Aftermath_SetData setShaderBinary) const;

			//*********************************************************
			// Static callback wrappers.
			//

			// GPU crash dump callback.
			static void GpuCrashDumpCallback(
				const void* pGpuCrashDump,
				const uint32_t gpuCrashDumpSize,
				void* pUserData);

			// Shader debug information callback.
			static void ShaderDebugInfoCallback(
				const void* pShaderDebugInfo,
				const uint32_t shaderDebugInfoSize,
				void* pUserData);

			// GPU crash dump description callback.
			static void CrashDumpDescriptionCallback(
				PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription,
				void* pUserData);

			// App-managed marker resolve callback
			static void ResolveMarkerCallback(
				const void* pMarkerData
				, const uint32_t markerDataSize
				, void* pUserData
				, void** ppResolvedMarkerData
				, uint32_t* pResolvedMarkerDataSize);

			// Shader debug information lookup callback.
			static void ShaderDebugInfoLookupCallback(
				const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier,
				PFN_GFSDK_Aftermath_SetData setShaderDebugInfo,
				void* pUserData);

			// Shader lookup callback.
			static void ShaderLookupCallback(
				const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash,
				PFN_GFSDK_Aftermath_SetData setShaderBinary,
				void* pUserData);

			// Shader source debug info lookup callback.
			static void ShaderSourceDebugInfoLookupCallback(
				const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName,
				PFN_GFSDK_Aftermath_SetData setShaderBinary,
				void* pUserData);


			// For thread-safe access of GPU crash tracker state.
			mutable std::mutex m_mutex;
		};
	}
}

namespace std
{
	template<typename T>
	inline std::string to_hex_string(T n)
	{
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(2 * sizeof(T)) << std::hex << n;
		return stream.str();
	}

	inline std::string to_string(GFSDK_Aftermath_Result result)
	{
		return std::string("0x") + to_hex_string(static_cast<uint32_t>(result));
	}

	inline std::string to_string(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier)
	{
		return to_hex_string(identifier.id[0]) + "-" + to_hex_string(identifier.id[1]);
	}

	inline std::string to_string(const GFSDK_Aftermath_ShaderBinaryHash& hash)
	{
		return to_hex_string(hash.hash);
	}
} // namespace std

//*********************************************************
// Helper for comparing shader hashes and debug info identifier.
//

// Helper for comparing GFSDK_Aftermath_ShaderDebugInfoIdentifier.
inline bool operator<(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& lhs, const GFSDK_Aftermath_ShaderDebugInfoIdentifier& rhs)
{
	if (lhs.id[0] == rhs.id[0])
	{
		return lhs.id[1] < rhs.id[1];
	}
	return lhs.id[0] < rhs.id[0];
}

// Helper for comparing GFSDK_Aftermath_ShaderBinaryHash.
inline bool operator<(const GFSDK_Aftermath_ShaderBinaryHash& lhs, const GFSDK_Aftermath_ShaderBinaryHash& rhs)
{
	return lhs.hash < rhs.hash;
}

// Helper for comparing GFSDK_Aftermath_ShaderDebugName.
inline bool operator<(const GFSDK_Aftermath_ShaderDebugName& lhs, const GFSDK_Aftermath_ShaderDebugName& rhs)
{
	return strncmp(lhs.name, rhs.name, sizeof(lhs.name)) < 0;
}

//*********************************************************
// Helper for checking Nsight Aftermath failures.
//

inline std::string  AftermathErrorMessage(GFSDK_Aftermath_Result result)
{
	switch (result)
	{
	case GFSDK_Aftermath_Result_FAIL_DriverVersionNotSupported:
		return "Unsupported driver version - requires an NVIDIA R495 display driver or newer.";
	default:
		return "Aftermath Error 0x" + std::to_hex_string(result);
	}
}

// Helper macro for checking Nsight Aftermath results and throwing exception
// in case of a failure.
#ifdef _WIN32
#define AFTERMATH_CHECK_ERROR(FC)                                                                       \
[&]() {                                                                                                 \
    GFSDK_Aftermath_Result _result = FC;                                                                \
    if (!GFSDK_Aftermath_SUCCEED(_result))                                                              \
    {                                                                                                   \
        MessageBoxA(0, AftermathErrorMessage(_result).c_str(), "Aftermath Error", MB_OK);               \
        exit(1);                                                                                        \
    }                                                                                                   \
}()
#else
#define AFTERMATH_CHECK_ERROR(FC)                                                                       \
[&]() {                                                                                                 \
    GFSDK_Aftermath_Result _result = FC;                                                                \
    if (!GFSDK_Aftermath_SUCCEED(_result))                                                              \
    {                                                                                                   \
        printf("%s\n", AftermathErrorMessage(_result).c_str());                                         \
        fflush(stdout);                                                                                 \
        exit(1);                                                                                        \
    }                                                                                                   \
}()
#endif
#endif