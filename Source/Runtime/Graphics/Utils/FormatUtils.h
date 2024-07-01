#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class RUNTIME_API FormatUtils final
	{
	public:
		static unsigned char GetFormatSize(const VkFormat format);
	public:
		FormatUtils() = delete;
		~FormatUtils() = delete;

	};
}