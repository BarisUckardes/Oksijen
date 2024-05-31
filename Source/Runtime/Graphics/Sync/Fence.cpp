#include "Fence.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
	Fence::Fence(const GraphicsDevice* pDevice, const bool bSignalled) : mLogicalDevice(pDevice->GetLogicalDevice()),mFence(VK_NULL_HANDLE)
	{
		VkFenceCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.flags = bSignalled ? VK_FENCE_CREATE_SIGNALED_BIT : VkFenceCreateFlags();
		info.pNext = nullptr;

		DEV_ASSERT(vkCreateFence(mLogicalDevice, &info, nullptr, &mFence) == VK_SUCCESS, "VulkanFence", "Failed to create fence");
	}
	Fence::~Fence()
	{
		vkDestroyFence(mLogicalDevice, mFence,nullptr);

	}
	void Fence::Wait(const unsigned long long waitAmount)
	{
		vkWaitForFences(mLogicalDevice, 1, &mFence, VK_TRUE, waitAmount);
	}
	void Fence::Reset()
	{
		vkResetFences(mLogicalDevice, 1, &mFence);
	}
}