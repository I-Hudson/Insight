#pragma once

#include "Insight/Core.h"
#include "Insight/Module/InputModule.h"
#include "Insight/Input/InsightKeyCodes.h"

class IS_API Input
{
public:

	static bool KeyDown(int keycode);
	static bool KeyDown(char keycode);
	static bool KeyUp(int keycode);
	static bool KeyUp(char keycode);
	static bool KeyHeld(int keycode);
	static bool KeyHeld(char keycode);

	static bool MouseButtonDown(int keycode);
	static bool MouseButtonUp(int keycode);
	static void GetMousePosition(double* mouseX, double* mouseY);
	static void GetMousePosition(float* mouseX, float* mouseY);
	static void GetMousePosition(int* mouseX, int* mouseY);

private:
	static bool KeyCodeExits(int keycode);
};
