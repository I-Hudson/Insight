#include "EditorWindows/InputWindow.h"

#include "Runtime/Engine.h"
#include "Input/InputSystem.h"
#include "Input/InputDevices/InputDevice_KeyboardMouse.h"
#include "Input/InputDevices/InputDeivce_Controller.h"

namespace Insight
{
	namespace Editor
	{
		InputWindow::InputWindow()
			: IEditorWindow()
		{ }
		InputWindow::InputWindow(u32 minWidth, u32 minHeight)
			: IEditorWindow(minWidth, minHeight)
		{ }

		InputWindow::InputWindow(u32 minWidth, u32 minHeight, u32 maxWidth, u32 maxHeight)
			: IEditorWindow(minWidth, minHeight, maxWidth, maxHeight)
		{ }

		InputWindow::~InputWindow()
		{ }

		void InputWindow::OnDraw()
		{
			App::Engine* engine = &App::Engine::Instance();
			Input::InputSystem* inputSystem = engine->GetSystemRegistry().GetSystem<Input::InputSystem>();
			std::vector<Input::IInputDevice*> inputDevices = inputSystem->GetAllInputDevices();
			for (auto const& device : inputDevices)
			{
				DrawSingleInputDevuce(device);
			}
		}

		void InputWindow::DrawSingleInputDevuce(Input::IInputDevice const* inputDevice)
		{
			std::string label = Input::InputDeviceTypeToString(inputDevice->GetDeviceType()) + std::to_string(inputDevice->GetId());
			if (ImGui::TreeNodeEx(label.c_str()))
			{
				if (inputDevice->GetDeviceType() == Input::InputDeviceTypes::KeyboardMouse)
				{
					Input::InputDevice_KeyboardMouse const* device = static_cast<Input::InputDevice_KeyboardMouse const*>(inputDevice);
					ImGui::Text("Buttons Pressed:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::KeyboardButtons::NumButtons); ++i)
					{
						if (device->WasPressed(static_cast<Input::KeyboardButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::KeyboardButtonToString(static_cast<Input::KeyboardButtons>(i)));
						}
					}
					ImGui::Unindent();

					ImGui::Text("Buttons Released:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::KeyboardButtons::NumButtons); ++i)
					{
						if (device->WasReleased(static_cast<Input::KeyboardButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::KeyboardButtonToString(static_cast<Input::KeyboardButtons>(i)));
						}
					}
					ImGui::Unindent();

					ImGui::Text("Buttons Held:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::KeyboardButtons::NumButtons); ++i)
					{
						if (device->WasHeld(static_cast<Input::KeyboardButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::KeyboardButtonToString(static_cast<Input::KeyboardButtons>(i)));
						}
					}
					ImGui::Unindent();
				}
				else if (inputDevice->GetDeviceType() == Input::InputDeviceTypes::Controller)
				{
					Input::InputDevice_Controller const* device = static_cast<Input::InputDevice_Controller const*>(inputDevice);
					ImGui::Text("Buttons Pressed:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::ControllerButtons::NumButtons); ++i)
					{
						if (device->WasPressed(static_cast<Input::ControllerButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::ControllerButtonsToString(static_cast<Input::ControllerButtons>(i)));
						}
					}
					ImGui::Unindent();

					ImGui::Text("Buttons Released:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::ControllerButtons::NumButtons); ++i)
					{
						if (device->WasReleased(static_cast<Input::ControllerButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::ControllerButtonsToString(static_cast<Input::ControllerButtons>(i)));
						}
					}
					ImGui::Unindent();

					ImGui::Text("Buttons Held:");
					ImGui::Separator();
					ImGui::Indent();
					for (size_t i = 0; i < static_cast<u64>(Input::ControllerButtons::NumButtons); ++i)
					{
						if (device->WasHeld(static_cast<Input::ControllerButtons>(i)))
						{
							ImGui::Text("Button: '%s'.", Input::ControllerButtonsToString(static_cast<Input::ControllerButtons>(i)));
						}
					}
					ImGui::Unindent();
				}
				ImGui::TreePop();
			}
		}
	}
}