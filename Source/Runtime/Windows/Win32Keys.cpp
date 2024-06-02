#include "Win32Keys.h"
#include <winuser.rh>

namespace Oksijen
{
	KeyboardKeys Win32Keys::GetKey(const unsigned int vk_key)
	{
        switch (vk_key)
        {
        case VK_TAB:
            return KeyboardKeys::Tab;
        case VK_INSERT:
            return KeyboardKeys::Insert;
        case VK_DELETE:
            return KeyboardKeys::Delete;
        case VK_BACK:
            return KeyboardKeys::Backspace;
        case VK_SPACE:
            return KeyboardKeys::Space;
        case VK_RETURN:
            return KeyboardKeys::Enter;
        case VK_ESCAPE:
            return KeyboardKeys::Escape;
        case VK_CONTROL:
            return KeyboardKeys::LeftControl;
        case VK_LSHIFT:
            return KeyboardKeys::LeftShift;
        case VK_RSHIFT:
            return KeyboardKeys::RightShift;
        case VK_SHIFT:
            return KeyboardKeys::LeftShift;
        case VK_CAPITAL:
            return KeyboardKeys::CapsLock;
        case VK_RIGHT:
            return KeyboardKeys::Right;
        case VK_LEFT:
            return KeyboardKeys::Left;
        case VK_UP:
            return KeyboardKeys::Up;
        case VK_DOWN:
            return KeyboardKeys::Down;
        case VK_LWIN:
        case VK_RWIN:
            return KeyboardKeys::Menu;
        case VK_MENU:
            return KeyboardKeys::LeftAlt;
        case VK_SNAPSHOT:
            return KeyboardKeys::PrintScreen;
        default:
        {
            if (vk_key >= 0x30 && vk_key <= 0x39) // its numeric
            {
                const KeyboardKeys key = (KeyboardKeys)((long long)KeyboardKeys::Num0 + vk_key - 0x30);
                return key;
            }
            if (vk_key >= 0x41 && vk_key <= 0x5A) // its Char
            {
                const KeyboardKeys key = (KeyboardKeys)((long long)KeyboardKeys::A + vk_key - 0x41);
                return key;
            }
            if (vk_key >= 0x60 && vk_key <= 0x69) // its numpad numerics
            {
                return (KeyboardKeys)((long long)KeyboardKeys::Kp0 + vk_key - 0x60);
            }
            if (vk_key >= 0x70 && vk_key <= 0x87) // its function keys
            {
                return (KeyboardKeys)((long long)KeyboardKeys::F1 + vk_key - 0x70);
            }

        }
        }

        return KeyboardKeys::Unknown;
	}
}