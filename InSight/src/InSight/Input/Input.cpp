#include "ispch.h"
#include "Input.h"

using namespace Insight::Module;

bool Input::KeyDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_PRESS;
}

bool Input::KeyUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_RELEASE;
}

bool Input::KeyHeld(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_REPEAT;
}

bool Input::KeyCodeExits(int keycode)
{
	return InputModule::m_inputStates.find(keycode) != InputModule::m_inputStates.end();
}

