#pragma once

#include "Insight/Core.h"
#include "Insight/Module/InputModule.h"
#include "Insight/Input/InsightKeyCodes.h"


class IS_API Input
{
public:

	static bool KeyDown(int keycode);
	static bool KeyUp(int keycode);
	static bool KeyHeld(int keycode);

private:
	static bool KeyCodeExits(int keycode);
};
