#include "Platforms/Windows/PlatformWindows.h"
#include "FileSystem/FileSystem.h"

#ifdef IS_PLATFORM_WINDOWS

#include "Core/Logger.h"
#include "Core/Asserts.h"

#include "Platforms/Platform.h"

#include <algorithm>

#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>

#include <powerbase.h>
#pragma comment(lib, "PowrProf.lib")

#include <psapi.h>

#include <rpc.h>
#include <Objbase.h>
#include <CommCtrl.h>
#include <ShlObj.h>
#include <lmwksta.h>
#pragma comment(lib, "Netapi32.lib")
#include <lmapibuf.h>
#include <VersionHelpers.h>

#ifdef AMD_Ryzen_Master_SDK
#include <IPlatform.h>
#include <IDevice.h>
#include <IDeviceManager.h>
#include <ICPUEx.h>
#include <IBIOSEx.h>
#endif

namespace Insight
{
	namespace Windows
	{
		Core::MemoryInformation PlatformWindows::s_memoryInformation;
		Core::CPUInformation PlatformWindows::s_cpuInformation;

		u32 PlatformWindows::s_mainThreadId;
		unsigned long PlatformWindows::s_processId;
		void* PlatformWindows::s_memoryReadProcessHandle;

		class CPUID 
		{
			uint32_t regs[4];

		public:
			explicit CPUID(unsigned funcId, unsigned subFuncId) {
#ifdef _WIN32
				__cpuidex((int*)regs, (int)funcId, (int)subFuncId);

#else
				asm volatile
					("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3])
						: "a" (funcId), "c" (subFuncId));
				// ECX is set to zero for CPUID function 4
#endif
			}

			const uint32_t& EAX() const { return regs[0]; }
			const uint32_t& EBX() const { return regs[1]; }
			const uint32_t& ECX() const { return regs[2]; }
			const uint32_t& EDX() const { return regs[3]; }
		};

		typedef struct _PROCESSOR_POWER_INFORMATION {
			ULONG Number;
			ULONG MaxMhz;
			ULONG CurrentMhz;
			ULONG MhzLimit;
			ULONG MaxIdleState;
			ULONG CurrentIdleState;
		} PROCESSOR_POWER_INFORMATION, * PPROCESSOR_POWER_INFORMATION;

		void PlatformWindows::Initialise()
		{
			ASSERT(SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)));

			INITCOMMONCONTROLSEX iccex;
			iccex.dwSize = sizeof(iccex);
			iccex.dwICC = ICC_PROGRESS_CLASS;
			ASSERT(InitCommonControlsEx(&iccex));

			s_memoryInformation = GetMemoryInformation();
			s_cpuInformation = GetCPUInformation();

