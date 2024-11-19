#include "Core/Console.h"

#include <imgui.h>
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

        void Console::Message(const std::string_view message)
        {
            ConsoleMessageColour colour;
            colour.r = 1;
            colour.g = 1;
            colour.b = 1;
            colour.a = 1;
            Message(message, colour);
        }

        void Console::Message(const std::string_view message, const ConsoleMessageColour colour)
        {
            ConsoleMessage consoleMessage;
            consoleMessage.Message = message;
            consoleMessage.Colour = colour;

            std::lock_guard l(m_messagesMutex);
            m_consoleMessages[m_consoleMessageLastIndex] = consoleMessage;

            // Wrap the start index back to 0 if required otherwise just increment by one.
            m_consoleMessageLastIndex = (m_consoleMessageLastIndex + 1) % c_ConsoleMessageSize;
        }

        void Console::Show(const bool shouldShow)
        {
            m_isShowing = shouldShow;
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
            //ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
            const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration;

            ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1, 0, 0, 1));
            ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

            const char* c_ConsoleWindowKey = "InAppConsoleWindow";
            if (ImGui::Begin(c_ConsoleWindowKey, nullptr, windowFlags))
            {
                ImGui::Text("Console");

                const ImVec2 textArea = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - ImGui::GetTextLineHeight());

                ImGui::BeginChild("InAppConsoleWindow_Text", textArea);
                {
                    std::lock_guard l(m_messagesMutex);

                    u8 messageIndex = m_consoleMessageLastIndex;
                    const u8 endMessageIndex = m_consoleMessageLastIndex == 0 ? c_ConsoleMessageSize - 1 : m_consoleMessageLastIndex - 1;
                    while (messageIndex != endMessageIndex)
                    {
                        const ConsoleMessage& message = m_consoleMessages[messageIndex];
                        if (!message.Message.empty())
                        {
                            ImGui::Text(message.Message.data());
                        }
                        messageIndex = (messageIndex + 1) % c_ConsoleMessageSize;
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
                    Message(inputText);
                    inputText.clear();
                }
            }
            ImGui::End();

            ImGui::PopStyleColor();
#endif
        }
    }
}