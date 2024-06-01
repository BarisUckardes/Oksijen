#pragma once
#include <Runtime/Monitor/MonitorDisplayMode.h>
#include <string>
#include <vector>

namespace Oksijen
{
	class RUNTIME_API PlatformMonitor
	{
	public:
		static PlatformMonitor* GetPrimaryMonitor();
		static std::vector<PlatformMonitor*> GetMonitors();
	public:
		PlatformMonitor(const std::string& name, const MonitorDisplayMode& currentMode, const std::vector<MonitorDisplayMode>& modes, const unsigned int width, const unsigned int height, const int x, const int y) :
			mName(name), mCurrentMode(currentMode), mModes(modes), mWidth(width), mHeight(height), mX(x), mY(y)
		{

		}
		~PlatformMonitor();

		FORCEINLINE std::string GetName() const noexcept { return mName; }
		FORCEINLINE MonitorDisplayMode GetCurrentMode() const noexcept { return mCurrentMode; }
		FORCEINLINE const std::vector<MonitorDisplayMode>& GetDisplayModes() const noexcept { return mModes; }
		FORCEINLINE unsigned int GetWidth() const noexcept { return mWidth; }
		FORCEINLINE unsigned int GetHeight() const noexcept { return mHeight; }
		FORCEINLINE int GetPosX() const noexcept { return mX; }
		FORCEINLINE int GetPosY() const noexcept { return mY; }

	private:
		const std::string mName;
		const std::vector<MonitorDisplayMode> mModes;
		const unsigned int mWidth;
		const unsigned int mHeight;
		const int mX;
		const int mY;
		MonitorDisplayMode mCurrentMode;
	};
}