			s_mainThreadId = GetCurrentThreadId();
			s_processId = GetCurrentProcessId();
			s_memoryReadProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, s_processId);
		}

		void PlatformWindows::Shutdown()
		{
			CloseHandle(s_memoryReadProcessHandle);
			CoUninitialize();
		}

		void PlatformWindows::MemCopy(void* dst, void const* src, u64 size)
		{
			memcpy(dst, src, size);
		}

		void PlatformWindows::MemSet(void* dst, int value, u64 size)
		{
			memset(dst, value, size);
		}

		void PlatformWindows::MemClear(void* dst, u64 size)
		{
			MemSet(dst, 0, size);
		}

		bool PlatformWindows::MemCompare(const void* buf1, const void* buf2, u64 size)
		{
			return memcmp(buf1, buf2, size);
		}

		bool PlatformWindows::StrCompare(const char* str1, const char* str2)
		{
			return strcmp(str1, str2);
		}

		bool PlatformWindows::WStrCompare(const wchar_t* wstr1, const wchar_t* wstr2)
		{
			return wcscmp(wstr1, wstr1);
		}

		bool PlatformWindows::RunProcessAndWait(const char* command)
		{
			bool success = true;
			STARTUPINFOA info = { sizeof(info) };
			PROCESS_INFORMATION processInfo;

			if (!CreateProcessA("C:\\Windows\\System32\\cmd.exe", (char*)command, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
			{
				IS_LOG_CORE_ERROR("[PackageBuild::BuildSolution] Unable to compile solution: '{0}'.", GetLastError());
				success = false;
			}
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			CloseHandle(processInfo.hProcess);
			CloseHandle(processInfo.hThread);

			return success;
		}

		/// https:///gist.github.com/rioki/85ca8295d51a5e0b7c56e5005b0ba8b4	
		inline std::string basename(const std::string& file)
		{
			unsigned int i = static_cast<int>(file.find_last_of("\\/"));
			if (i == std::string::npos)
			{
				return file;
			}
			else
			{
				return file.substr(i + 1);
			}
		}

		struct StackFrame
		{
			DWORD64 address;
			std::string name;
			std::string module;
			unsigned int line;
			std::string file;
		};

		std::vector<std::string> PlatformWindows::GetCallStack(int frameCount)
		{
#if _WIN64
			DWORD machine = IMAGE_FILE_MACHINE_AMD64;
#else
			DWORD machine = IMAGE_FILE_MACHINE_I386;
#endif

			static HANDLE process = nullptr;
			if (!process)
			{
				process = GetCurrentProcess();
			}

			HANDLE thread = nullptr;
			if (!thread)
			{
				thread = GetCurrentThread();
			}

			if (SymInitialize(process, NULL, FALSE) == FALSE)
			{
				DWORD error = GetLastError();
				IS_LOG_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to call SymInitialize. Error: {}", error);
				return std::vector<std::string>();
			}

			SymSetOptions(SYMOPT_LOAD_LINES);

			CONTEXT    context = {};
			context.ContextFlags = CONTEXT_FULL;
			RtlCaptureContext(&context);

#if _WIN64
			STACKFRAME frame = {};
			frame.AddrPC.Offset = context.Rip;
			frame.AddrPC.Mode = AddrModeFlat;
			frame.AddrFrame.Offset = context.Rbp;
			frame.AddrFrame.Mode = AddrModeFlat;
			frame.AddrStack.Offset = context.Rsp;
			frame.AddrStack.Mode = AddrModeFlat;
#else
			STACKFRAME frame = {};
			frame.AddrPC.Offset = context.Eip;
			frame.AddrPC.Mode = AddrModeFlat;
			frame.AddrFrame.Offset = context.Ebp;
			frame.AddrFrame.Mode = AddrModeFlat;
			frame.AddrStack.Offset = context.Esp;
			frame.AddrStack.Mode = AddrModeFlat;
#endif

			bool first = true;

			std::vector<std::string> frames;
			while (StackWalk(machine, process, thread, &frame, &context, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL))
			{
				if (frames.size() == ++frameCount)
				{
					break;
				}

				StackFrame f = {};
				f.address = frame.AddrPC.Offset;

#if _WIN64
				DWORD64 moduleBase = 0;
#else
				DWORD moduleBase = 0;
#endif

				moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);

				char moduelBuff[MAX_PATH];
				if (moduleBase && GetModuleFileNameA((HINSTANCE)moduleBase, moduelBuff, MAX_PATH))
				{
					f.module = basename(moduelBuff);
				}
				else
				{
					f.module = "Unknown Module";
				}
#if _WIN64
				DWORD64 offset = 0;
#else
				DWORD offset = 0;
#endif

				const int symbolBufferSize = sizeof(IMAGEHLP_SYMBOL) + 2047;
				char symbolBuffer[symbolBufferSize];
				PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)symbolBuffer;
				symbol->SizeOfStruct = symbolBufferSize;
				symbol->MaxNameLength = 255;

				if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol))
				{
					f.name = symbol->Name;
				}
				else
				{
					DWORD error = GetLastError();
					///IS_LOG_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to resolve address {}: {}\n", frame.AddrPC.Offset, error);
					f.name = "Unknown Function";
				}

				IMAGEHLP_LINE line;
				line.SizeOfStruct = sizeof(IMAGEHLP_LINE) + 1;

				DWORD offset_ln = 0;
				if (SymGetLineFromAddr(process, frame.AddrPC.Offset, &offset_ln, &line))
				{
					f.file = line.FileName;
					f.line = line.LineNumber;
				}
				else
				{
					DWORD error = GetLastError();
					///IS_LOG_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to resolve line for: {0:x}, {}\n", frame.AddrPC.Offset, error);
					f.line = 0;
				}

				if (!first)
				{
					frames.push_back(f.name);
				}
				first = false;
			}
			SymCleanup(process);

			return frames;
		}

		std::string PlatformWindows::StringFromWString(const std::wstring& wstring)
		{
			if (wstring.empty()) return std::string();
			int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstring[0], static_cast<int>(wstring.size()), NULL, 0, NULL, NULL);
			std::string strTo(size_needed, 0);
			WideCharToMultiByte(CP_UTF8, 0, &wstring[0], static_cast<int>(wstring.size()), &strTo[0], size_needed, NULL, NULL);
			return strTo;
		}

		std::wstring PlatformWindows::WStringFromString(const std::string& string)
		{
			if (string.empty()) return std::wstring();
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), &wstrTo[0], size_needed);
			return wstrTo;
		}

		std::wstring PlatformWindows::WStringFromStringView(std::string_view string)
		{
			if (string.empty()) return std::wstring();
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &string[0], static_cast<int>(string.size()), &wstrTo[0], size_needed);
			return wstrTo;
		}

		std::string PlatformWindows::GetExecuteablePath()
		{
			char path[MAX_PATH];
			GetModuleFileNameA(NULL, path, MAX_PATH);
			std::string sString(path);
			FileSystem::PathToUnix(sString);
			sString = sString.substr(0, sString.find_last_of('/'));
			return sString;
		}

		std::string PlatformWindows::GetExecuteableName()
		{
			char path[MAX_PATH];
			GetModuleFileNameA(NULL, path, MAX_PATH);
			std::string sString(path);
			FileSystem::PathToUnix(sString);
			sString = sString.substr(sString.find_last_of('/') + 1);
			return sString;
		}

		Core::GUID PlatformWindows::CreateGUID()
		{
			Core::GUID guid;
			AssignGUID(guid);
			return guid;
		}

		void PlatformWindows::AssignGUID(Core::GUID& guid)
		{
			_GUID gidReference;
			HRESULT hCreateGuid = CoCreateGuid(&gidReference);
			guid = Core::GUID(gidReference.Data1, gidReference.Data2, gidReference.Data3, gidReference.Data4);
		}

		bool PlatformWindows::IsMainThread()
		{
			return GetCurrentThreadId() == s_mainThreadId;
		}

		bool PlatformWindows::IsDebuggerAttached()
		{
			return IsDebuggerPresent();
		}

		Core::CPUInformation PlatformWindows::GetCPUInformation()
		{
			if (!s_cpuInformation.Initialised)
			{
				const u32 MAX_INTEL_TOP_LVL = 4;

				// Bit positions for data extractions
				const u32 SSE_POS = 0x02000000;
				const u32 SSE2_POS = 0x04000000;
				const u32 SSE3_POS = 0x00000001;
				const u32 SSE41_POS = 0x00080000;
				const u32 SSE42_POS = 0x00100000;
				const u32 AVX_POS = 0x10000000;
				const u32 AVX2_POS = 0x00000020;
				const u32 LVL_NUM = 0x000000FF;
				const u32 LVL_TYPE = 0x0000FF00;
				const u32 LVL_CORES = 0x0000FFFF;

				// Get vendor name EAX=0
				CPUID cpuID0(0, 0);

				u32 HFS = cpuID0.EAX();
				s_cpuInformation.Vendor += std::string((const char*)&cpuID0.EBX(), 4);
				s_cpuInformation.Vendor += std::string((const char*)&cpuID0.EDX(), 4);
				s_cpuInformation.Vendor += std::string((const char*)&cpuID0.ECX(), 4);

				// Get SSE instructions availability
				CPUID cpuID1(1, 0);
				s_cpuInformation.IsHyperThreaded = cpuID1.EDX()  & AVX_POS;
				s_cpuInformation.IsSSE   = cpuID1.EDX() & SSE_POS;
				s_cpuInformation.IsSSE2  = cpuID1.EDX() & SSE2_POS;
				s_cpuInformation.IsSSE3  = cpuID1.ECX() & SSE3_POS;
				s_cpuInformation.IsSSE41 = cpuID1.ECX() & SSE41_POS;
				s_cpuInformation.IsSSE42 = cpuID1.ECX() & SSE41_POS;
				s_cpuInformation.IsAVX   = cpuID1.ECX() & AVX_POS;

				// Get AVX2 instructions availability
				CPUID cpuID7(7, 0);
				s_cpuInformation.IsAVX2 = cpuID7.EBX()  & AVX2_POS;

				std::string upVId = s_cpuInformation.Vendor;
				std::for_each(upVId.begin(), upVId.end(), [](char& in) { in = ::toupper(in); });
				int mNumSMT = 0;
				// Get num of cores
				if (upVId.find("INTEL") != std::string::npos)
				{
					if (HFS >= 11) 
					{
						for (int lvl = 0; lvl < MAX_INTEL_TOP_LVL; ++lvl) 
						{
							CPUID cpuID4(0x0B, lvl);
							uint32_t currLevel = (LVL_TYPE & cpuID4.ECX()) >> 8;
							switch (currLevel) {
							case 0x01: mNumSMT					= LVL_CORES & cpuID4.EBX(); break;
							case 0x02: s_cpuInformation.LogicalCoreCount	= LVL_CORES & cpuID4.EBX(); break;
							default: break;
							}
						}
						s_cpuInformation.PhysicalCoreCount = s_cpuInformation.LogicalCoreCount / mNumSMT;
					}
					else 
					{
						if (HFS >= 1) 
						{
							s_cpuInformation.LogicalCoreCount = (cpuID1.EBX()  >> 16) & 0xFF;
							if (HFS >= 4) 
							{
								s_cpuInformation.PhysicalCoreCount = 1 + (CPUID(4, 0).EAX() >> 26) & 0x3F;
							}
						}
						if (s_cpuInformation.IsHyperThreaded)
						{
							if (!(s_cpuInformation.PhysicalCoreCount > 1))
							{
								s_cpuInformation.PhysicalCoreCount = 1;
								s_cpuInformation.LogicalCoreCount = (s_cpuInformation.LogicalCoreCount >= 2 ? s_cpuInformation.LogicalCoreCount : 2);
							}
						}
						else 
						{
							s_cpuInformation.PhysicalCoreCount = s_cpuInformation.LogicalCoreCount = 1;
						}
					}
				}
				else if (upVId.find("AMD") != std::string::npos) 
				{
					if (HFS >= 1) 
					{
						s_cpuInformation.LogicalCoreCount = (cpuID1.EBX() >> 16) & 0xFF;
						if (CPUID(0x80000000, 0).EAX() >= 8)
						{
							s_cpuInformation.PhysicalCoreCount = 1 + (CPUID(0x80000008, 0).ECX() & 0xFF);
						}
					}

					if (s_cpuInformation.IsHyperThreaded)
					{
						if (!(s_cpuInformation.PhysicalCoreCount > 1)) 
						{
							s_cpuInformation.PhysicalCoreCount = 1;
							s_cpuInformation.LogicalCoreCount = (s_cpuInformation.LogicalCoreCount >= 2 ? s_cpuInformation.LogicalCoreCount : 2);
						}
						else
						{
							s_cpuInformation.PhysicalCoreCount = s_cpuInformation.LogicalCoreCount / 2;
						}
					}
					else
					{
						s_cpuInformation.PhysicalCoreCount = s_cpuInformation.LogicalCoreCount = 1;
					}
				}
				else 
				{
					IS_LOG_CORE_INFO("Unexpected vendor id.");
				}

				if (cpuID0.EAX() >= 0x16)
				{
					CPUID cpuID16(0x16, 0);

				}
				else
				{
					std::vector<PROCESSOR_POWER_INFORMATION> processInformation;
					processInformation.resize(s_cpuInformation.LogicalCoreCount);
					LONG result = CallNtPowerInformation(ProcessorInformation, NULL, 0, processInformation.data(), static_cast<ULONG>(processInformation.size() * sizeof(PROCESSOR_POWER_INFORMATION)));
					if (result == 0)
					{
						s_cpuInformation.SpeedInMHz = static_cast<float>(processInformation.at(0).MaxMhz);
					}
				}

				// Get processor brand string
				// This seems to be working for both Intel & AMD vendors
				for (int i = 0x80000002; i < 0x80000005; ++i) 
				{
					CPUID cpuID(i, 0);
					s_cpuInformation.Model += std::string((const char*)&cpuID.EAX(), 4);
					s_cpuInformation.Model += std::string((const char*)&cpuID.EBX(), 4);
					s_cpuInformation.Model += std::string((const char*)&cpuID.ECX(), 4);
					s_cpuInformation.Model += std::string((const char*)&cpuID.EDX(), 4);
				}

				// Clean up the model name. Getting the model name can have trailing spaces.
				// So we clean that up to remove unnecessary spaces.
				if (s_cpuInformation.Model.size() > 0)
				{
					while (s_cpuInformation.Model.back() == ' '
						|| s_cpuInformation.Model.back() == '\0')
					{
						s_cpuInformation.Model.pop_back();
					}
					s_cpuInformation.Model.push_back('\0');
				}

				SYSTEM_INFO systemInfo;
				GetSystemInfo(&systemInfo);

				switch (systemInfo.wProcessorArchitecture)
				{
					case 9:
					{
						s_cpuInformation.CPUArchitecture = Core::CPUArchitectures::AMD64;
						break;
					}
					case 5:
					{
						s_cpuInformation.CPUArchitecture = Core::CPUArchitectures::ARM;
						break;
					}
					case 12:
					{
						s_cpuInformation.CPUArchitecture = Core::CPUArchitectures::ARM64;
						break;
					}
					case 6:
					{
						s_cpuInformation.CPUArchitecture = Core::CPUArchitectures::IA64;
						break;
					}
					case 0:
					{
						s_cpuInformation.CPUArchitecture = Core::CPUArchitectures::INTEL;
						break;

					}
				}
				s_cpuInformation.Initialised = true;
			}
			Core::CPUInformation info = s_cpuInformation;

#ifdef AMD_Ryzen_Master_SDK
			const bool isUserAnAdmin = IsUserAnAdmin();
			const unsigned int OSVersion = GetOSVersion();
			const bool AuthenticAMD = info.Vendor == "AuthenticAMD";
			if (false
				&& isUserAnAdmin
				&& OSVersion >= 10
				&& AuthenticAMD)
			{
				bool ryzenMasterSDKDriver = AMDQueryDrvService() == 0;
				if (!ryzenMasterSDKDriver)
				{
					ryzenMasterSDKDriver = AMDInstallDriver();
				}

				if (ryzenMasterSDKDriver
					&& AMDIsSupportedProcessor())
				{
					IPlatform& rPlatform = GetPlatform();
					bool bRetCode = rPlatform.Init();
					if (bRetCode)
					{
						IDeviceManager& rDeviceManager = rPlatform.GetIDeviceManager();
						ICPUEx* obj = (ICPUEx*)rDeviceManager.GetDevice(dtCPU, 0);
						if (obj)
						{
							CPUParameters cpuParameters;
							if (obj->GetCPUParameters(cpuParameters) == 0)
							{
								info.SpeedInMHz = static_cast<float>(cpuParameters.dPeakSpeed);
							}
						}
					}
				}
			}
#else
#endif
			return info;
		}

		Core::MemoryInformation PlatformWindows::GetMemoryInformation()
		{
			u64 const currentTimeStamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock().now().time_since_epoch()).count();
			u64 const deltaFromLastUpdate = currentTimeStamp - s_memoryInformation.LastUpdateTimeStamp;
			if (s_memoryInformation.Initialised && deltaFromLastUpdate < 10)
			{
				// Update memory usage every 10 seconds.
				return s_memoryInformation;
			}
			s_memoryInformation.LastUpdateTimeStamp = currentTimeStamp;

			// Get the general state of the memory of the machine.
			MEMORYSTATUSEX memoryStatus;
			memoryStatus.dwLength = sizeof(memoryStatus);
			GlobalMemoryStatusEx(&memoryStatus);

			s_memoryInformation.TotalPhyscialMemoryBytes = static_cast<u64>(memoryStatus.ullTotalPhys);
			s_memoryInformation.TotalVirtualMemoryBytes = static_cast<u64>(memoryStatus.ullTotalVirtual);

			s_memoryInformation.TotalPhyscialMemoryFreeBytes = static_cast<u64>(memoryStatus.ullAvailPhys);
			s_memoryInformation.TotalVirtualMemoryFreeBytes = static_cast<u64>(memoryStatus.ullAvailVirtual);

			s_memoryInformation.TotalPhyscialMemoryUsedBytes = s_memoryInformation.TotalPhyscialMemoryBytes - s_memoryInformation.TotalPhyscialMemoryFreeBytes;
			s_memoryInformation.TotalPhyscialMemoryUsedBytes = s_memoryInformation.TotalVirtualMemoryBytes - s_memoryInformation.TotalVirtualMemoryFreeBytes;

			PROCESS_MEMORY_COUNTERS processMemoryCounters = {};
			if (s_memoryReadProcessHandle
				&& GetProcessMemoryInfo(s_memoryReadProcessHandle, &processMemoryCounters, sizeof(processMemoryCounters)))
			{
				s_memoryInformation.ProcessMemoryUsageBytes = static_cast<u64>(processMemoryCounters.WorkingSetSize);
			}

			_PERFORMANCE_INFORMATION performanceInfo = {};
			if (GetPerformanceInfo(&performanceInfo, sizeof(performanceInfo)))
			{
				s_memoryInformation.ProcessNumOfPagesCommitted = static_cast<u64>(performanceInfo.CommitTotal);
				s_memoryInformation.PageSizeBytes = static_cast<u64>(performanceInfo.PageSize);
			}

			s_memoryInformation.Initialised = true;
			return s_memoryInformation;
		}

		void* PlatformWindows::LoadDynamicLibrary(std::string_view path)
		{
			return ::LoadLibraryA(path.data());
		}

		void PlatformWindows::FreeDynamicLibrary(void*& library)
		{
			if (library)
			{
				::FreeLibrary((HMODULE)library);
				library = nullptr;
			}
		}

		void* PlatformWindows::GetDynamicFunctionVoid(void* library, const char* functionName)
		{
			if (library == nullptr || (!functionName || strlen(functionName) == 0))
			{
				return nullptr;
			}
			void* procAddress = GetProcAddress(reinterpret_cast<HMODULE>(library), functionName);
			return procAddress;
		}

		unsigned int PlatformWindows::GetOSVersion()
		{
			DWORD major = 0;
			DWORD minor = 0;
			LPBYTE pinfoRawData;
			if (IsWindowsServer())
			{
				return false;
			}
			if (0 == NetWkstaGetInfo(NULL, 100, &pinfoRawData))
			{
				WKSTA_INFO_100* pworkstationInfo = (WKSTA_INFO_100*)pinfoRawData;
				major = pworkstationInfo->wki100_ver_major;
				minor = pworkstationInfo->wki100_ver_minor;
				::NetApiBufferFree(pinfoRawData);
			}
			return major;
		}

		bool PlatformWindows::GetDriverPath(wchar_t* pDriverPath, const wchar_t* driverFilePath64)
		{
#define LOG_PROCESS_ERROR(__CONDITION__)		\
	do											\
	{											\
		if (!(__CONDITION__))					\
		{										\
			goto Exit0;							\
		}										\
	} while (false)

			wchar_t* pTemp = _wgetenv(L"AMDRMMONITORSDKPATH");
			wchar_t driverPath[200] = { '\0' };
			size_t iDriverPathLength = 0;

			LOG_PROCESS_ERROR(pTemp);

			iDriverPathLength = wcslen(pTemp);
			wcsncpy(driverPath, pTemp, iDriverPathLength);
			driverPath[iDriverPathLength] = '\0';

			wsprintf(pDriverPath, L"%s%s", driverPath, driverFilePath64);
			return true;
		Exit0:
			return false;

#undef LOG_PROCESS_ERROR
		}

