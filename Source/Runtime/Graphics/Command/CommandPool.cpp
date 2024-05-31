#include "CommandPool.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	CommandPool::CommandPool(const GraphicsDevice* pDevice, const VkQueueFlags flags) : mPool(VK_NULL_HANDLE),mLogicalDevice(pDevice->GetLogicalDevice())
	{
		//Get queue family index
		const unsigned char familyIndex = pDevice->GetQueueFamilyIndex(flags);
		DEV_ASSERT(familyIndex != 255, "CommandPool", "Invalid queue family index");

		//Create command pool
		VkCommandPoolCreateInfo cmdPoolInfo = {};
		cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolInfo.queueFamilyIndex = familyIndex;
		cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		DEV_ASSERT(vkCreateCommandPool(mLogicalDevice, &cmdPoolInfo, nullptr, &mPool) == VK_SUCCESS, "VulkanCommandPool", "Failed to create command pool");
	}
	CommandPool::~CommandPool()
	{
		vkDestroyCommandPool(mLogicalDevice, mPool, nullptr);
	}
}