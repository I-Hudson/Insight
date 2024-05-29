#include "Graphics/RHI/RHI_GPUCrashTracker.h"
#ifdef IS_DX12_ENABLED
#include "Graphics/RHI/DX12/RenderContext_DX12.h"
#endif
#ifdef IS_NVIDIA_AFTERMATH_ENABLED
#include "GFSDK_Aftermath.h"
#endif

#include "Core/Memory.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "FileSystem/FileSystem.h"

#include <string>

#ifdef IS_NVIDIA_AFTERMATH_ENABLED

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

namespace Insight
{
	namespace Graphics
	{
		constexpr static const char* c_CrashFolder = "gpuCrashDumps/";
		constexpr static const char* c_shaderFolder = "gpuShaders/";

		RHI_GPUCrashTracker* RHI_GPUCrashTracker::Create()
		{
#ifdef IS_NVIDIA_AFTERMATH_ENABLED
			return NewTracked(RHI_GPUCrashTrackerNvidiaAftermath);
#endif // IS_NVIDIA_AFTERMATH_ENABLED
			return nullptr;
		}

#ifdef IS_NVIDIA_AFTERMATH_ENABLED
		void RHI_GPUCrashTrackerNvidiaAftermath::Init()
		{
			// Enable GPU crash dumps and set up the callbacks for crash dump notifications,
			// shader debug information notifications, and providing additional crash
			// dump description data.Only the crash dump callback is mandatory. The other two
			// callbacks are optional and can be omitted, by passing nullptr, if the corresponding
			// functionality is not used.
			// The DeferDebugInfoCallbacks flag enables caching of shader debug information data
			// in memory. If the flag is set, ShaderDebugInfoCallback will be called only
			// in the event of a crash, right before GpuCrashDumpCallback. If the flag is not set,
			// ShaderDebugInfoCallback will be called for every shader that is compiled.
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_EnableGpuCrashDumps(
				GFSDK_Aftermath_Version_API,
				GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan | GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_DX,
				GFSDK_Aftermath_GpuCrashDumpFeatureFlags_Default,				  // Let the Nsight Aftermath library cache shader debug information.
				GpuCrashDumpCallback,                                             // Register callback for GPU crash dumps.
				ShaderDebugInfoCallback,                                          // Register callback for shader debug information.
				CrashDumpDescriptionCallback,                                     // Register callback for GPU crash dump description.
				ResolveMarkerCallback,                                            // Register callback for resolving application-managed markers.
				this));                                                           // Set the GpuCrashTracker object as user data for the above callbacks.

#ifdef IS_DX12_ENABLED
			// Initialize Nsight Aftermath for this device.
			const uint32_t aftermathFlags =
				GFSDK_Aftermath_FeatureFlags_EnableMarkers |             // Enable event marker tracking.
				GFSDK_Aftermath_FeatureFlags_CallStackCapturing |        // Enable automatic call stack event markers.
				GFSDK_Aftermath_FeatureFlags_EnableResourceTracking |    // Enable tracking of resources.
				GFSDK_Aftermath_FeatureFlags_GenerateShaderDebugInfo |   // Generate debug information for shaders.
				GFSDK_Aftermath_FeatureFlags_EnableShaderErrorReporting; // Enable additional runtime shader error reporting.

			RHI::DX12::RenderContext_DX12& renderContextDX12 = static_cast<RHI::DX12::RenderContext_DX12&>(RenderContext::Instance());
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DX12_Initialize(
				GFSDK_Aftermath_Version_API,
				aftermathFlags,
				renderContextDX12.GetDevice()));
#endif

			m_initiaised = true;
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::Destroy()
		{
			IS_PROFILE_FUNCTION();
			if (m_initiaised)
			{
				AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_DisableGpuCrashDumps());
				m_initiaised = false;
			}
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::DeviceLost()
		{
			if (m_initiaised)
			{
				GFSDK_Aftermath_CrashDump_Status status = GFSDK_Aftermath_CrashDump_Status_Unknown;
				AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

				auto tStart = std::chrono::steady_clock::now();
				auto tElapsed = std::chrono::milliseconds::zero();

				// Loop while Aftermath crash dump data collection has not finished or
				// the application is still processing the crash dump data.
				while (status != GFSDK_Aftermath_CrashDump_Status_CollectingDataFailed &&
					   status != GFSDK_Aftermath_CrashDump_Status_Finished &&
					   tElapsed.count() < 3000)
				{
					// Sleep a couple of milliseconds and poll the status again.
					std::this_thread::sleep_for(std::chrono::milliseconds(50));
					AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetCrashDumpStatus(&status));

					tElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - tStart);
				}

