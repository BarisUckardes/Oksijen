#pragma once
#include <Runtime/Window/WindowDesc.h>
#include <Runtime/Window/WindowMode.h>
#include <Runtime/Window/WindowEventData.h>

namespace Oksijen
{
	class RUNTIME_API PlatformWindow
	{
	public:
		static PlatformWindow* Create(const WindowDesc& desc);
	public:
		virtual ~PlatformWindow() {}

		FORCEINLINE const std::vector<WindowEventData>& GetBufferedEvents() const noexcept { return mBufferedEvents; }
		FORCEINLINE std::string GetTitle() const noexcept { return mTitle; }
		FORCEINLINE WindowMode GetMode() const noexcept { return mMode; }
		FORCEINLINE int GetPosX() const noexcept { return mX; }
		FORCEINLINE int GetPosY() const noexcept { return mY; }
		FORCEINLINE unsigned int GetWidth() const noexcept { return mWidth; }
		FORCEINLINE unsigned int GetHeight() const noexcept { return mHeight; }
		FORCEINLINE bool IsActive() const noexcept { return mActive; }
		FORCEINLINE bool IsVisible() const noexcept { return mVisible; }

		void SetTitle(const std::string& title);
		void SetMode(const WindowMode mode);
		void SetPosition(const int x, const int y);
		void SetSize(const unsigned int width, const unsigned int height);
		void PollEvents();
		void Show();
		void Hide();
	protected:
		PlatformWindow(const WindowDesc& desc) : mTitle(desc.Title), mX(desc.X), mY(desc.Y), mWidth(desc.Width), mHeight(desc.Height), mVisible(false), mActive(true)
		{

		}

		void DispatchWindowEvent(const WindowEventData& event);

		virtual void SetTitleCore(const std::string& title) = 0;
		virtual void SetModeCore(const WindowMode mode) = 0;
		virtual void SetPositionCore(const int x, const int y) = 0;
		virtual void SetSizeCore(const unsigned int width, const unsigned int height) = 0;
		virtual void PollEventsCore() = 0;
		virtual void ShowCore() = 0;
		virtual void HideCore() = 0;
	private:
		std::vector<WindowEventData> mBufferedEvents;
		std::string mTitle;
		WindowMode mMode;
		int mX;
		int mY;
		unsigned int mWidth;
		unsigned int mHeight;
		bool mActive;
		bool mVisible;
	};
}