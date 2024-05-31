#include "Platform.h"

namespace Oksijen
{
	PlatformType Platform::GetPlatformType()
	{
#ifdef OKSIJEN_PLATFORM_WINDOWS
		return PlatformType::Windows;
#elif OKSIJEN_PLATFORM_LINUX
		return PlatformType::Linux;
#error ("Invalid platform detected!")
#endif
	}
	void Platform::Initialize()
	{

	}
}