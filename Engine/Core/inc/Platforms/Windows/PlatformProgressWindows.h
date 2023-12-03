#pragma once

#ifdef IS_PLATFORM_WINDOWS
#include "Core/Defines.h"

#include <string>

namespace Insight
{
    namespace Platforms::Windows
    {
        class IS_CORE PlatformProgressWindows
        {
        public:
            PlatformProgressWindows();
            ~PlatformProgressWindows();

            void Show(std::string title);
            void Close();
            void UpdateProgress(const float progress, std::string message = "");

        private:
            void* m_windowHandle = nullptr;
            void* m_progressBarHandle = nullptr;
            const wchar_t* m_windowClassName = L"ProgressBar";
        };
    }
}
#endif