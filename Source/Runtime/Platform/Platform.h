#pragma once
#include <Runtime/Platform/PlatformType.h>

namespace Oksijen
{
	class RUNTIME_API Platform final
	{
	public:
		FORCEINLINE static PlatformType GetPlatformType();
		static void Initialize();
	public:
		Platform() = delete;
		~Platform() = delete;
	};
}