				if (status == GFSDK_Aftermath_CrashDump_Status_Finished)
				{
					IS_LOG_CORE_INFO("Aftermath finished processing the crash dump.\n");
				}
				else
				{
					IS_LOG_CORE_INFO("Unexpected crash dump status after timeout: %d\n", static_cast<int>(status));
				}
			}
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
		{
			// Make sure only one thread at a time...
			std::lock_guard<std::mutex> lock(m_mutex);
			// Write to file for later in-depth analysis with Nsight Graphics.
			WriteGpuCrashDumpToFile(pGpuCrashDump, gpuCrashDumpSize);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize)
		{
			// Make sure only one thread at a time...
			std::lock_guard<std::mutex> lock(m_mutex);

			// Get shader debug information identifier
			GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GetShaderDebugInfoIdentifier(
				GFSDK_Aftermath_Version_API,
				pShaderDebugInfo,
				shaderDebugInfoSize,
				&identifier));

			// Write to file for later in-depth analysis of crash dumps with Nsight Graphics
			WriteShaderDebugInformationToFile(identifier, pShaderDebugInfo, shaderDebugInfoSize);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription)
		{
			// Add some basic description about the crash. This is called after the GPU crash happens, but before
			// the actual GPU crash dump callback. The provided data is included in the crash dump and can be
			// retrieved using GFSDK_Aftermath_GpuCrashDump_GetDescription().
			addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, "Insight");
			addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, "v1.0");
			addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined, "This is a GPU crash dump example.");
			addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 1, "Engine State: Rendering.");
			addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_UserDefined + 2, "More user-defined information...");
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize)
		{
			// Important: the pointer passed back via resolvedMarkerData must remain valid after this function returns
			// using references for all of the m_markerMap accesses ensures that the pointers refer to the persistent data
			//for (auto& map : m_markerMap)
			//{
			//	const auto& foundMarker = map.find((uint64_t)pMarker);
			//	if (foundMarker != map.end())
			//	{
			//		const std::string& markerData = foundMarker->second;
			//		// std::string::data() will return a valid pointer until the string is next modified
			//		// we don't modify the string after calling data() here, so the pointer should remain valid
			//		*resolvedMarkerData = (void*)markerData.data();
			//		*markerSize = (uint32_t)markerData.length();
			//		return;
			//	}
			//}
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::WriteGpuCrashDumpToFile(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize)
		{
			// Create a GPU crash dump decoder object for the GPU crash dump.
			GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_CreateDecoder(
				GFSDK_Aftermath_Version_API,
				pGpuCrashDump,
				gpuCrashDumpSize,
				&decoder));

			// Use the decoder object to read basic information, like application
			// name, PID, etc. from the GPU crash dump.
			GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo));

			// Use the decoder object to query the application name that was set
			// in the GPU crash dump description.
			uint32_t applicationNameLength = 0;
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(
				decoder,
				GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
				&applicationNameLength));

			std::vector<char> applicationName(applicationNameLength, '\0');

			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetDescription(
				decoder,
				GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName,
				uint32_t(applicationName.size()),
				applicationName.data()));

			// Create a unique file name for writing the crash dump data to a file.
			// Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
			// driver release) we may see redundant crash dumps. As a workaround,
			// attach a unique count to each generated file name.
			static int count = 0;
			const std::string baseFileName =
				std::string(applicationName.data())
				+ "-"
				+ std::to_string(baseInfo.pid)
				+ "-"
				+ std::to_string(++count);

			// Write the crash dump data to a file using the .nv-gpudmp extension
			// registered with Nsight Graphics.
			const std::string crashDumpFileName = c_CrashFolder + baseFileName + ".nv-gpudmp";
			FileSystem::CreateFolder(c_CrashFolder);

			std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
			if (dumpFile)
			{
				dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
				dumpFile.close();
			}

			// Decode the crash dump to a JSON string.
			// Step 1: Generate the JSON and get the size.
			uint32_t jsonSize = 0;
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
				decoder,
				GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
				GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
				ShaderDebugInfoLookupCallback,
				ShaderLookupCallback,
				ShaderSourceDebugInfoLookupCallback,
				this,
				&jsonSize));
			// Step 2: Allocate a buffer and fetch the generated JSON.
			std::vector<char> json(jsonSize);
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_GetJSON(
				decoder,
				uint32_t(json.size()),
				json.data()));

			// Write the crash dump data as JSON to a file.
			const std::string jsonFileName = crashDumpFileName + ".json";
			std::ofstream jsonFile(jsonFileName, std::ios::out | std::ios::binary);
			if (jsonFile)
			{
				// Write the JSON to the file (excluding string termination)
				jsonFile.write(json.data(), json.size() - 1);
				jsonFile.close();
			}

			// Destroy the GPU crash dump decoder object.
			AFTERMATH_CHECK_ERROR(GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder));
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::WriteShaderDebugInformationToFile(GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier, const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize)
		{
			// Create a unique file name.
			const std::string filePath = "shader-" + std::to_string(identifier) + ".nvdbg";

			FileSystem::CreateFolder(c_shaderFolder);
			std::ofstream f(c_shaderFolder + filePath, std::ios::out | std::ios::binary);
			if (f)
			{
				f.write((const char*)pShaderDebugInfo, shaderDebugInfoSize);
				f.close();
			}
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& identifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) const
		{
			// Search the list of shader debug information blobs received earlier.
			//auto i_debugInfo = m_shaderDebugInfo.find(identifier);
			//if (i_debugInfo == m_shaderDebugInfo.end())
			//{
			//	// Early exit, nothing found. No need to call setShaderDebugInfo.
			//	return;
			//}
			//
			//// Let the GPU crash dump decoder know about the shader debug information
			//// that was found.
			//setShaderDebugInfo(i_debugInfo->second.data(), uint32_t(i_debugInfo->second.size()));
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) const
		{
			// Find shader binary data for the shader hash in the shader database.
			//std::vector<uint8_t> shaderBinary;
			//if (!m_shaderDatabase.FindShaderBinary(shaderHash, shaderBinary))
			//{
			//	// Early exit, nothing found. No need to call setShaderBinary.
			//	return;
			//}
			//
			//// Let the GPU crash dump decoder know about the shader data
			//// that was found.
			//setShaderBinary(shaderBinary.data(), uint32_t(shaderBinary.size()));
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::OnShaderSourceDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary) const
		{
			// Find source debug info for the shader DebugName in the shader database.
			std::vector<uint8_t> shaderBinary;
			//if (!m_shaderDatabase.FindShaderBinaryWithDebugData(shaderDebugName, shaderBinary))
			//{
			//	// Early exit, nothing found. No need to call setShaderBinary.
			//	return;
			//}
			//
			//// Let the GPU crash dump decoder know about the shader debug data that was
			//// found.
			//setShaderBinary(shaderBinary.data(), uint32_t(shaderBinary.size()));
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::GpuCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnCrashDump(pGpuCrashDump, gpuCrashDumpSize);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::ShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnShaderDebugInfo(pShaderDebugInfo, shaderDebugInfoSize);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnDescription(addDescription);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::ResolveMarkerCallback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnResolveMarker(pMarkerData, markerDataSize, pUserData, ppResolvedMarkerData, pResolvedMarkerDataSize);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::ShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnShaderDebugInfoLookup(*pIdentifier, setShaderDebugInfo);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::ShaderLookupCallback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnShaderLookup(*pShaderHash, setShaderBinary);
		}

		void RHI_GPUCrashTrackerNvidiaAftermath::ShaderSourceDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData)
		{
			RHI_GPUCrashTrackerNvidiaAftermath* pGpuCrashTracker = reinterpret_cast<RHI_GPUCrashTrackerNvidiaAftermath*>(pUserData);
			pGpuCrashTracker->OnShaderSourceDebugInfoLookup(*pShaderDebugName, setShaderBinary);
		}
#endif
	}
}