#pragma once

namespace Oksijen
{
	class RUNTIME_API PlatformTime final
	{
		friend class Platform;
	private:
		static void Initialize();
	public:
		static unsigned long long GetCurrentTimeAsNanoseconds();
	public:
		PlatformTime() = delete;
		~PlatformTime() = delete;
	};
}