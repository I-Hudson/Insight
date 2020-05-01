#include "ispch.h"
#include "Input.h"

using namespace Insight::Module;

bool Input::KeyDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_PRESS;
}

bool Input::KeyDown(char keycode)
{
	return KeyDown((int)keycode);
}

bool Input::KeyUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_RELEASE;
}

bool Input::KeyUp(char keycode)
{
	return KeyUp((int)keycode);
}

bool Input::KeyHeld(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode] == KEY_REPEAT;
}

bool Input::KeyHeld(char keycode)
{
	return KeyHeld((int)keycode);
}

void Input::GetMousePosition(double* mouseX, double* mouseY)
{
	*mouseX = InputModule::m_mouseX;
	*mouseY = InputModule::m_mouseY;
}

bool Input::KeyCodeExits(int keycode)
{
	return InputModule::m_inputStates.find(keycode) != InputModule::m_inputStates.end();
}