#ifdef AMD_Ryzen_Master_SDK
#define RM_SER_NAME L##"AMDRyzenMasterDriverV22"
#define AOD_DRIVER_NAME		L"AMDRyzenMasterDriverV22"
#define DRIVER_FILE_PATH_64	L"bin\\AMDRyzenMasterDriver.sys"

		int PlatformWindows::AMDQueryDrvService()
		{
			SERVICE_STATUS ServiceStatus;
			SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
			if (!hSCM)
				return -1;

			SC_HANDLE hOpenService = OpenService(hSCM, RM_SER_NAME, SC_MANAGER_ALL_ACCESS);
			if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
			{
				CloseServiceHandle(hOpenService);
				CloseServiceHandle(hSCM);
				return   -1;
			}
			QueryServiceStatus(hOpenService, &ServiceStatus);
			if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			{
				CloseServiceHandle(hOpenService);
				CloseServiceHandle(hSCM);
				return -1;
			}

			CloseServiceHandle(hOpenService);
			CloseServiceHandle(hSCM);
			return 0;
		}

		bool PlatformWindows::AMDInstallDriver()
		{
#define LOG_PROCESS_ERROR(__CONDITION__)		\
	do											\
	{											\
		if (!(__CONDITION__))					\
		{										\
			goto Exit0;							\
		}										\
	} while (false)

			bool bRetCode = false;
			bool bResult = false;
			DWORD dwLastError;
			SC_HANDLE hSCManager = NULL;
			SC_HANDLE hService = NULL;
			wchar_t szDriverPath[256];


			HANDLE m_hDriver = CreateFile(L"\\\\.\\" AOD_DRIVER_NAME,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL);

			if (m_hDriver == INVALID_HANDLE_VALUE)
			{
				bRetCode = GetDriverPath(szDriverPath, DRIVER_FILE_PATH_64);
				LOG_PROCESS_ERROR(bRetCode);

				hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
				LOG_PROCESS_ERROR(hSCManager);

				// Install the driver
				hService = CreateService(hSCManager,
					AOD_DRIVER_NAME, AOD_DRIVER_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
					SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szDriverPath,
					NULL, NULL, NULL, NULL, NULL);
				if (hService == NULL)
				{
					dwLastError = GetLastError();
					if (dwLastError == ERROR_SERVICE_EXISTS)
						hService = OpenService(hSCManager, AOD_DRIVER_NAME, SERVICE_ALL_ACCESS);
					else if (dwLastError == ERROR_SERVICE_MARKED_FOR_DELETE)
					{
						hService = OpenService(hSCManager, AOD_DRIVER_NAME, SERVICE_ALL_ACCESS);
						SERVICE_STATUS ServiceStatus;
						ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);

						hService = CreateService(hSCManager,
							AOD_DRIVER_NAME, AOD_DRIVER_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
							SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, szDriverPath,
							NULL, NULL, NULL, NULL, NULL);
					}
					else
						printf("InstallDriver: error code returned from CreateService is: %d", GetLastError());
				}
				LOG_PROCESS_ERROR(hService);

				// Start the driver
				BOOL bRet = StartService(hService, 0, NULL);
				if (!bRet)
				{
					dwLastError = GetLastError();
					if (dwLastError == ERROR_PATH_NOT_FOUND)
					{
						bRet = DeleteService(hService);
						LOG_PROCESS_ERROR(bRet);

						CloseServiceHandle(hService);

						hService = CreateService(hSCManager,
							AOD_DRIVER_NAME, AOD_DRIVER_NAME, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
							SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, szDriverPath,
							NULL, NULL, NULL, NULL, NULL);
						LOG_PROCESS_ERROR(hService);

						bRet = StartService(hService, 0, NULL);
						LOG_PROCESS_ERROR(bRet);
					}

					if (dwLastError != ERROR_SERVICE_ALREADY_RUNNING)
					{
						LOG_PROCESS_ERROR(bRet);
					}
				}

				// Try to create the file again
				m_hDriver = CreateFile(L"\\\\.\\" AOD_DRIVER_NAME,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					NULL);
				LOG_PROCESS_ERROR(m_hDriver != INVALID_HANDLE_VALUE);
			}


			bResult = true;

		Exit0:
			if (m_hDriver != INVALID_HANDLE_VALUE)
				CloseHandle(m_hDriver);

			if (hSCManager)
				CloseServiceHandle(hSCManager);
			if (hService)
				CloseServiceHandle(hService);


			return bResult;

