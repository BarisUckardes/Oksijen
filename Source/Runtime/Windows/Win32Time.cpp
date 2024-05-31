#include "Win32Time.h"
#include <Windows.h>

namespace Oksijen
{
	LARGE_INTEGER sFrequency = {};

	unsigned long long Win32Time::GetCurrentTimeAsNanoseconds()
	{
		LARGE_INTEGER value = {};
		QueryPerformanceCounter(&value);

		value.QuadPart *= 1000000;
		value.QuadPart /= sFrequency.QuadPart;

		return value.QuadPart;
	}
	void Win32Time::Initialize()
	{
		QueryPerformanceFrequency(&sFrequency);
	}
}