#include "PlatformTime.h"

#ifdef OKSIJEN_PLATFORM_WINDOWS
#include <Runtime/Windows/Win32Time.h>
typedef Oksijen::Win32Time PlatformAbstraction;
#endif

namespace Oksijen
{
	void PlatformTime::Initialize()
	{
		PlatformAbstraction::Initialize();
	}
	unsigned long long PlatformTime::GetCurrentTimeAsNanoseconds()
	{
		return PlatformAbstraction::GetCurrentTimeAsNanoseconds();
	}
}