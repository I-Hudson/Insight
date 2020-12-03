#pragma once

enum IS_UIFlags
{
	UIFlags_None = 0,
	UIFlags_ShowInEditor = 1 << 1,
	UIFlags_ClampZero = 1 << 2,
	UIFlags_ReadOnly = 1 << 2,
};