#pragma once
#include <Runtime/Platform/PlatformMonitor.h>
#include <Windows.h>

namespace Oksijen
{
	class RUNTIME_API Win32Monitor : public PlatformMonitor
	{
	public:
		static PlatformMonitor* GetPrimaryMonitor();
		static std::vector<PlatformMonitor*> GetMonitors();

	public:
		Win32Monitor(const HMONITOR handle, const std::string& name, const MonitorDisplayMode& currentMode, const std::vector<MonitorDisplayMode>& modes, const unsigned int width, const unsigned int height, const int x, const int y);
		~Win32Monitor();

		FORCEINLINE HMONITOR win32_handle() const noexcept
		{
			return mHandle;
		}

	private:
		const HMONITOR mHandle;
	};
}