#undef LOG_PROCESS_ERROR
		}

		bool PlatformWindows::AMDIsSupportedProcessor()
		{
			enum CPU_PackageType
			{
				cptFP5 = 0,
				cptAM5 = 0,
				cptFP7 = 1,
				cptFL1 = 1,
				cptFP8 = 1,
				cptAM4 = 2,
				cptFP7r2 = 2,
				cptAM5_B0 = 3,
				cptSP3 = 4,
				cptFP7_B0 = 4,
				cptFP7R2_B0 = 5,
				cptSP3r2 = 7,
				cptSP6 = 8,
				cptUnknown = 0xF
			};

			bool retBool = false;
			int CPUInfo[4] = { -1 };
			__cpuid(CPUInfo, 0x80000001);
			unsigned long uCPUID = CPUInfo[0];
			CPU_PackageType pkgType = (CPU_PackageType)((CPUInfo[1] >> 28) & 0x0F);

			switch (pkgType)
			{
			case cptFP5:
				//case cptAM5:
				switch (uCPUID)
				{
				case 0x00810F80:
				case 0x00810F81:
				case 0x00860F00:
				case 0x00860F01:
				case 0x00A50F00:
				case 0x00A50F01:
				case 0x00860F81:
				case 0x00A60F00:
				case 0x00A60F01:
				case 0x00A60F10:
				case 0x00A60F11:
				case 0x00A60F12:
				case 0x00A70F80:
				case 0x00A70F52:
					retBool = true;
					break;
				default:
					break;
				}
				break;

			case cptAM4:
			case cptFP7R2_B0:
				switch (uCPUID)
				{
				case 0x00800F00:
				case 0x00800F10:
				case 0x00800F11:
				case 0x00800F12:

				case 0x00810F10:
				case 0x00810F11:

				case 0x00800F82:
				case 0x00800F83:

				case 0x00870F00:
				case 0x00870F10:

				case 0x00810F80:
				case 0x00810F81:

				case 0x00860F00:
				case 0x00860F01:

				case 0x00A20F00:
				case 0x00A20F10:
				case 0x00A20F12:

				case 0x00A50F00:
				case 0x00A50F01:

					//cptFP7r2
				case 0x00A40F00:
				case 0x00A40F40:
				case 0x00A40F41:

				case 0x00A70F00:
				case 0x00A70F40:
				case 0x00A70F41:
				case 0x00A70F42:
				case 0x00A70F80:

				case 0x00A70F52:
				case 0x00A70FC0:
					retBool = true;
					break;
				default:
					break;
				}
				break;

			case cptSP3r2:
				switch (uCPUID)
				{
				case 0x00800F10:
				case 0x00800F11:
				case 0x00800F12:

				case 0x00800F82:
				case 0x00800F83:
				case 0x00830F00:
				case 0x00830F10:
					retBool = true;
					break;
				default:
					break;
				}
				break;

			case cptFP7:
				//case cptFL1:
				//case cptFP8:
				//case cptFP7_B0:
			case cptSP3:
				switch (uCPUID)
				{
				case 0x00A40F00:
				case 0x00A40F40:
				case 0x00A40F41:

				case 0x00A60F11:
				case 0x00A60F12:
					retBool = true;
					break;
				case 0x00A00F80:
				case 0x00A00F82:

				case 0x00A70F00:
				case 0x00A70F40:
				case 0x00A70F41:
				case 0x00A70F42:
				case 0x00A70F80:
				case 0x00A70F52:
				case 0x00A70FC0:
					retBool = true;
					break;
				default:
					break;
				}
				break;

			case cptSP6:
				switch (uCPUID)
				{
				case 0x00A10F81:
				case 0x00A10F80:
					retBool = true;
					break;
				}
				break;

			default:
				break;
			}
			return retBool;
		}

#undef RM_SER_NAME
#undef AOD_DRIVER_NAME
#undef DRIVER_FILE_PATH_64
#endif
	}
}
#endif /// IS_PLATFORM_WINDOWS