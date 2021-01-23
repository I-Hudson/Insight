// Copyright (c) 2012-2021 Wojciech Figat. All rights reserved.

#pragma once

#if IS_PLATFORM_WINDOWS

class WindowsClipboard;
typedef WindowsClipboard Clipboard;
class Win32CriticalSection;
typedef Win32CriticalSection CriticalSection;
class Win32ConditionVariable;
typedef Win32ConditionVariable ConditionVariable;
class WindowsFileSystem;
typedef WindowsFileSystem FileSystem;
class WindowsFileSystemWatcher;
typedef WindowsFileSystemWatcher FileSystemWatcher;
class Win32File;
typedef Win32File File;
class WindowsPlatform;
typedef WindowsPlatform Platform;
class Win32Thread;
typedef Win32Thread Thread;
class WindowsWindow;
typedef WindowsWindow Window;

#else

#error Missing Types implementation!

#endif
