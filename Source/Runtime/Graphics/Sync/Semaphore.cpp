#include "Semaphore.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
    Semaphore::Semaphore(const GraphicsDevice* pDevice, const bool bSignalled) : mLogicalDevice(pDevice->GetLogicalDevice()),mSemaphore(VK_NULL_HANDLE)
    {
        VkSemaphoreCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        info.flags = bSignalled ? VK_FENCE_CREATE_SIGNALED_BIT : VkSemaphoreCreateFlags();
        info.pNext = nullptr;

        DEV_ASSERT(vkCreateSemaphore(mLogicalDevice, &info, nullptr, &mSemaphore) == VK_SUCCESS, "VulkanSemaphore", "Failed to create semaphore");
    }
    Semaphore::~Semaphore()
    {
        vkDestroySemaphore(mLogicalDevice, mSemaphore, nullptr);
    }
}