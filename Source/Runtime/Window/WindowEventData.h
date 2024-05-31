#pragma once
#include <Runtime/Window/WindowEventType.h>
#include <Runtime/Input/KeyboardKeys.h>
#include <Runtime/Input/MouseButtons.h>
#include <string>
#include <vector>

namespace Oksijen
{
	struct RUNTIME_API WindowEventData final
	{
		WindowEventType Type;

		unsigned int WindowWidth;
		unsigned int WindowHeight;
		int WindowPosX;
		int WindowPosY;

		MouseButtons MouseButton;
		int MousePosX;
		int MousePosY;
		float MouseWheelDelta;

		KeyboardKeys KeyboardKey;
		char KeyboardChar;

		std::vector<std::string> DropItems;
	};
}