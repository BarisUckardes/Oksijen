#include "Win32Window.h"
#include <Runtime/Core/Core.h>

namespace Oksijen
{
	Win32Window* GetUserWindowData(const HWND hwnd)
	{
		return (Win32Window*)GetWindowLongPtr(hwnd, -21); // -21 is a specific number that holds the user provided information
	}
	LRESULT Win32Window::WindowsProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
		{
			//Set user data ptr
			Win32Window* pWindow = (Win32Window*)((LPCREATESTRUCT)lParam)->lpCreateParams;
			SetWindowLongPtr(hwnd, -21, (LONG_PTR)pWindow);

			//Accept files
			DragAcceptFiles(hwnd, TRUE);
			break;
		}
		case WM_DESTROY:
		{
			DragAcceptFiles(hwnd, FALSE);
			break;
		}
		case WM_CLOSE:
		{
			Win32Window* pWindow = GetUserWindowData(hwnd);
			WindowEventData event = {};
			event.Type = WindowEventType::WindowClosed;

			pWindow->DispatchWindowEvent(event);
			break;
		}
		case WM_MOVE:
		{
			Win32Window* pWindow = GetUserWindowData(hwnd);

			WindowEventData event = {};
			event.Type = WindowEventType::WindowMoved;
			event.WindowPosX = LOWORD(lParam);
			event.WindowPosY = HIWORD(lParam);

			pWindow->DispatchWindowEvent(event);

			break;
		}
		case WM_SIZE:
		{
			Win32Window* pWindow = GetUserWindowData(hwnd);

			WindowEventData event = {};
			event.Type = WindowEventType::WindowResized;
			event.WindowWidth = LOWORD(lParam);
			event.WindowHeight = HIWORD(lParam);

			pWindow->DispatchWindowEvent(event);
			break;
		}
		}

		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	Win32Window::Win32Window(const WindowDesc& desc) : PlatformWindow(desc)
	{
		constexpr const char WINDOW_CLASS_NAME[] = "RuntimeWin32WindowClass";

		//Get process handle
		const HMODULE processHandle = GetModuleHandle(NULL);

		//Register window class
		WNDCLASSEX wndClassDesc = {};
		wndClassDesc.cbSize = sizeof(wndClassDesc);
		wndClassDesc.cbClsExtra = 0;
		wndClassDesc.cbWndExtra = sizeof(Win32Window*);
		wndClassDesc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndClassDesc.hCursor = LoadCursor(processHandle, IDC_ARROW);
		wndClassDesc.hIcon = LoadIcon(processHandle, IDI_WINLOGO);
		wndClassDesc.hIconSm = NULL;
		wndClassDesc.hInstance = processHandle;
		wndClassDesc.style = NULL;
		wndClassDesc.lpszMenuName = NULL;
		wndClassDesc.lpszClassName = WINDOW_CLASS_NAME;
		wndClassDesc.lpfnWndProc = WindowsProc;

		DEV_ASSERT(RegisterClassEx(&wndClassDesc) != NULL, "Win32Window", "Failed to register window class");

		//Create window
		const HWND windowHandle = CreateWindowEx(NULL,
			WINDOW_CLASS_NAME, desc.Title.c_str(),
			WS_OVERLAPPEDWINDOW,
			desc.X, desc.Y, desc.Width, desc.Height,
			NULL, NULL, processHandle,
			this);

		mHandle = windowHandle;
		mHDC = GetDC(windowHandle);
	}
	Win32Window::~Win32Window()
	{
		ReleaseDC(mHandle, mHDC);
		DestroyWindow(mHandle);
	}
	void Win32Window::SetTitleCore(const std::string& title)
	{
		SetWindowText(mHandle, title.c_str());
	}
	void Win32Window::SetModeCore(const WindowMode mode)
	{
		switch (mode)
		{
		case Oksijen::WindowMode::Fixed:
		{
			SetWindowLongPtr(mHandle, GWL_STYLE, WS_OVERLAPPEDWINDOW);
			break;
		}
		case Oksijen::WindowMode::Borderless:
		{
			SetWindowLongPtr(mHandle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
			break;
		}
		default:
			break;
		}
	}
	void Win32Window::SetPositionCore(const int x, const int y)
	{
		SetWindowPos(mHandle, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
	}
	void Win32Window::SetSizeCore(const unsigned int width, const unsigned int height)
	{
		SetWindowPos(mHandle, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_FRAMECHANGED);
	}
	void Win32Window::PollEventsCore()
	{
		MSG msg = {};
		while (PeekMessage(&msg, mHandle, NULL, NULL, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	void Win32Window::ShowCore()
	{
		ShowWindow(mHandle, SW_SHOW);
	}
	void Win32Window::HideCore()
	{
		ShowWindow(mHandle, SW_HIDE);
	}
}