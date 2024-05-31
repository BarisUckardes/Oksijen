#pragma once

namespace Oksijen
{
	class RUNTIME_API Win32Time final
	{
		friend class PlatformTime;
	public:
		FORCEINLINE static unsigned long long GetCurrentTimeAsNanoseconds();
	private:
		static void Initialize();
	public:
		Win32Time() = delete;
		~Win32Time() = delete;
	};
}