#pragma once

#include "Core/TypeAlias.h"

namespace Insight
{
	namespace Input
	{
		////
		/// Taken from GLFW.
		////
		/* The unknown key */
#define IS_KEY_UNKNOWN            -1

/* Printable keys */
#define IS_KEY_SPACE              32
#define IS_KEY_APOSTROPHE         39  /* ' */
#define IS_KEY_COMMA              44  /* , */
#define IS_KEY_MINUS              45  /* - */
#define IS_KEY_PERIOD             46  /* . */
#define IS_KEY_SLASH              47  /* / */
#define IS_KEY_0                  48
#define IS_KEY_1                  49
#define IS_KEY_2                  50
#define IS_KEY_3                  51
#define IS_KEY_4                  52
#define IS_KEY_5                  53
#define IS_KEY_6                  54
#define IS_KEY_7                  55
#define IS_KEY_8                  56
#define IS_KEY_9                  57
#define IS_KEY_SEMICOLON          59  /* ; */
#define IS_KEY_EQUAL              61  /* = */
#define IS_KEY_A                  65
#define IS_KEY_B                  66
#define IS_KEY_C                  67
#define IS_KEY_D                  68
#define IS_KEY_E                  69
#define IS_KEY_F                  70
#define IS_KEY_G                  71
#define IS_KEY_H                  72
#define IS_KEY_I                  73
#define IS_KEY_J                  74
#define IS_KEY_K                  75
#define IS_KEY_L                  76
#define IS_KEY_M                  77
#define IS_KEY_N                  78
#define IS_KEY_O                  79
#define IS_KEY_P                  80
#define IS_KEY_Q                  81
#define IS_KEY_R                  82
#define IS_KEY_S                  83
#define IS_KEY_T                  84
#define IS_KEY_U                  85
#define IS_KEY_V                  86
#define IS_KEY_W                  87
#define IS_KEY_X                  88
#define IS_KEY_Y                  89
#define IS_KEY_Z                  90
#define IS_KEY_LEFT_BRACKET       91  /* [ */
#define IS_KEY_BACKSLASH          92  /* \ */
#define IS_KEY_RIGHT_BRACKET      93  /* ] */
#define IS_KEY_GRAVE_ACCENT       96  /* ` */
#define IS_KEY_WORLD_1            161 /* non-US #1 */
#define IS_KEY_WORLD_2            162 /* non-US #2 */

/* Function keys */
#define IS_KEY_ESCAPE             256
#define IS_KEY_ENTER              257
#define IS_KEY_TAB                258
#define IS_KEY_BACKSPACE          259
#define IS_KEY_INSERT             260
#define IS_KEY_DELETE             261
#define IS_KEY_RIGHT              262
#define IS_KEY_LEFT               263
#define IS_KEY_DOWN               264
#define IS_KEY_UP                 265
#define IS_KEY_PAGE_UP            266
#define IS_KEY_PAGE_DOWN          267
#define IS_KEY_HOME               268
#define IS_KEY_END                269
#define IS_KEY_CAPS_LOCK          280
#define IS_KEY_SCROLL_LOCK        281
#define IS_KEY_NUM_LOCK           282
#define IS_KEY_PRINT_SCREEN       283
#define IS_KEY_PAUSE              284
#define IS_KEY_F1                 290
#define IS_KEY_F2                 291
#define IS_KEY_F3                 292
#define IS_KEY_F4                 293
#define IS_KEY_F5                 294
#define IS_KEY_F6                 295
#define IS_KEY_F7                 296
#define IS_KEY_F8                 297
#define IS_KEY_F9                 298
#define IS_KEY_F10                299
#define IS_KEY_F11                300
#define IS_KEY_F12                301
#define IS_KEY_F13                302
#define IS_KEY_F14                303
#define IS_KEY_F15                304
#define IS_KEY_F16                305
#define IS_KEY_F17                306
#define IS_KEY_F18                307
#define IS_KEY_F19                308
#define IS_KEY_F20                309
#define IS_KEY_F21                310
#define IS_KEY_F22                311
#define IS_KEY_F23                312
#define IS_KEY_F24                313
#define IS_KEY_F25                314
#define IS_KEY_KP_0               320
#define IS_KEY_KP_1               321
#define IS_KEY_KP_2               322
#define IS_KEY_KP_3               323
#define IS_KEY_KP_4               324
#define IS_KEY_KP_5               325
#define IS_KEY_KP_6               326
#define IS_KEY_KP_7               327
#define IS_KEY_KP_8               328
#define IS_KEY_KP_9               329
#define IS_KEY_KP_DECIMAL         330
#define IS_KEY_KP_DIVIDE          331
#define IS_KEY_KP_MULTIPLY        332
#define IS_KEY_KP_SUBTRACT        333
#define IS_KEY_KP_ADD             334
#define IS_KEY_KP_ENTER           335
#define IS_KEY_KP_EQUAL           336
#define IS_KEY_LEFT_SHIFT         340
#define IS_KEY_LEFT_CONTROL       341
#define IS_KEY_LEFT_ALT           342
#define IS_KEY_LEFT_SUPER         343
#define IS_KEY_RIGHT_SHIFT        344
#define IS_KEY_RIGHT_CONTROL      345
#define IS_KEY_RIGHT_ALT          346
#define IS_KEY_RIGHT_SUPER        347
#define IS_KEY_MENU               348

#define IS_KEY_LAST               IS_KEY_MENU

/*! @brief If this bit is set one or more Shift keys were held down.
 *
 *  If this bit is set one or more Shift keys were held down.
 */
#define IS_MOD_SHIFT           0x0001
 /*! @brief If this bit is set one or more Control keys were held down.
  *
  *  If this bit is set one or more Control keys were held down.
  */
#define IS_MOD_CONTROL         0x0002
  /*! @brief If this bit is set one or more Alt keys were held down.
   *
   *  If this bit is set one or more Alt keys were held down.
   */
#define IS_MOD_ALT             0x0004
   /*! @brief If this bit is set one or more Super keys were held down.
	*
	*  If this bit is set one or more Super keys were held down.
	*/
#define IS_MOD_SUPER           0x0008
	/*! @brief If this bit is set the Caps Lock key is enabled.
	 *
	 *  If this bit is set the Caps Lock key is enabled and the @ref
	 *  IS_LOCK_KEY_MODS input mode is set.
	 */
#define IS_MOD_CAPS_LOCK       0x0010
	 /*! @brief If this bit is set the Num Lock key is enabled.
	  *
	  *  If this bit is set the Num Lock key is enabled and the @ref
	  *  IS_LOCK_KEY_MODS input mode is set.
	  */
#define IS_MOD_NUM_LOCK        0x0020

	  /*! @} */

	  /*! @defgroup buttons Mouse buttons
	   *  @brief Mouse button IDs.
	   *
	   *  See [mouse button input](@ref input_mouse_button) for how these are used.
	   *
	   *  @ingroup input
	   *  @{ */
#define IS_MOUSE_BUTTON_1         0
#define IS_MOUSE_BUTTON_2         1
#define IS_MOUSE_BUTTON_3         2
#define IS_MOUSE_BUTTON_4         3
#define IS_MOUSE_BUTTON_5         4
#define IS_MOUSE_BUTTON_6         5
#define IS_MOUSE_BUTTON_7         6
#define IS_MOUSE_BUTTON_8         7
#define IS_MOUSE_BUTTON_LAST      IS_MOUSE_BUTTON_8
#define IS_MOUSE_BUTTON_LEFT      IS_MOUSE_BUTTON_1
#define IS_MOUSE_BUTTON_RIGHT     IS_MOUSE_BUTTON_2
#define IS_MOUSE_BUTTON_MIDDLE    IS_MOUSE_BUTTON_3
	}
}