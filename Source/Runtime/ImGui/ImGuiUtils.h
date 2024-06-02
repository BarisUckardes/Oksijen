#include <Runtime/Input/KeyboardKeys.h>
#include <Runtime/Input/MouseButtons.h>
#include <imgui.h>

namespace Oksijen
{
	class RUNTIME_API ImGuiUtils final
	{
	public:
		ImGuiUtils() = delete;
		~ImGuiUtils() = delete;
	public:
		static ImGuiKey GetKeyboardKey(const KeyboardKeys key);
		static ImGuiMouseButton GetMouseButton(const MouseButtons button);
	};
}