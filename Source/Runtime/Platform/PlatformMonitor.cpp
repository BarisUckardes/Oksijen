#include "PlatformMonitor.h"

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Runtime/Windows/Win32Monitor.h>
typedef Oksijen::Win32Monitor PlatformAbstraction;
#endif
namespace Oksijen
{
	PlatformMonitor* PlatformMonitor::GetPrimaryMonitor()
	{
		return PlatformAbstraction::GetPrimaryMonitor();
	}
	std::vector<PlatformMonitor*> PlatformMonitor::GetMonitors()
	{
		return PlatformAbstraction::GetMonitors();
	}
	PlatformMonitor::~PlatformMonitor()
	{
	}
}