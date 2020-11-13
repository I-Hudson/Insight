#include "ispch.h"
#include "Input.h"

using namespace Insight::Module;

bool Input::KeyDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].KeyPressed & 1;
}

bool Input::KeyDown(char keycode)
{
	return KeyDown(toupper(keycode));
}

bool Input::KeyUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].KeyReleased & 1;
}

bool Input::KeyUp(char keycode)
{
	return KeyUp((int)keycode);
}

bool Input::KeyHeld(int keycode)
{
	return KeyCodeExits(keycode) && (InputModule::m_inputStates[keycode].KeyHeld & 1 || KeyDown(keycode));
}

bool Input::KeyHeld(char keycode)
{
	return KeyHeld((int)keycode);
}

bool Input::MouseButtonDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].MouseButtonPressed & 1;
}

bool Input::MouseButtonUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].MouseButtonReleased & 1;
}

void Input::GetMousePosition(double* mouseX, double* mouseY)
{
	*mouseX = InputModule::m_mouseX;
	*mouseY = InputModule::m_mouseY;
}

void Input::GetMousePosition(float* mouseX, float* mouseY)
{
	*mouseX = (float)InputModule::m_mouseX;
	*mouseY = (float)InputModule::m_mouseY;
}

void Input::GetMousePosition(int* mouseX, int* mouseY)
{
	*mouseX = (int)InputModule::m_mouseX;
	*mouseY = (int)InputModule::m_mouseY;
}

bool Input::KeyCodeExits(int keycode)
{
	return InputModule::m_inputStates.find(keycode) != InputModule::m_inputStates.end();
}

