#pragma once
#include <Runtime/Platform/PlatformWindow.h>
#include <Windows.h>

namespace Oksijen
{
	class RUNTIME_API Win32Window final : public PlatformWindow
	{
	private:
		static LRESULT CALLBACK WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		Win32Window(const WindowDesc& desc);
		virtual ~Win32Window() override;

		FORCEINLINE HWND GetWin32Handle() const noexcept { return mHandle; }
		FORCEINLINE HDC GetWin32HDC() const noexcept { return mHDC; }
	private:

	private:
		HWND mHandle;
		HDC mHDC;

		// Inherited via PlatformWindow
		void SetTitleCore(const std::string& title) override;
		void SetModeCore(const WindowMode mode) override;
		void SetPositionCore(const int x, const int y) override;
		void SetSizeCore(const unsigned int width, const unsigned int height) override;
		void PollEventsCore() override;
		void ShowCore() override;
		void HideCore() override;
	};
}