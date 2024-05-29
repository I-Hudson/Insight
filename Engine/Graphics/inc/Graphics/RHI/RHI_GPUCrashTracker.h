#pragma once

#include "Core/Singleton.h"

#ifdef IS_NVIDIA_AFTERMATH_ENABLED
#include "GFSDK_Aftermath_GpuCrashDump.h"
#include "GFSDK_Aftermath_GpuCrashDumpDecoding.h"
#endif

#include <iomanip>
#include <string>
#include <sstream>
#include <mutex>

#ifdef IS_PLATFORM_WINDOWS
//#include <Windows.h>
#endif

namespace Insight
{
	namespace Graphics
	{
		class RHI_GPUCrashTracker : public Core::Singleton<RHI_GPUCrashTracker>
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
#endif
	}
}