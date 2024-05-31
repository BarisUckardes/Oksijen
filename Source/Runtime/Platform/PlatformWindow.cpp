#include "PlatformWindow.h"

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Runtime/Windows/Win32Window.h>
typedef Oksijen::Win32Window PlatformAbstraction;
#endif

namespace Oksijen
{
	PlatformWindow* PlatformWindow::Create(const WindowDesc& desc)
	{
		return new PlatformAbstraction(desc);
	}
	void PlatformWindow::SetTitle(const std::string& title)
	{
		SetTitleCore(title);
		mTitle = title;
	}
	void PlatformWindow::SetMode(const WindowMode mode)
	{
		SetModeCore(mode);
		SetSize(mWidth, mHeight);
		mMode = mode;
	}
	void PlatformWindow::SetPosition(const int x, const int y)
	{
		SetPositionCore(x, y);
		mX = x;
		mY = y;
	}
	void PlatformWindow::SetSize(const unsigned int width, const unsigned int height)
	{
		SetSizeCore(width, height);
		mWidth = width;
		mHeight = height;
	}
	void PlatformWindow::PollEvents()
	{
		mBufferedEvents.clear();
		PollEventsCore();
	}
	void PlatformWindow::Show()
	{
		ShowCore();
		mVisible = true;
	}
	void PlatformWindow::Hide()
	{
		HideCore();
		mVisible = false;
	}
	void PlatformWindow::DispatchWindowEvent(const WindowEventData& event)
	{
		switch (event.Type)
		{
		case Oksijen::WindowEventType::WindowClosed:
		{
			mVisible = false;
			mActive = false;
			break;
		}
		case Oksijen::WindowEventType::WindowMoved:
		{
			mX = event.WindowPosX;
			mY = event.WindowPosY;
			break;
		}
		case Oksijen::WindowEventType::WindowResized:
		{
			mWidth = event.WindowWidth;
			mHeight = event.WindowHeight;
			break;
		}
		case Oksijen::WindowEventType::DragDrop:
			break;
		case Oksijen::WindowEventType::KeyboardDown:
			break;
		case Oksijen::WindowEventType::KeyboardUp:
			break;
		case Oksijen::WindowEventType::Char:
			break;
		case Oksijen::WindowEventType::MouseButtonDown:
			break;
		case Oksijen::WindowEventType::MouseButtonUp:
			break;
		case Oksijen::WindowEventType::MouseMoved:
			break;
		case Oksijen::WindowEventType::MouseScrolled:
			break;
		default:
			break;
		}

		mBufferedEvents.push_back(event);
	}
}