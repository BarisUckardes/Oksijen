#include "Win32Monitor.h"
#include <Runtime/Core/Core.h>

namespace Oksijen
{

#define WIN32_MAX_MONITORS 32
	struct MonitorProcData
	{
		HMONITOR Monitors[WIN32_MAX_MONITORS];
		unsigned int ProcIndex;
	};
	BOOL CALLBACK MonitorEnumProc(HMONITOR PlatformMonitor, HDC monitorContext, LPRECT rect, LPARAM userData)
	{
		MonitorProcData* pData = (MonitorProcData*)userData;
		pData->Monitors[pData->ProcIndex] = PlatformMonitor;
		pData->ProcIndex++;
		return TRUE;
	}
	PlatformMonitor* Win32Monitor::GetPrimaryMonitor()
	{
		//Get process handle
		const HMODULE processHandle = GetModuleHandle(NULL);

		//Get monitors
		MonitorProcData procData = {};
		procData.ProcIndex = 0;
		if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&procData) == 0)
		{
			return nullptr;
		}

		//Get primary PlatformMonitor
		if (procData.ProcIndex == 0)
			return nullptr;

		const HMONITOR monitorHandle = procData.Monitors[0];

		//Get PlatformMonitor information
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(monitorInfo);
		if (GetMonitorInfo(monitorHandle, &monitorInfo) == 0)
			return nullptr;

		//Get dev mode
		DEVMODE currentDisplayModeWin32 = {};
		currentDisplayModeWin32.dmSize = sizeof(currentDisplayModeWin32);
		if (EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &currentDisplayModeWin32) == 0)
			return nullptr;

		MonitorDisplayMode currentDisplayMode = {};
		currentDisplayMode.BitsPerPixel = currentDisplayModeWin32.dmBitsPerPel;
		currentDisplayMode.RefreshRate = currentDisplayModeWin32.dmDisplayFrequency;
		currentDisplayMode.Width = currentDisplayModeWin32.dmPelsWidth;
		currentDisplayMode.Height = currentDisplayModeWin32.dmPelsHeight;

		std::vector<MonitorDisplayMode> displayModes;
		unsigned int modeIndex = 0;
		if (EnumDisplaySettings(monitorInfo.szDevice, 0, &currentDisplayModeWin32) == 0)
			return nullptr;

		while (true)
		{
			if (EnumDisplaySettings(monitorInfo.szDevice, modeIndex, &currentDisplayModeWin32) == 0)
				break;

			MonitorDisplayMode displayMode = {};
			displayMode.Width = currentDisplayModeWin32.dmPelsWidth;
			displayMode.Height = currentDisplayModeWin32.dmPelsHeight;
			displayMode.BitsPerPixel = currentDisplayModeWin32.dmBitsPerPel;
			displayMode.RefreshRate = currentDisplayModeWin32.dmDisplayFrequency;

			displayModes.push_back(displayMode);
			modeIndex++;
		}
		Win32Monitor* pMonitor = new Win32Monitor(monitorHandle, monitorInfo.szDevice, currentDisplayMode, displayModes, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top);
		return pMonitor;
	}
	std::vector<PlatformMonitor*> Win32Monitor::GetMonitors()
	{
		//Get process handle
		const HMODULE processHandle = GetModuleHandle(NULL);

		//Get monitors
		MonitorProcData procData = {};
		procData.ProcIndex = 0;
		if (EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&procData) == 0)
		{
			return {};
		}

		//Get primary PlatformMonitor
		if (procData.ProcIndex == 0)
			return {};

		std::vector<PlatformMonitor*> monitors;
		for (unsigned int i = 0; i < procData.ProcIndex; i++)
		{
			const HMONITOR monitorHandle = procData.Monitors[i];

			//Get PlatformMonitor information
			MONITORINFOEX monitorInfo = {};
			monitorInfo.cbSize = sizeof(monitorInfo);
			if (GetMonitorInfo(monitorHandle, &monitorInfo) == 0)
				return {};

			//Get dev mode
			DEVMODE currentDisplayModeWin32 = {};
			currentDisplayModeWin32.dmSize = sizeof(currentDisplayModeWin32);
			if (EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &currentDisplayModeWin32) == 0)
				return {};

			MonitorDisplayMode currentDisplayMode = {};
			currentDisplayMode.BitsPerPixel = currentDisplayModeWin32.dmBitsPerPel;
			currentDisplayMode.RefreshRate = currentDisplayModeWin32.dmDisplayFrequency;
			currentDisplayMode.Width = currentDisplayModeWin32.dmPelsWidth;
			currentDisplayMode.Height = currentDisplayModeWin32.dmPelsHeight;

			std::vector<MonitorDisplayMode> displayModes;
			unsigned int modeIndex = 0;
			if (EnumDisplaySettings(monitorInfo.szDevice, 0, &currentDisplayModeWin32) == 0)
				return {};

			while (true)
			{
				if (EnumDisplaySettings(monitorInfo.szDevice, modeIndex, &currentDisplayModeWin32) == 0)
					break;

				MonitorDisplayMode displayMode = {};
				displayMode.Width = currentDisplayModeWin32.dmPelsWidth;
				displayMode.Height = currentDisplayModeWin32.dmPelsHeight;
				displayMode.BitsPerPixel = currentDisplayModeWin32.dmBitsPerPel;
				displayMode.RefreshRate = currentDisplayModeWin32.dmDisplayFrequency;

				displayModes.push_back(displayMode);
				modeIndex++;
			}

			Win32Monitor* pMonitor = new Win32Monitor(monitorHandle, monitorInfo.szDevice, currentDisplayMode, displayModes, monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top);
			monitors.push_back(pMonitor);
		}

		return monitors;
	}
	Win32Monitor::Win32Monitor(const HMONITOR handle, const std::string& name, const MonitorDisplayMode& currentMode, const std::vector<MonitorDisplayMode>& modes, const unsigned int width, const unsigned int height, const int x, const int y)
		: PlatformMonitor(name, currentMode, modes, width, height, x, y), mHandle(handle)
	{

	}
	Win32Monitor::~Win32Monitor()
	{

	}
}