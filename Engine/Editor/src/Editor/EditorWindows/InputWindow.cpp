#include "Editor/EditorWindows/InputWindow.h"

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
			IS_PROFILE_FUNCTION();

			App::Engine* engine = &App::Engine::Instance();
			Input::InputSystem* inputSystem = engine->GetSystemRegistry().GetSystem<Input::InputSystem>();

			int currentInputManagerType = static_cast<int>(inputSystem->GetInputManagerType());
			std::vector<const char*> inputManagerTypes;
			inputManagerTypes.reserve(static_cast<u64>(Input::InputSystemInputManagerTypes::NumInputSystemInputManagers));

			for (size_t i = 0; i < static_cast<u64>(Input::InputSystemInputManagerTypes::NumInputSystemInputManagers); ++i)
			{
				inputManagerTypes.push_back(Input::InputSystemInputManagerTypeToString(static_cast<Input::InputSystemInputManagerTypes>(i)));
			}

			if (ImGui::Combo("Current Input system", &currentInputManagerType, inputManagerTypes.data(), static_cast<int>(inputManagerTypes.size())))
			{
				inputSystem->SetInputManagerType(static_cast<Input::InputSystemInputManagerTypes>(currentInputManagerType));
			}

			Input::IInputDevice const* lastUsedDevice = inputSystem->GetLastUsedInputDevices();
			std::string lastUsedDeviceLabel = "None";
			if (lastUsedDevice)
			{
				lastUsedDeviceLabel = Input::InputDeviceTypeToString(lastUsedDevice->GetDeviceType()) + std::to_string(lastUsedDevice->GetId());
			}
			ImGui::Text("Last used input device: '%s'.", lastUsedDeviceLabel.c_str());
			ImGui::Separator();

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
					{
						ImGui::Text("Mouse:");
						ImGui::Separator();
					}

					{
						ImGui::Text("Position:");
						ImGui::Separator();
						ImGui::Indent();
						ImGui::Text("X: '%f'.", device->GetMouseXPosition());
						ImGui::Text("Y: '%f'.", device->GetMouseYPosition());
						ImGui::Unindent();
					}

					{
						ImGui::Text("Scroll:");
						ImGui::Separator();
						ImGui::Indent();
						ImGui::Text("X offset: '%f'.", device->GetMouseXScrollOffset());
						ImGui::Text("Y offset: '%f'.", device->GetMouseYScrollOffset());
						ImGui::Unindent();
					}

					{
						ImGui::Text("Buttons Pressed:");
						ImGui::Separator();
						ImGui::Indent();
						for (size_t i = 0; i < static_cast<u64>(Input::MouseButtons::NumButtons); ++i)
						{
							if (device->WasPressed(static_cast<Input::MouseButtons>(i)))
							{
								ImGui::Text("Button: '%s'.", Input::MouseButtonToString(static_cast<Input::MouseButtons>(i)));
							}
						}
						ImGui::Unindent();
					}

					{
						ImGui::Text("Buttons Released:");
						ImGui::Separator();
						ImGui::Indent();
						for (size_t i = 0; i < static_cast<u64>(Input::MouseButtons::NumButtons); ++i)
						{
							if (device->WasReleased(static_cast<Input::MouseButtons>(i)))
							{
								ImGui::Text("Button: '%s'.", Input::MouseButtonToString(static_cast<Input::MouseButtons>(i)));
							}
						}
						ImGui::Unindent();
					}

					{
						ImGui::Text("Buttons Held:");
						ImGui::Separator();
						ImGui::Indent();
						for (size_t i = 0; i < static_cast<u64>(Input::MouseButtons::NumButtons); ++i)
						{
							if (device->WasHeld(static_cast<Input::MouseButtons>(i)))
							{
								ImGui::Text("Button: '%s'.", Input::MouseButtonToString(static_cast<Input::MouseButtons>(i)));
							}
						}
						ImGui::Unindent();
					}

					{
						ImGui::Text("Keyboard:");
						ImGui::Separator();
					}

					{
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
					}

					{
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
					}

					{
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
				}
				else if (inputDevice->GetDeviceType() == Input::InputDeviceTypes::Controller)
				{
					Input::InputDevice_Controller* device = const_cast<Input::InputDevice_Controller*>(static_cast<Input::InputDevice_Controller const*>(inputDevice));
					{
						ImGui::Text("Controller vendor: '%s'.", Input::ControllerVendorToString(device->GetVendor()));
						ImGui::Text("Controller sub type: '%s'.", Input::ControllerSubTypeToString(device->GetSubType()));
						ImGui::Separator();
					}
					
					{
						ImGui::Text("Thumbsticks:");
						ImGui::Separator();
						ImGui::Indent();
						for (size_t i = 0; i < static_cast<u64>(Input::ControllerThumbsticks::NumThumbsticks); ++i)
						{
							Input::ControllerThumbsticks currentThumbstick = static_cast<Input::ControllerThumbsticks>(i);
							ImGui::Text("Thumbstick: '%s', value: '%f'.",
								Input::ControllerThumbstickToString(currentThumbstick),
								device->GetThumbstickValue(currentThumbstick));
						}
						ImGui::Unindent();
					}

					{
						ImGui::Text("Triggers:");
						ImGui::Separator();
						ImGui::Indent();
						for (size_t i = 0; i < static_cast<u64>(Input::ControllerTriggers::NumTriggers); ++i)
						{
							Input::ControllerTriggers currentTrigger = static_cast<Input::ControllerTriggers>(i);
							ImGui::Text("Trigger: '%s', value: '%f'.",
								Input::ControllerTriggerToString(currentTrigger),
								device->GetTriggerValue(currentTrigger));
						}
						ImGui::Unindent();
					}

					{
						ImGui::Text("Rumble:");
						ImGui::Separator();
						ImGui::Indent();
						for (int i = 0; i < static_cast<u64>(Input::ControllerRumbles::NumRumbles); ++i)
						{
							Input::ControllerRumbles currentRumble = static_cast<Input::ControllerRumbles>(i);
							ImGui::Text("Rumble: '%s', value: '%f'.",
								Input::ControllerRumbleToString(currentRumble),
								device->GetRumbleValue(currentRumble));
							ImGui::NewLine();

							float rumbleValue = device->GetRumbleValue(currentRumble);
							ImGui::PushID(i);
							ImGui::DragFloat("Rumble value", &rumbleValue, 0.001f, 0.0f, 1.0f);
							ImGui::PopID();
							device->SetRumbleValue(currentRumble, rumbleValue);

							ImGui::NewLine();
							ImGui::NewLine();
						}
						ImGui::Unindent();
					}

					{
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
					}

					{
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
					}

					{
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
				}
				ImGui::TreePop();
			}
		}
	}
}