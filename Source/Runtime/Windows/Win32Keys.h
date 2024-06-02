#pragma once
#include <Runtime/Input/KeyboardKeys.h>
#include <Runtime/Core/Core.h>

namespace Oksijen
{
	class RUNTIME_API Win32Keys
	{
	public:
		static KeyboardKeys GetKey(const unsigned int vk_key);
	public:
		Win32Keys() = delete;
		~Win32Keys() = delete;
	};
}