#pragma once

#include "Insight/Core/Core.h"
#include "Insight/Module/InputModule.h"
#include "Insight/Input/InsightKeyCodes.h"

class IS_API Input
{
public:

	static bool IsKeyDown(int keycode);
	static bool IsKeyDown(char keycode);
	static bool IsKeyUp(int keycode);
	static bool IsKeyUp(char keycode);
	static bool IsKeyHeld(int keycode);
	static bool IsKeyHeld(char keycode);

	static bool IsMouseButtonDown(int keycode);
	static bool IsMouseButtonUp(int keycode);
	static bool IsMouseButtonPressed(int keycode);
	static bool IsMouseButtonReleased(int keycode);

	static void GetMousePosition(double* mouseX, double* mouseY);
	static void GetMousePosition(float* mouseX, float* mouseY);
	static void GetMousePosition(int* mouseX, int* mouseY);

private:
	static bool KeyCodeExits(int keycode);
};
