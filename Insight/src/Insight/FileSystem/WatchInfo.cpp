#include "ispch.h"
#include "WatchInfo.h"

namespace Insight
{
	namespace FileSystem
	{
		void WatchInfo::ProcessNotification(const FILE_NOTIFY_INFORMATION& notIf, FileNotifcationQueue& notifications) const
		{
			std::wstring wPathName(
				notIf.FileName, notIf.FileName + (notIf.FileNameLength / sizeof(notIf.FileName)));
			if (notIf.Action != FILE_ACTION_REMOVED && notIf.Action != FILE_ACTION_RENAMED_OLD_NAME) {
				std::wstring longName = this->TryToGetLongName(wPathName);
				if (longName != wPathName) {
					wPathName = longName;
				}
			}
			notifications.emplace_back(wPathName, notIf.Action);
		}

		std::wstring WatchInfo::TryToGetLongName(const std::wstring& pathName) const
		{
			const std::wstring fullPath = this->path + L"\\" + pathName;
			const DWORD longSize = GetLongPathNameW(fullPath.c_str(), NULL, 0);
			std::wstring longPathName;
			longPathName.resize(longSize);
			const DWORD retVal = GetLongPathNameW(
				fullPath.c_str(), longPathName.data(), static_cast<DWORD>(longPathName.size()));
			if (retVal == 0) {
				return pathName;
			}
			while (!longPathName.empty() && longPathName.back() == L'\0') {
				longPathName.pop_back();
			}
			if (longPathName.find(this->path) == 0 && this->path.size() < longPathName.size()) {
				std::wstring longName = longPathName.substr(this->path.size() + 1);
				if (longName.empty()) {
					return pathName;
				}
				else {
					return longName;
				}
			}
			else {
				return pathName;
			}
		}

		WatchInfo::WatchInfo(const U64& rId, std::unique_ptr<OVERLAPPED>&& overlapped,
			const std::wstring& path, HandlePtr&& dirHandle)
			: overlapped(std::move(overlapped))
			, notifBuffer()
			, path(path)
			, directory(std::move(dirHandle))
			, rId(rId)
			, state(State::Initialized)
		{
		}

		WatchInfo::~WatchInfo()
		{
			if (this->state == State::Listening) {
				std::cerr << "Destructing a listening WatchInfo" << std::endl;
			}
		}

		bool WatchInfo::Listen()
		{
			if (this->state != State::Initialized) {
				std::cerr << "Invalid state." << std::endl;
				return false;
			}
			constexpr DWORD flags = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME
				| FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION
				| FILE_NOTIFY_CHANGE_SECURITY;
			const BOOL res = ReadDirectoryChangesW(this->directory.get(), &this->notifBuffer,
				static_cast<DWORD>(sizeof(this->notifBuffer)), true /* bWatchSubtree */, flags,
				nullptr /* lpBytesReturned */, this->overlapped.get(), nullptr /* lpCompletionRoutine */);
			if (res == FALSE) {
				this->state = State::Initialized;
				std::cerr << "An error has occurred: " << GetLastError() << std::endl;
				return false;
			}
			this->state = State::Listening;
			return true;
		}

		void WatchInfo::Stop()
		{
			if (state == State::Listening) {
				state = State::PendingClose;
			}
			directory.reset(INVALID_HANDLE_VALUE);
		}

		FileNotifcationQueue WatchInfo::ProcessNotifications() const
		{
			FileNotifcationQueue notifications;

			auto notInf = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(&notifBuffer);
			for (bool moreNotif = true; moreNotif; moreNotif = notInf->NextEntryOffset > 0,
				notInf = reinterpret_cast<const FILE_NOTIFY_INFORMATION*>(
					reinterpret_cast<const char*>(notInf) + notInf->NextEntryOffset))
			{
				ProcessNotification(*notInf, notifications);
			}

			return std::move(notifications);
		}
	}
}