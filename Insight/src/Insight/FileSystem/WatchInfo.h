#pragma once
#include "Insight/Core/Core.h"
#include <deque>

namespace Insight
{
	namespace FileSystem
	{
		struct HandleDeleter
		{
			void operator()(HANDLE handle)
			{
				if (handle != INVALID_HANDLE_VALUE && handle != NULL)
				{
					CloseHandle(handle);
				}
			}
		};
		using HandlePtr = std::unique_ptr<std::remove_pointer<HANDLE>::type, HandleDeleter>;
		using FileNotifcationQueue = std::vector<std::pair<std::wstring, uint32_t>>;

		// Information about a subscription for the changes of a single directory.
		class WatchInfo
		{
		private:
			std::unique_ptr<OVERLAPPED> overlapped;
			// Stores FILE_NOTIFY_INFORMATION, that is written by RDC() asynchronously.
			// Has to be destructed after the directory handle is closed!
			// RDC() requires a "pointer to the DWORD-aligned formatted buffer."
			std::aligned_storage_t<64 * 1024, sizeof(DWORD)> notifBuffer;
			static_assert(sizeof(WatchInfo::notifBuffer) <= 64 * 1024, "Must be smaller than RDC()'s network limit!");
			static_assert(sizeof(WatchInfo::notifBuffer) >= (sizeof(FILE_NOTIFY_INFORMATION) + (1 << 15)), "Must be able to store a long path.");

			std::wstring path;
			HandlePtr directory;

			void ProcessNotification(const FILE_NOTIFY_INFORMATION& notIf, FileNotifcationQueue& notifications) const;
			// The FileName might be in the short 8.3 form, so we try to get the long form.
			// Similar solution to libuv's.
			std::wstring TryToGetLongName(const std::wstring& pathName) const;

		public:
			const U64 rId;
			enum class State : uint8_t
			{
				Initialized, // No outstanding RDC() call.
				Listening, // RDC() call was made, and we're waiting for changes.
				PendingClose, // Directory handle was closed, and we're waiting for the "closing"
							  // notification on IOCP.
							  // Most of the time this is an "empty" notification, but sometimes it is a
							  // legitimate notification about a change. This is behavior is not documented
							  // explicitly.
			};
			std::atomic<State> state;

			WatchInfo(const U64& rId, std::unique_ptr<OVERLAPPED>&& overlapped,
				const std::wstring& path, HandlePtr&& dirHandle);
			WatchInfo(const WatchInfo&) = delete;
			WatchInfo& operator=(const WatchInfo&) = delete;
			~WatchInfo();

			bool Listen();
			void Stop();
			bool CanRun() const
			{
				return this->state != State::PendingClose;
			}

			FileNotifcationQueue ProcessNotifications() const;
		};
	}
}