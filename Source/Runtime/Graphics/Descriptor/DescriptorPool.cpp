#include "DescriptorPool.h"
#include <Runtime/Graphics/Device/GraphicsDevice.h>

namespace Oksijen
{
    DescriptorPool::DescriptorPool(
        const GraphicsDevice* pDevice,
        const VkDescriptorPoolCreateFlags flags,
        const unsigned int maxSets,
        const VkDescriptorType* pDescriptorTypes, const unsigned int* pDescriptorCounts,
        const unsigned int descriptorTypeCount) : mLogicalDevice(pDevice->GetLogicalDevice()),mPool(VK_NULL_HANDLE)
    {
        VkDescriptorPoolSize sizes[8];
        for (unsigned int i = 0; i < descriptorTypeCount; i++)
        {
            VkDescriptorPoolSize size = {};
            size.type = pDescriptorTypes[i];
            size.descriptorCount = pDescriptorCounts[i];
            sizes[i] = size;
        }

        VkDescriptorPoolCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.maxSets = maxSets;
        info.poolSizeCount = descriptorTypeCount;
        info.pPoolSizes = sizes;
        info.flags = flags;
        info.pNext = nullptr;

        DEV_ASSERT(vkCreateDescriptorPool(mLogicalDevice, &info, nullptr, &mPool) == VK_SUCCESS, "DescriptorSetPool","Failed to create descriptor pool");
    }
    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(mLogicalDevice, mPool, nullptr);
    }
}