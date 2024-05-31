#pragma once

namespace Oksijen
{
	enum class RUNTIME_API WindowEventType : unsigned char
	{
		WindowClosed,
		WindowMoved,
		WindowResized,

		DragDrop,

		KeyboardDown,
		KeyboardUp,
		Char,

		MouseButtonDown,
		MouseButtonUp,
		MouseMoved,
		MouseScrolled
	};
}