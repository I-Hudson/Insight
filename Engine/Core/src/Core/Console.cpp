#include "Core/Console.h"

#include "Core/StringUtils.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <../imgui/misc/cpp/imgui_stdlib.h>

namespace Insight
{
    namespace Core
    {
        std::unordered_map<std::string, ConsoleValue> Console::m_values;

        Console::Console()
        { }

        Console::~Console()
        { }

        void Console::SetValue(const std::string& key, const std::string& value)
        {
            m_values[key] = ConsoleValue{ key, value };
        }

        const ConsoleValue& Console::GetValue(const std::string& key) const
        {
            if (auto iter = m_values.find(key);
                iter != m_values.end())
            {
                return iter->second;
            }
            return m_emptyConsoleValue;
        }

        void Console::Message(const std::string_view message, const ConsoleLogLevel logLevel)
        {
            ConsoleMessageColour colour;
            colour.r = 0;
            colour.g = 0;
            colour.b = 0;
            colour.a = 0;
            Message(message, logLevel, colour);
        }

        void Console::Message(const std::string_view message, const ConsoleLogLevel logLevel, const ConsoleMessageColour colour)
        {
            ConsoleMessage consoleMessage;
            consoleMessage.Message = message;
            consoleMessage.Colour = colour;
            consoleMessage.Level = logLevel;

            while (!consoleMessage.Message.empty() 
                && consoleMessage.Message.back() == '\n')
            {
                consoleMessage.Message.pop_back();
            }

            std::lock_guard l(m_messagesMutex);
            m_consoleMessages[m_consoleMessageLastIndex] = consoleMessage;

            // Wrap the start index back to 0 if required otherwise just increment by one.
            m_consoleMessageLastIndex = (m_consoleMessageLastIndex + 1) % c_ConsoleMessageSize;
        }

        Console::ConsoleMessageColour Console::GetMesageColour(const ConsoleMessage& message) const
        {
            if (message.Colour.IsSet())
            {
                return message.Colour;
            }

            switch (message.Level)
            {
            case ConsoleLogLevel::Trace:
            case ConsoleLogLevel::Debug:
            case ConsoleLogLevel::Info: return Console::ConsoleMessageColour(1.0f, 1.0f, 1.0f, 1.0f);

            case ConsoleLogLevel::Warn: return Console::ConsoleMessageColour(0.80f, 0.98f, 0.0f, 1.0f);

            case ConsoleLogLevel::Error:
            case ConsoleLogLevel::Critical: return Console::ConsoleMessageColour(1.0f, 0.0f, 0.0f, 1.0f);
            }

            return Console::ConsoleMessageColour(1, 1, 1, 1);
        }

        void Console::Show(const bool shouldShow)
        {
            m_isShowing = shouldShow;
            m_windowDiffStateLastFrame = true;
        }

        bool Console::IsShowing() const
        {
            return m_isShowing;
        }

        void Console::Render(const u32 x, const u32 y, const u32 width, const u32 height)
        {
#ifndef IMGUI_DISABLE
            if (!m_isShowing)
            {
                return;
            }

            static std::string inputText;

            const ImVec2 windowPos = ImVec2(static_cast<float>(x), static_cast<float>(y));
            const ImVec2 windowSize = ImVec2(static_cast<float>(width), static_cast<float>(height));

            ImGui::SetNextWindowPos(windowPos);
            ImGui::SetNextWindowSize(windowSize);
            const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration;

            const ImVec4 consoleBackgroundColour(65.0f / 255.0f, 65.0f / 255.0f, 65 / 255.0f, 1.0);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, consoleBackgroundColour);
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

            const char* c_ConsoleWindowKey = "InAppConsoleWindow";
            if (ImGui::Begin(c_ConsoleWindowKey, nullptr, windowFlags))
            {
                ImGui::Text("Console");

                const ImVec2 textArea = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight());

                ImGui::BeginChild("InAppConsoleWindow_Text", textArea);
                {
                    ImGuiWindow* window = ImGui::GetCurrentWindow();

                    const ImVec2 windowAbsMin = ImGui::GetWindowPos();
                    const ImVec2 windowAbsMax = ImVec2(windowAbsMin.x + ImGui::GetWindowSize().x, windowAbsMin.y + ImGui::GetWindowSize().y);
                    const ImRect windowRect(windowAbsMin, windowAbsMax);

                    const ImVec4 consoleTextColour(
                        consoleBackgroundColour.x + 0.2f,
                        consoleBackgroundColour.x + 0.2f,
                        consoleBackgroundColour.x + 0.2f,
                        1.0f);

                    u8 messageIndex = m_consoleMessageLastIndex == 0 ? c_ConsoleMessageSize - 1 : m_consoleMessageLastIndex - 1;
                    const u8 endMessageIndex = m_consoleMessageLastIndex;
                    std::lock_guard l(m_messagesMutex);
                    while (messageIndex != endMessageIndex)
                    {
                        const ConsoleMessage& message = m_consoleMessages[messageIndex];
                        if (!message.Message.empty())
                        {
                            const float worldWrapLength = (windowAbsMax.x - windowAbsMin.x);
                            const ImVec2 textCursorPos = ImVec2(ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
                            const float wrapWidthPos = ImGui::CalcWrapWidthForPos(ImGui::GetCursorPos(), worldWrapLength);
                            const ImVec2 textSize = ImGui::CalcTextSize(message.Message.data());
                            const ImVec2 rectMax(windowAbsMax.x - windowAbsMin.x, (textCursorPos.y + textSize.y) - 2);

                            ImRect textBGRect(textCursorPos, rectMax);
                            //if (textBGRect.Overlaps(windowRect))
                            {
                                textBGRect.ClipWith(windowRect);
                                //ImGui::GetBackgroundDrawList()->AddRectFilled(textBGRect.Min, textBGRect.Max, ImGui::ColorConvertFloat4ToU32(consoleTextColour));
                                const ImVec2 endCursorPos = ImGui::GetCursorScreenPos();

                                //ImGui::SetCursorScreenPos(textCursorPos);

                                const ConsoleMessageColour messageColour = GetMesageColour(message);
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(messageColour.r, messageColour.g, messageColour.b, messageColour.a));
                                ImGui::TextWrapped(message.Message.data());
                                ImGui::PopStyleColor();
                                ImGui::Separator();

                                //ImGui::SetCursorScreenPos(endCursorPos);
                            }
                        }
                        messageIndex = messageIndex == 0 ? c_ConsoleMessageSize - 1 : messageIndex - 1;
                    }

                    const ConsoleMessage& message = m_consoleMessages[messageIndex];
                    if (!message.Message.empty())
                    {
                        ImGui::Text(message.Message.data());
                    }
                }

                ImGui::EndChild();

                ImGui::SetNextItemWidth(-FLT_MIN);
                if (ImGui::InputText("##ConsoleInputText", &inputText, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    Message(">>>" + inputText, ConsoleLogLevel::Info);
                    // Change console variables is possible.
                    const std::vector<std::string_view> cVar = SplitString(std::string_view(inputText.data(), inputText.size()), '=');
                    if (cVar.size() != 2)
                    {
                        Message("Unable to set CVar, the format must be 'Key=Value'.", ConsoleLogLevel::Error);
                    }
                    else
                    {
                        Console::Instance().SetValue(std::string(cVar[0]), std::string(cVar[1]));
                    }
                    inputText.clear();
                }
            }
            ImGui::End();

            ImGui::PopStyleColor();
#endif
            m_windowDiffStateLastFrame = false;
        }
    }
}