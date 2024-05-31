#pragma once
#include <Runtime/Core/Core.h>
#include <vulkan/vulkan.h>

namespace Oksijen
{
	class GraphicsDevice;
	class RUNTIME_API CommandPool final
	{
	public:
		CommandPool(const GraphicsDevice* pDevice,const VkQueueFlags flags);
		~CommandPool();

		FORCEINLINE VkCommandPool GetCommandPool() const noexcept { return mPool; }
	private:
		const VkDevice mLogicalDevice;
		VkCommandPool mPool;
	};
}