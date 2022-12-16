#include "Platforms/Windows/PlatformWindows.h"

#ifdef IS_PLATFORM_WINDOWS

#include "Core/Logger.h"

#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>

#include <rpc.h>
#include <Objbase.h>

namespace Insight
{
	namespace Windows
	{
		std::thread::id PlatformWindows::s_mainThreadId;

		void PlatformWindows::Initialise()
		{
			s_mainThreadId = std::this_thread::get_id();
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
				IS_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to call SymInitialize. Error: {}", error);
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
					///IS_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to resolve address {}: {}\n", frame.AddrPC.Offset, error);
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
					///IS_CORE_ERROR("[PlatformWindows::GetCallStack] Failed to resolve line for: {0:x}, {}\n", frame.AddrPC.Offset, error);
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
			guid = Core::GUID(gidReference.Data1, gidReference.Data2, gidReference.Data3, 
				{ gidReference.Data4[0], gidReference.Data4[1], gidReference.Data4[2], gidReference.Data4[3],
				 gidReference.Data4[4], gidReference.Data4[5], gidReference.Data4[6], gidReference.Data4[7] });
		}

		bool PlatformWindows::IsMainThread()
		{
			return std::this_thread::get_id() == s_mainThreadId;
		}
	}
}
#endif /// IS_PLATFORM_WINDOWS