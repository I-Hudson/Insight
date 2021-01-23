#include "ispch.h"
#include "Input.h"

using namespace Module;

bool Input::IsKeyDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].KeyDown & 1;
}

bool Input::IsKeyDown(char keycode)
{
	return IsKeyDown(toupper(keycode));
}

bool Input::IsKeyUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].KeyUp & 1;
}

bool Input::IsKeyUp(char keycode)
{
	return IsKeyUp((int)keycode);
}

bool Input::IsKeyHeld(int keycode)
{
	return KeyCodeExits(keycode) && (InputModule::m_inputStates[keycode].KeyHeld & 1 || IsKeyDown(keycode));
}

bool Input::IsKeyHeld(char keycode)
{
	return IsKeyHeld((int)keycode);
}

bool Input::IsMouseButtonDown(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].MouseButtonDown & 1;
}

bool Input::IsMouseButtonUp(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].MouseButtonUp & 1;
}

bool Input::IsMouseButtonPressed(int keycode)
{
	return KeyCodeExits(keycode) && InputModule::m_inputStates[keycode].MouseButtonPressed & 1;
}

bool Input::IsMouseButtonReleased(int keycode)
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

